# Drops the cooked triangle-mesh ("complex") collision the game never uses from the template car meshes
# (editor Python, not during PIE; or headless: UnrealEditor-Cmd ProjectI.uproject -run=pythonscript -script=<this file>). Measured from the 20 Oct cook: 4 MB of the meshes' 10 MB.
# - SM_Offroad_Tire (0.97 MB), SM_SportsCar (0.58 MB), SM_SportsCar_Wheel: no collision at all. The tire
#   components on BP_OffroadCar_Pawn are NoCollision, and the roadblocks (ANBProp) draw the sports car without
#   collision and block with their own box.
# - SM_Offroad_Body (2.5 MB): the Chassis component collides (BlockAllDynamic), but physics only uses its one
#   convex hull. Use simple as complex, so complex queries hit that hull too. Nothing in our code traces complex,
#   and the car's wheel traces ignore the car itself.
# The meshes are LFS-lockable: `git lfs lock` them first, or the save fails as read-only.
import unreal

SAVE = True

NO_COLLISION = [
    "/Game/Vehicles/OffroadCar/SM_Offroad_Tire",
    "/Game/Vehicles/SportsCar/SM_SportsCar",
    "/Game/Vehicles/SportsCar/SM_SportsCar_Wheel",
]
SIMPLE_AS_COMPLEX = ["/Game/Vehicles/OffroadCar/SM_Offroad_Body"]

for path in NO_COLLISION + SIMPLE_AS_COMPLEX:
    mesh = unreal.load_asset(path)
    # Property edits go through PostEditChange, which drops the body's cached physics data. (No editor
    # subsystems here: they don't exist in a commandlet.)
    mesh.modify()
    body = mesh.get_editor_property("body_setup")
    body.modify()
    if path in NO_COLLISION:
        body.set_editor_property("agg_geom", unreal.KAggregateGeom())
    body.set_editor_property("collision_trace_flag", unreal.CollisionTraceFlag.CTF_USE_SIMPLE_AS_COMPLEX)
    geom = body.get_editor_property("agg_geom")
    unreal.log("NBStripCollision: %s -> convex %d, boxes %d, simple as complex" % (path, len(geom.get_editor_property("convex_elems")), len(geom.get_editor_property("box_elems"))))
    if SAVE:
        unreal.EditorAssetLibrary.save_loaded_asset(mesh, only_if_is_dirty=False)
