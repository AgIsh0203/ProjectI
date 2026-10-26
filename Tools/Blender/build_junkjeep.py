# Builds the "junk jeep" (A2): a toy jeep (Poly Pizza, CC-BY 3.0) patched up with trash by the squirrels,
# where every part that breaks is a piece of trash you can see: a car battery on the hood (the engine),
# a sponge on a twig lever at a rear wheel (the brakes), a cardboard flap (the door), tape on the tyres.
# Fitted to the template buggy's skeleton (SKM_Offroad), so Chaos, the wheel bones and the physics asset
# stay as they are.
#
# blender --background --factory-startup --python build_junkjeep.py -- <layout.json> <out_dir>
#
# Writes, in metres, x forward, z up (all positions in the layout and the printout are Unreal's: y right):
#   SM_JunkJeep_Body.fbx   the jeep and the decor props merged, origin = the car actor's origin, with a
#                          UCX_ box (the collision the template welds into the car) round the tub only
#   SM_JunkJeep_Wheel.fbx  one wheel, origin = axle centre, radius = the Chaos wheel radius
#   SM_JunkJeep_Door.fbx   the door props, origin = the hinge (layout["door"]["hinge"])
#   SM_JunkJeep_Engine.fbx the engine props, origin = layout["engine"]["origin"]
#   preview_*.png          with the seats drawn as squirrel-sized boxes
# Without "props" in the layout it prints the fitted jeep's landmarks, to place things by.
import json
import math
import os
import sys

import bmesh
import bpy
from mathutils import Euler, Matrix, Vector

args = sys.argv[sys.argv.index("--") + 1:]
L = json.load(open(args[0]))
out_dir = os.path.abspath(args[1])
src_dir = os.path.abspath(L["src_dir"])
bpy.ops.wm.read_factory_settings(use_empty=True)
scene = bpy.context.scene


def to_bl(v):
    """Unreal (y right) -> Blender (y left)."""
    return Vector((v[0], -v[1], v[2]))


def ue(v):
    return (round(v.x, 1), round(-v.y, 1), round(v.z, 1))


def bounds(objs):
    pts = [o.matrix_world @ v.co for o in objs for v in o.data.vertices]
    lo = Vector((min(p.x for p in pts), min(p.y for p in pts), min(p.z for p in pts)))
    hi = Vector((max(p.x for p in pts), max(p.y for p in pts), max(p.z for p in pts)))
    return lo, hi


def import_glb(name):
    before = set(scene.objects)
    bpy.ops.import_scene.gltf(filepath=os.path.join(src_dir, name))
    new = [o for o in scene.objects if o not in before]
    meshes = [o for o in new if o.type == "MESH"]
    for o in meshes:
        mw = o.matrix_world.copy()
        o.parent = None
        o.data = o.data.copy()
        o.data.transform(mw)
        o.matrix_world = Matrix.Identity(4)
    for o in new:
        if o.type != "MESH":
            bpy.data.objects.remove(o)
    return join(meshes, name)


def join(objs, name):
    bpy.ops.object.select_all(action="DESELECT")
    for o in objs:
        o.select_set(True)
    bpy.context.view_layer.objects.active = objs[0]
    if len(objs) > 1:
        bpy.ops.object.join()
    ob = bpy.context.view_layer.objects.active
    ob.name = name
    return ob


def split_islands(ob):
    bpy.ops.object.select_all(action="DESELECT")
    ob.select_set(True)
    bpy.context.view_layer.objects.active = ob
    bpy.ops.object.mode_set(mode="EDIT")
    bpy.ops.mesh.select_all(action="SELECT")
    bpy.ops.mesh.remove_doubles(threshold=1e-4)
    bpy.ops.mesh.separate(type="LOOSE")
    bpy.ops.object.mode_set(mode="OBJECT")
    return [o for o in scene.objects if o.type == "MESH" and (o is ob or o.name.startswith(ob.name + "."))]


# --- The jeep: forward is its -y, up z, in metres. Turn it to drive along +x and fit it on the skeleton.
jeep = import_glb(L["jeep"])
J = L["fit"]
parts = split_islands(jeep)
wheel_like = [o for o in parts if len(o.data.vertices) == J["wheel_verts"]]
ground = min(bounds([o])[0].z for o in wheel_like)
wheels = [o for o in wheel_like if bounds([o])[0].z < ground + 0.01]
assert len(wheels) == 4, [o.name for o in wheels]
# The spare tyre on the tailgate: from the chase camera it hid the whole car.
for o in [o for o in wheel_like if o not in wheels and J.get("drop_spare")]:
    parts.remove(o)
    bpy.data.objects.remove(o)
