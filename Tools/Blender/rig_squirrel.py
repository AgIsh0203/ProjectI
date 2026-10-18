# Rigs a squirrel model with the shared squirrel skeleton, adds the animations from
# squirrel_anims.py, and saves a .blend plus an FBX for Unreal.
#   blender --background --factory-startup --python rig_squirrel.py -- <rig.json> <out.blend> <out.fbx>
# The rig file names the model (next to it) and the joint positions in the model's own metres.
# Every model gets the same bone names and hierarchy, so they can share one Unreal skeleton
# and its animations; only the joint positions differ.
import json
import os
import sys

import bmesh
import bpy
from mathutils import Vector

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import squirrel_anims  # noqa: E402

args = sys.argv[sys.argv.index("--") + 1:]
rig_path, out_blend, out_fbx = (os.path.abspath(a) for a in args[:3])
rig = json.load(open(rig_path))
model_path = os.path.join(os.path.dirname(rig_path), rig["model"])

bpy.ops.wm.read_factory_settings(use_empty=True)
scene = bpy.context.scene
scene.render.fps = squirrel_anims.FPS

# --- Mesh: one object, coincident verts merged (the glTF splits them for flat shading), flat shaded,
# feet on z = 0, origin between the feet.
bpy.ops.import_scene.gltf(filepath=model_path)
meshes = [o for o in scene.objects if o.type == "MESH"]
for o in scene.objects:
    o.select_set(o in meshes)
bpy.context.view_layer.objects.active = meshes[0]
if len(meshes) > 1:
    bpy.ops.object.join()
body = bpy.context.view_layer.objects.active
body.parent = None
bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
body.name = body.data.name = "SK_Squirrel"

bm = bmesh.new()
bm.from_mesh(body.data)
bmesh.ops.remove_doubles(bm, verts=bm.verts, dist=1e-4)
floor = min(v.co.z for v in bm.verts)
shift = Vector((0, 0, -floor))
for v in bm.verts:
    v.co += shift
for f in bm.faces:
    f.smooth = False
bm.to_mesh(body.data)
bm.free()
body.data.update()

def P(name_or_xyz, mirror=False):
    xyz = Vector(rig[name_or_xyz] if isinstance(name_or_xyz, str) else name_or_xyz) + shift
    if mirror:
        xyz.y = -xyz.y
    return xyz

# --- Skeleton. Bones point from joint to the next joint; leaf bones point at the *_end points.
arm_data = bpy.data.armatures.new("Armature")
arm = bpy.data.objects.new("Armature", arm_data)  # "Armature" stops Unreal adding an extra root bone
scene.collection.objects.link(arm)
bpy.context.view_layer.objects.active = arm
for o in scene.objects:
    o.select_set(o is arm)
bpy.ops.object.mode_set(mode="EDIT")
eb = arm_data.edit_bones

def bone(name, head, tail, parent=None, connect=False, deform=True):
    b = eb.new(name)
    b.head, b.tail = head, tail
    if (b.tail - b.head).length < 1e-3:
        b.tail = b.head + Vector((0, 0, 0.02))
    b.parent = eb.get(parent) if parent else None
    b.use_connect = connect and b.parent is not None and (b.parent.tail - b.head).length < 1e-4
    b.use_deform = deform
    b.roll = 0
    return b

pelvis = P("pelvis")
bone("root", Vector((0, 0, 0)), Vector((0, 0, 0.1)), deform=False)
bone("pelvis", pelvis, P("spine_02"), "root")
bone("spine_02", P("spine_02"), P("neck"), "pelvis", True)
bone("neck", P("neck"), P("head"), "spine_02", True)
bone("head", P("head"), P("head_end"), "neck", True)
for side, m in (("l", False), ("r", True)):
    bone(f"upperarm_{side}", P("upperarm", m), P("lowerarm", m), "spine_02")
    bone(f"lowerarm_{side}", P("lowerarm", m), P("hand", m), f"upperarm_{side}", True)
    bone(f"hand_{side}", P("hand", m), P("hand_end", m), f"lowerarm_{side}", True)
    bone(f"thigh_{side}", P("thigh", m), P("calf", m), "pelvis")
    bone(f"calf_{side}", P("calf", m), P("foot", m), f"thigh_{side}", True)
    bone(f"foot_{side}", P("foot", m), P("foot_end", m), f"calf_{side}", True)
tail = rig["tail"]
for i in range(len(tail) - 1):
    bone(f"tail_{i + 1:02d}", P(tail[i]), P(tail[i + 1]), f"tail_{i:02d}" if i else "pelvis", i > 0)
bpy.ops.object.mode_set(mode="OBJECT")

# --- Skin: Blender's heat-map automatic weights, then at most 4 influences per vertex
# (what Unreal keeps by default), normalised.
for o in scene.objects:
    o.select_set(o in (body, arm))
bpy.context.view_layer.objects.active = arm
bpy.ops.object.parent_set(type="ARMATURE_AUTO")
bpy.context.view_layer.objects.active = body

bpy.ops.object.vertex_group_limit_total(group_select_mode="ALL", limit=4)
bpy.ops.object.vertex_group_normalize_all(group_select_mode="ALL", lock_active=False)

names = {g.index: g.name for g in body.vertex_groups}
counts, unweighted = {}, 0
for v in body.data.vertices:
    gs = [g for g in v.groups if g.weight > 0.01]
    if not gs:
        unweighted += 1
        continue
    top = max(gs, key=lambda g: g.weight)
    counts[names[top.group]] = counts.get(names[top.group], 0) + 1
print(f"RIG verts={len(body.data.vertices)} unweighted={unweighted} by_bone={dict(sorted(counts.items()))}")

# --- Animations
squirrel_anims.build(arm)
print(f"RIG actions={[a.name for a in bpy.data.actions]}")

os.makedirs(os.path.dirname(out_blend), exist_ok=True)
bpy.ops.wm.save_as_mainfile(filepath=out_blend)

# --- FBX for Unreal: armature + mesh, deform bones only, no leaf bones, every action baked.
for o in scene.objects:
    o.select_set(o in (body, arm))
bpy.ops.export_scene.fbx(
    filepath=out_fbx,
    use_selection=True,
    object_types={"ARMATURE", "MESH"},
    apply_unit_scale=True,
    apply_scale_options="FBX_SCALE_ALL",
    mesh_smooth_type="FACE",
    use_armature_deform_only=False,
    add_leaf_bones=False,
    primary_bone_axis="Y",
    secondary_bone_axis="X",
    bake_anim=True,
    bake_anim_use_all_actions=True,
    bake_anim_use_nla_strips=False,
    bake_anim_force_startend_keying=True,
    bake_anim_simplify_factor=0.0,
)
print(f"RIG_DONE {out_fbx}")
