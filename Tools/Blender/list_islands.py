# Lists a model's loose parts (vertex islands joined by edges, after merging coincident verts):
# their vertex count, bounds and centre. Separate limbs bind cleanly to one bone each.
# blender --background --factory-startup --python list_islands.py -- <model.glb>
import sys
import bmesh
import bpy

path = sys.argv[sys.argv.index("--") + 1]
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=path)
ob = next(o for o in bpy.data.objects if o.type == "MESH")
bm = bmesh.new()
bm.from_mesh(ob.data)
bm.transform(ob.matrix_world)
bmesh.ops.remove_doubles(bm, verts=bm.verts, dist=1e-4)
print(f"merged verts={len(bm.verts)} faces={len(bm.faces)}")
seen = set()
islands = []
for v in bm.verts:
    if v.index in seen:
        continue
    stack, isl = [v], []
    seen.add(v.index)
    while stack:
        cur = stack.pop()
        isl.append(cur)
        for e in cur.link_edges:
            o = e.other_vert(cur)
            if o.index not in seen:
                seen.add(o.index)
                stack.append(o)
    islands.append(isl)
for i, isl in enumerate(sorted(islands, key=len, reverse=True)):
    xs = [v.co.x for v in isl]; ys = [v.co.y for v in isl]; zs = [v.co.z for v in isl]
    print(f"ISL {i}: verts={len(isl)} x=[{min(xs):.3f},{max(xs):.3f}] y=[{min(ys):.3f},{max(ys):.3f}] z=[{min(zs):.3f},{max(zs):.3f}] c=({sum(xs)/len(xs):.3f},{sum(ys)/len(ys):.3f},{sum(zs)/len(zs):.3f})")
print("ISLANDS_DONE")