body = [o for o in parts if o not in wheels]
wc = {o: (bounds([o])[0] + bounds([o])[1]) / 2 for o in wheels}
radius = sum((bounds([o])[1].z - bounds([o])[0].z) / 2 for o in wheels) / 4
front_y = min(c.y for c in wc.values())
rear_y = max(c.y for c in wc.values())
half_track = sum(abs(c.x) for c in wc.values()) / 4

s = (L["wheels"][0][0] - L["wheels"][2][0]) / (rear_y - front_y)
sy = s * J["width_scale"]
turn = Matrix.Rotation(math.radians(90), 4, "Z")  # -y (front) -> +x
fit = (Matrix.Translation(Vector((L["wheels"][0][0], 0, L["wheels"][0][2])))
       @ Matrix.Diagonal(Vector((s, sy, s, 1)))
       @ Matrix.Translation(Vector((-(-front_y), 0, -(ground + radius))))
       @ turn)
print(f"FIT s={s:.2f} sy={sy:.2f} radius={radius * s:.1f} half_track={half_track * sy:.1f}")
for o in body:
    o.data.transform(fit)

# One wheel, centred, scaled uniformly to the Chaos radius; the others go.
wheel = wheels[0]
wcen = wc[wheel]
ws = L["wheel_radius"] / radius
wheel.data.transform(Matrix.Diagonal(Vector((ws, ws, ws, 1))) @ turn @ Matrix.Translation(-wcen))
if wcen.x > 0:
    wheel.data.transform(Matrix.Rotation(math.pi, 4, "Z"))
for o in wheels[1:]:
    bpy.data.objects.remove(o)
wheel.name = "SM_JunkJeep_Wheel"

body_ob = join(body, "SM_JunkJeep_Body")
blo, bhi = bounds([body_ob])
print(f"JEEP body min={ue(blo)} max={ue(bhi)}")

if "props" not in L:
    # Landmarks: the jeep's loose parts with their Unreal bounds, biggest first.
    for o in sorted(split_islands(body_ob), key=lambda o: -len(o.data.vertices)):
        lo, hi = bounds([o])
        print(f"  ISL verts={len(o.data.vertices):4d} min={ue(lo)} max={ue(hi)} mats={sorted({o.data.materials[p.material_index].name for p in o.data.polygons})}")
    print("BUILD_JUNKJEEP_DONE (landmarks only)")
    sys.exit(0)


# --- Props: each is scaled so its longest side is "size" cm (or by "scale"), rotated (Unreal degrees:
# roll, pitch, yaw), and placed with the middle of its base at "at". "into" picks the mesh it joins.
def place_prop(p):
    ob = import_glb(p["model"])
    lo, hi = bounds([ob])
    ob.data.transform(Matrix.Translation(-Vector(((lo.x + hi.x) / 2, (lo.y + hi.y) / 2, lo.z))))
    k = p["scale"] if "scale" in p else p["size"] / max(hi - lo)
    sc = Vector(p.get("stretch", (1, 1, 1))) * k
    r = p.get("rot", (0, 0, 0))
    # Unreal rotator (roll x, pitch y, yaw z; left-handed) -> Blender: negate pitch and yaw for the y flip.
    rot = Euler((math.radians(r[0]), math.radians(-r[1]), math.radians(-r[2])), "XYZ").to_matrix().to_4x4()
    ob.data.transform(Matrix.Translation(to_bl(p["at"])) @ rot @ Matrix.Diagonal(Vector((sc.x, sc.y, sc.z, 1))))
    return ob


groups = {"body": [body_ob], "door": [], "engine": []}
for p in L["props"]:
    groups[p.get("into", "body")].append(place_prop(p))
body_ob = join(groups["body"], "SM_JunkJeep_Body")
door_ob = join(groups["door"], "SM_JunkJeep_Door")
engine_ob = join(groups["engine"], "SM_JunkJeep_Engine")
hinge = to_bl(L["door"]["hinge"])
door_ob.data.transform(Matrix.Translation(-hinge))
eng_origin = to_bl(L["engine"]["origin"])
engine_ob.data.transform(Matrix.Translation(-eng_origin))

