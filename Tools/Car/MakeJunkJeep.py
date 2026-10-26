# Run in the Unreal editor's Python. Builds the junk jeep (A2) from Tools/Blender/build_junkjeep.py's output:
#   1. imports SourceArt/JunkJeep/SM_JunkJeep_*.fbx into /Game/ThirdParty/PolyPizza/JunkJeep; the body keeps
#      its UCX_ box (the tub: the template welds the Chassis mesh's collision into the car), the door,
#      engine and wheels get none,
#   2. makes (or updates) DA_JunkJeepBody there, the UNBCarBody with the layout below,
#   3. makes (or updates) BP_JunkJeep, a child of BP_OffroadCar_Pawn with CarBody = DA_JunkJeepBody, next to
#      it in the always-cooked OffroadCar folder. The game mode spawns BP_JunkJeep when it exists.
# Rerun after changing the layout. REIMPORT = False keeps the meshes and only redoes 2 and 3.
import os

import unreal

TAG = "NBJEEP"
REIMPORT = True
project = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
src = os.path.join(project, "SourceArt/JunkJeep")
dest = "/Game/ThirdParty/PolyPizza/JunkJeep"
car_dir = "/Game/VehicleTemplate/Blueprints/OffroadCar"
MESHES = ["SM_JunkJeep_Body", "SM_JunkJeep_Door", "SM_JunkJeep_Wheel", "SM_JunkJeep_Engine"]

# Layout in the car's mesh space (cm; x forward, y right, z up), matching SourceArt/JunkJeep/junkjeep_layout.json.
# Seats are the squirrel's capsule centre, 22 cm above what it sits on. Each part is in reach of one seat only
# (Range below): the hood seat fixes the battery engine, the back seat the sponge brake, the Gears seat the door.
ENGINE_AT = unreal.Vector(165.0, 10.0, 160.0)  # layout["engine"]["origin"]
LOCATIONS = {
    "WheelSeat": (23, 57, 202),       # on the steering wheel (the jeep is right-hand drive)
    "PedalSeat": (55, 57, 95),        # the driver's footwell
    "PassengerSeat": (-60, -47, 132), # the Gears seat: the passenger seat, by the cardboard door
    "HoodSeat": (110, -60, 188),      # on the hood beside the battery
    "DeckSeat": (-150, 95, 188),      # the back, beside the acorn crate, over the sponge brake
    "EnginePart": (165, 10, 200),     # the car battery on the hood
    "BrakePart": (-185, 120, 100),    # the sponge behind the rear-right tyre
    "DoorPart": (-60, -135, 110),     # the cardboard flap
    "DoorHinge": (-26, -135, 110),    # layout["door"]["hinge"]
    "EngineAudio": (165, 10, 180),
}
RANGES = {"EnginePart": 110.0, "BrakePart": 110.0, "DoorPart": 110.0}
ACORN_CORNER = unreal.Vector(-230.0, -39.0, 147.0)  # in the cardboard crate across the back

tools = unreal.AssetToolsHelpers.get_asset_tools()
eal = unreal.EditorAssetLibrary

if REIMPORT:
    tasks = []
    for name in MESHES:
        ui = unreal.FbxImportUI()
        ui.import_mesh = True
        ui.import_as_skeletal = False
        ui.mesh_type_to_import = unreal.FBXImportType.FBXIT_STATIC_MESH
        ui.import_materials = True
        ui.import_textures = False
        ui.static_mesh_import_data.set_editor_property("auto_generate_collision", False)
        ui.static_mesh_import_data.set_editor_property("one_convex_hull_per_ucx", True)
        ui.static_mesh_import_data.set_editor_property("combine_meshes", True)
        ui.static_mesh_import_data.set_editor_property("normal_import_method", unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS)
        task = unreal.AssetImportTask()
        task.filename = os.path.join(src, name + ".fbx")
        task.destination_path = dest
        task.destination_name = name
        task.automated = True
        task.replace_existing = True
        task.save = True
        task.options = ui
        tasks.append(task)
    tools.import_asset_tasks(tasks)
    for t in tasks:
        unreal.log(f"{TAG} imported {list(t.imported_object_paths)}")

