# Prints what a model file contains: objects, vertex counts, bounds, materials.
# blender --background --factory-startup --python inspect_mesh.py -- <model.glb|.fbx|.obj>
import sys
import bpy
from mathutils import Vector

path = sys.argv[sys.argv.index("--") + 1]
bpy.ops.wm.read_factory_settings(use_empty=True)
ext = path.lower().rsplit(".", 1)[-1]
if ext in ("glb", "gltf"):
    bpy.ops.import_scene.gltf(filepath=path)
elif ext == "fbx":
    bpy.ops.import_scene.fbx(filepath=path)
elif ext == "obj":
    bpy.ops.wm.obj_import(filepath=path)

for ob in bpy.data.objects:
    line = f"OBJ {ob.name} type={ob.type} parent={ob.parent.name if ob.parent else None} loc={tuple(round(v, 3) for v in ob.location)} rot={tuple(round(v, 3) for v in ob.rotation_euler)} scale={tuple(round(v, 3) for v in ob.scale)}"
    print(line)
    if ob.type == "MESH":
        me = ob.data
        pts = [ob.matrix_world @ v.co for v in me.vertices]
        lo = Vector((min(p.x for p in pts), min(p.y for p in pts), min(p.z for p in pts)))
        hi = Vector((max(p.x for p in pts), max(p.y for p in pts), max(p.z for p in pts)))
        print(f"  verts={len(me.vertices)} faces={len(me.polygons)} min={tuple(round(v, 3) for v in lo)} max={tuple(round(v, 3) for v in hi)}")
        print(f"  materials={[m.name for m in me.materials if m]} uv={len(me.uv_layers)} colors={len(me.color_attributes)}")
        # Vertex histogram along each axis: shows where the body is thin (joints).
        for axis in range(3):
            n = 20
            span = hi[axis] - lo[axis]
            bins = [0] * n
            for p in pts:
                bins[min(n - 1, int((p[axis] - lo[axis]) / span * n))] += 1
            print(f"  axis{'XYZ'[axis]} hist={bins}")
print("INSPECT_DONE")