# The collision box: the tub only (not the roll bar or windscreen), so acorns spilled over the top and
# squirrels on the seats aren't inside it.
c = L["collision"]
bm = bmesh.new()
bmesh.ops.create_cube(bm, size=1.0)
lo, hi = to_bl(c["min"]), to_bl(c["max"])
lo, hi = Vector((min(lo.x, hi.x), min(lo.y, hi.y), min(lo.z, hi.z))), Vector((max(lo.x, hi.x), max(lo.y, hi.y), max(lo.z, hi.z)))
bm.transform(Matrix.Translation((lo + hi) / 2) @ Matrix.Diagonal(Vector((*(hi - lo), 1))))
ucx_me = bpy.data.meshes.new("UCX_SM_JunkJeep_Body")
bm.to_mesh(ucx_me)
ucx = bpy.data.objects.new("UCX_SM_JunkJeep_Body", ucx_me)
scene.collection.objects.link(ucx)

for o in (body_ob, door_ob, engine_ob, wheel):
    lo, hi = bounds([o])
    print(f"OUT {o.name} min={ue(lo)} max={ue(hi)}")

# --- Previews.
previews = []


def ghost(src_ob, loc, yaw=0.0):
    cp = src_ob.copy()
    cp.data = src_ob.data.copy()
    cp.location = loc
    cp.rotation_euler = (0, 0, yaw)
    cp.hide_render = False
    scene.collection.objects.link(cp)
    previews.append(cp)


for o in (door_ob, engine_ob, wheel, ucx):
    o.hide_render = True
for (x, y, z) in L["wheels"]:
    ghost(wheel, to_bl((x, y, z)), math.pi if y > 0 else 0)
ghost(door_ob, hinge, math.radians(L.get("preview_door_open", 0)))
ghost(engine_ob, eng_origin)
mat = bpy.data.materials.new("Seat")
mat.diffuse_color = (1, 0.5, 0.1, 1)
for name, v in L.get("seats", {}).items():
    bpy.ops.mesh.primitive_cube_add(size=1, location=to_bl(v))
    b = bpy.context.active_object
    b.scale = (30, 30, 44)
    b.data.materials.append(mat)
    previews.append(b)
if L.get("preview_collision"):
    ghost(ucx, (0, 0, 0))
scene.render.engine = "BLENDER_WORKBENCH"
scene.display.shading.light = "STUDIO"
scene.display.shading.color_type = "MATERIAL"
scene.display.shading.show_object_outline = True
scene.render.resolution_x = 800
scene.render.resolution_y = 600
cam_data = bpy.data.cameras.new("Cam")
cam_data.type = "ORTHO"
cam_data.clip_end = 100000
cam = bpy.data.objects.new("Cam", cam_data)
scene.collection.objects.link(cam)
scene.camera = cam
bpy.context.view_layer.update()
lo, hi = bounds([body_ob] + previews)
mid = (lo + hi) / 2
cam_data.ortho_scale = max(hi - lo) * 1.1
# Directions in Unreal axes (x forward, y right).
for name, d in {"front34": (1, -1, 0.7), "rear34": (-1, 1, 0.8), "right34": (0.4, 1, 0.5), "side": (0, -1, 0),
                "top": (0, 0, 1), "chase": (-1, 0, 0.35)}.items():
    d = to_bl(d).normalized()
    cam.location = mid + d * 3000
    cam.rotation_euler = (-d).to_track_quat("-Z", "X" if name == "top" else "Y").to_euler()
    scene.render.filepath = os.path.join(out_dir, f"preview_{name}.png")
    bpy.ops.render.render(write_still=True)
for p in previews:
    bpy.data.objects.remove(p)
for o in (door_ob, engine_ob, wheel, ucx):
    o.hide_render = False

# --- Export: centimetres -> metres.
for o in (body_ob, door_ob, engine_ob, wheel, ucx):
    o.data.transform(Matrix.Diagonal(Vector((0.01, 0.01, 0.01, 1))))
    for poly in o.data.polygons:
        poly.use_smooth = False
os.makedirs(out_dir, exist_ok=True)
bpy.ops.wm.save_as_mainfile(filepath=os.path.join(out_dir, "JunkJeep.blend"))
for o, extra in ((body_ob, [ucx]), (door_ob, []), (engine_ob, []), (wheel, [])):
    bpy.ops.object.select_all(action="DESELECT")
    for x in [o] + extra:
        x.select_set(True)
    bpy.ops.export_scene.fbx(filepath=os.path.join(out_dir, o.name + ".fbx"), use_selection=True,
                             object_types={"MESH"}, apply_unit_scale=True, apply_scale_options="FBX_SCALE_ALL",
                             mesh_smooth_type="FACE", bake_anim=False)
print("BUILD_JUNKJEEP_DONE")
