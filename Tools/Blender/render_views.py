# Renders orthographic preview images of a model or .blend (front, side, back, top, 3/4),
# with the armature drawn in front if there is one. Workbench, so it runs headless.
# blender --background --factory-startup --python render_views.py -- <model.glb|.blend> <out_prefix> [action] [frame,frame,...]
import math
import sys
import bpy
from mathutils import Vector

args = sys.argv[sys.argv.index("--") + 1:]
path, out = args[0], args[1]
action_name = args[2] if len(args) > 2 and args[2] != "-" else None
frames = [int(f) for f in args[3].split(",")] if len(args) > 3 else None

if path.lower().endswith(".blend"):
    bpy.ops.wm.open_mainfile(filepath=path)
else:
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.gltf(filepath=path)

scene = bpy.context.scene
scene.render.engine = "BLENDER_WORKBENCH"
scene.display.shading.light = "STUDIO"
scene.display.shading.color_type = "MATERIAL"
scene.display.shading.show_object_outline = True
scene.render.resolution_x = 512
scene.render.resolution_y = 512
scene.render.film_transparent = False
scene.world = scene.world or bpy.data.worlds.new("W")
scene.render.image_settings.file_format = "PNG"

meshes = [o for o in scene.objects if o.type == "MESH"]
arm = next((o for o in scene.objects if o.type == "ARMATURE"), None)
if arm:
    arm.show_in_front = True
    arm.data.display_type = "STICK"
    if action_name and action_name != "weights":
        arm.animation_data_create()
        arm.animation_data.action = bpy.data.actions[action_name]
        if hasattr(arm.animation_data, "action_slot") and bpy.data.actions[action_name].slots:
            arm.animation_data.action_slot = bpy.data.actions[action_name].slots[0]

def bounds():
    deps = bpy.context.evaluated_depsgraph_get()
    pts = []
    for o in meshes:
        ev = o.evaluated_get(deps)
        me = ev.to_mesh()
        pts += [ev.matrix_world @ v.co for v in me.vertices]
        ev.to_mesh_clear()
    lo = Vector((min(p.x for p in pts), min(p.y for p in pts), min(p.z for p in pts)))
    hi = Vector((max(p.x for p in pts), max(p.y for p in pts), max(p.z for p in pts)))
    return lo, hi

cam_data = bpy.data.cameras.new("PreviewCam")
cam_data.type = "ORTHO"
cam = bpy.data.objects.new("PreviewCam", cam_data)
scene.collection.objects.link(cam)
scene.camera = cam

views = {"front": (1, 0, 0), "side": (0, -1, 0), "back": (-1, 0, 0), "top": (0, 0, 1), "34": (1, -1, 0.6)}

# Workbench doesn't draw armatures, so the bones are drawn as yellow sticks (left side green)
# through a see-through body.
stick_mats = {}
for key, rgba in (("c", (1, 0.85, 0.1, 1)), ("l", (0.2, 1, 0.3, 1)), ("r", (0.2, 0.6, 1, 1))):
    m = bpy.data.materials.new(f"Stick_{key}")
    m.diffuse_color = rgba
    stick_mats[key] = m
sticks = None

def build_sticks():
    global sticks
    if sticks:
        bpy.data.objects.remove(sticks)
    me = bpy.data.meshes.new("Sticks")
    for m in stick_mats.values():
        me.materials.append(m)
    verts, faces, mats = [], [], []
    for pb in arm.pose.bones:
        h = arm.matrix_world @ pb.head
        t = arm.matrix_world @ pb.tail
        axis = (t - h)
        if axis.length < 1e-5:
            continue
        side = axis.orthogonal().normalized() * 0.008
        side2 = axis.normalized().cross(side)
        base = len(verts)
        for p in (h, t):
            for o in (side + side2, side - side2, -side - side2, -side + side2):
                verts.append(p + o)
        quads = [(0, 1, 5, 4), (1, 2, 6, 5), (2, 3, 7, 6), (3, 0, 4, 7), (0, 3, 2, 1), (4, 5, 6, 7)]
        faces += [tuple(base + i for i in q) for q in quads]
        key = "l" if pb.name.endswith("_l") else "r" if pb.name.endswith("_r") else "c"
        mats += [list(stick_mats).index(key)] * len(quads)
    me.from_pydata(verts, [], faces)
    for poly, mi in zip(me.polygons, mats):
        poly.material_index = mi
    sticks = bpy.data.objects.new("Sticks", me)
    scene.collection.objects.link(sticks)

if arm and action_name == "weights":
    # Colour each face by the bone that owns most of its weight (rest pose, no sticks).
    import colorsys
    arm.animation_data.action = None
    for pb in arm.pose.bones:
        pb.rotation_quaternion = (1, 0, 0, 0)
        pb.location = (0, 0, 0)
    body = meshes[0]
    me = body.data
    me.materials.clear()
    groups = list(body.vertex_groups)
    for i, g in enumerate(groups):
        m = bpy.data.materials.new(f"W_{g.name}")
        m.diffuse_color = (*colorsys.hsv_to_rgb((i * 0.618) % 1.0, 0.75, 1.0), 1)
        me.materials.append(m)
    for poly in me.polygons:
        tot = [0.0] * len(groups)
        for vi in poly.vertices:
            for g in me.vertices[vi].groups:
                tot[g.group] += g.weight
        poly.material_index = max(range(len(groups)), key=lambda i: tot[i])
    print("LEGEND " + ", ".join(f"{g.name}=hue{(i * 0.618) % 1.0:.2f}" for i, g in enumerate(groups)))
    arm = None

if arm:
    scene.display.shading.show_xray = True
    scene.display.shading.xray_alpha = 0.55

for f in (frames or [scene.frame_current]):
    scene.frame_set(f)
    if arm:
        build_sticks()
    lo, hi = bounds()
    centre = (lo + hi) / 2
    size = max(hi - lo) * 1.25
    cam_data.ortho_scale = size
    for name, d in views.items():
        d = Vector(d).normalized()
        cam.location = centre + d * 5
        up = Vector((0, 1, 0)) if name == "top" else Vector((0, 0, 1))
        cam.rotation_euler = (-d).to_track_quat("-Z", "Y").to_euler()
        if name == "top":
            cam.rotation_euler = (0, 0, math.radians(90))
        scene.render.filepath = f"{out}_{name}" + (f"_f{f}" if frames else "") + ".png"
        bpy.ops.render.render(write_still=True)
print("RENDER_DONE")