meshes = {n: unreal.load_asset(f"{dest}/{n}") for n in MESHES}
# One box round the tub (not the roll bar, windscreen or props), simple-as-complex like the buggy body's
# hull: the template welds the Chassis mesh's collision into the car, so it's what the body hits things with.
# (UE 5.7's Interchange FBX import drops build_junkjeep.py's UCX_ box, so it's set here from the same numbers.)
TUB_MIN, TUB_MAX = (-245, -125, 63), (230, 118, 166)  # layout["collision"]
box = unreal.KBoxElem()
box.set_editor_property("center", unreal.Vector(*[(a + b) / 2 for a, b in zip(TUB_MIN, TUB_MAX)]))
box.set_editor_property("x", TUB_MAX[0] - TUB_MIN[0])
box.set_editor_property("y", TUB_MAX[1] - TUB_MIN[1])
box.set_editor_property("z", TUB_MAX[2] - TUB_MIN[2])
setup = meshes["SM_JunkJeep_Body"].get_editor_property("body_setup")
agg = setup.get_editor_property("agg_geom")
agg.set_editor_property("box_elems", [box])
agg.set_editor_property("convex_elems", [])
setup.set_editor_property("agg_geom", agg)
setup.set_editor_property("collision_trace_flag", unreal.CollisionTraceFlag.CTF_USE_SIMPLE_AS_COMPLEX)
meshes["SM_JunkJeep_Body"].modify()
eal.save_loaded_asset(meshes["SM_JunkJeep_Body"])
for n, m in meshes.items():
    if not m:
        unreal.log_error(f"{TAG} missing {n}")
        continue
    b = m.get_bounds()
    agg = m.get_editor_property("body_setup").get_editor_property("agg_geom")
    hulls = len(agg.get_editor_property("convex_elems")) + len(agg.get_editor_property("box_elems"))
    unreal.log(f"{TAG} {n} origin={b.origin} extent={b.box_extent} collision_shapes={hulls}")

# --- The car body data asset.
body_path = f"{dest}/DA_JunkJeepBody"
body = unreal.load_asset(body_path) if eal.does_asset_exist(body_path) else None
if not body:
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", unreal.NBCarBody)
    body = tools.create_asset("DA_JunkJeepBody", dest, unreal.NBCarBody, factory)
body.set_editor_property("body_mesh", meshes["SM_JunkJeep_Body"])
body.set_editor_property("wheel_mesh", meshes["SM_JunkJeep_Wheel"])
body.set_editor_property("door_mesh", meshes["SM_JunkJeep_Door"])
body.set_editor_property("engine_mesh", meshes["SM_JunkJeep_Engine"])
body.set_editor_property("engine_transform", unreal.Transform(location=ENGINE_AT))
body.set_editor_property("locations", {unreal.Name(k): unreal.Vector(*v) for k, v in LOCATIONS.items()})
body.set_editor_property("ranges", {unreal.Name(k): v for k, v in RANGES.items()})
body.set_editor_property("acorn_pile_corner", ACORN_CORNER)
eal.save_loaded_asset(body)
unreal.log(f"{TAG} body {body.get_path_name()}")

# --- BP_JunkJeep: the buggy Blueprint's child with the body on.
bp_path = f"{car_dir}/BP_JunkJeep"
bp = unreal.load_asset(bp_path) if eal.does_asset_exist(bp_path) else None
if not bp:
    parent = unreal.load_asset(f"{car_dir}/BP_OffroadCar_Pawn")
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent.generated_class())
    bp = tools.create_asset("BP_JunkJeep", car_dir, unreal.Blueprint, factory)
cdo = unreal.get_default_object(bp.generated_class())
cdo.set_editor_property("car_body", body)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
eal.save_loaded_asset(bp)
cdo = unreal.get_default_object(bp.generated_class())
unreal.log(f"{TAG} bp {bp.get_path_name()} car_body={cdo.get_editor_property('car_body')}")
unreal.log(f"{TAG} done")
