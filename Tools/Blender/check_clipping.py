# Measures how deep the paws go into the head in every frame of every action of a rigged squirrel.
#   blender --background --python check_clipping.py -- <rig.blend>
# Paw vertices = mostly weighted to an arm bone; head surface = faces mostly weighted to head/neck.
# A vertex is inside when most axis rays from it cross the head surface an odd number of times;
# its depth is the distance to the nearest head face.
# Prints, per action, the worst depth (cm) and the frame; 0 = never touches.
import sys

import bpy
from mathutils import Vector
from mathutils.bvhtree import BVHTree

path = sys.argv[sys.argv.index("--") + 1]
bpy.ops.wm.open_mainfile(filepath=path)
scene = bpy.context.scene
arm = next(o for o in scene.objects if o.type == "ARMATURE")
body = next(o for o in scene.objects if o.type == "MESH")
names = {g.index: g.name for g in body.vertex_groups}

def share(v, prefixes):
    return sum(g.weight for g in v.groups if names[g.group].startswith(prefixes))

me0 = body.data
paw = [v.index for v in me0.vertices if share(v, ("hand", "lowerarm", "upperarm")) > 0.5]
tail = [v.index for v in me0.vertices if share(v, ("tail",)) > 0.5]
head_v = {v.index for v in me0.vertices if share(v, ("head",)) > 0.5}
head_f = [p.index for p in me0.polygons if sum(1 for i in p.vertices if i in head_v) >= len(p.vertices) - 0]
tail_set = set(tail)
print(f"CLIP paw_verts={len(paw)} head_faces={len(head_f)}")

DIRS = [Vector(d) for d in ((1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1))]


def inside(bvh, p):
    # The head is open at the neck, so a point counts as inside when at least 4 of 6 axis rays
    # cross the head surface an odd number of times.
    odd = 0
    for d in DIRS:
        n, o = 0, p.copy()
        while True:
            hit = bvh.ray_cast(o, d)
            if hit[0] is None:
                break
            n += 1
            o = hit[0] + d * 1e-4
        odd += n % 2
    return odd >= 4


worst_all = 0.0
REST = bpy.data.actions.new("_Rest")
REST.frame_range = (0, 0)
for action in [REST] + [a for a in bpy.data.actions if a is not REST]:
    arm.animation_data.action = action
    if action.slots:
        arm.animation_data.action_slot = action.slots[0]
    start, end = int(action.frame_range[0]), int(action.frame_range[1])
    worst, worst_f, tail_worst = 0.0, None, 0.0
    for f in range(start, end + 1):
        scene.frame_set(f)
        deps = bpy.context.evaluated_depsgraph_get()
        ev = body.evaluated_get(deps)
        me = ev.to_mesh()
        verts = [ev.matrix_world @ v.co for v in me.vertices]
        polys = [[i for i in me.polygons[fi].vertices] for fi in head_f]
        bvh = BVHTree.FromPolygons(verts, polys)
        for vi in paw + tail:
            p = verts[vi]
            if not inside(bvh, p):
                continue
            depth = bvh.find_nearest(p)[3]
            if vi in tail_set:
                tail_worst = max(tail_worst, depth)
            elif depth > worst:
                worst, worst_f = depth, f
        ev.to_mesh_clear()
    worst_all = max(worst_all, worst, tail_worst)
    print(f"CLIP {action.name:8s} paws={worst * 100:5.1f} cm (frame {worst_f})  tail={tail_worst * 100:5.1f} cm")
print(f"CLIP_DONE worst={worst_all * 100:.1f} cm")
