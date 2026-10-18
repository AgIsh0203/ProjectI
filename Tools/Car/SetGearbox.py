# Sets the car's gearbox and engine torque on BP_OffroadCar_Pawn (run in the editor's Python).
# SAVE = False applies them in memory only, for trying values in PIE; True also saves the Blueprint
# (it's an LFS-lockable asset: `git lfs lock` it first, or the save fails as read-only).
#
# History:
# - Template: ratios 4.25 / 2.52 / 1.66 / 1.22 / 1.0, 750 Nm. 1st alone reached ~2200 cm/s and the automatic box
#   (up at 6000 rpm) never left 1st, so shifting didn't matter.
# - 18 Oct: 9.5 / 5.94 / 4.32 / 3.39 / 2.79, up 4700 / down 2600 rpm. PIE tops per gear ~1200 / 1690 / 2135 /
#   2690 / 3000 cm/s. The user: still too fast, the top speed should be much less.
# - 18 Oct, below: taller ratios with less torque (about the same pull at the wheels in each gear, so the launch
#   feels the same) and so less power: lower tops in every gear. PIE tops per gear ~806 / 1055 / 1260 / 1572 /
#   1891 cm/s (was 907 / 1191 / 1451 / 1820 / 2127 with 14.25 / 8.9 / 6.5 / 5.1 / 4.2 and 500 Nm, still too fast).
import unreal

FORWARD = [16.4, 10.2, 7.5, 5.9, 4.85]
MAX_TORQUE = 435.0
CHANGE_UP_RPM = 4700.0
CHANGE_DOWN_RPM = 2600.0
SAVE = True

BP_PATH = "/Game/VehicleTemplate/Blueprints/OffroadCar/BP_OffroadCar_Pawn"

bp_class = unreal.load_class(None, BP_PATH + ".BP_OffroadCar_Pawn_C")
cdo = unreal.get_default_object(bp_class)
movement = cdo.get_components_by_class(unreal.ChaosWheeledVehicleMovementComponent)[0]

setup = movement.get_editor_property("transmission_setup")
setup.set_editor_property("forward_gear_ratios", FORWARD)
setup.set_editor_property("change_up_rpm", CHANGE_UP_RPM)
setup.set_editor_property("change_down_rpm", CHANGE_DOWN_RPM)
movement.set_editor_property("transmission_setup", setup)

engine = movement.get_editor_property("engine_setup")
engine.set_editor_property("max_torque", MAX_TORQUE)
movement.set_editor_property("engine_setup", engine)

blueprint = unreal.load_asset(BP_PATH)
unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
if SAVE:
    unreal.EditorAssetLibrary.save_asset(BP_PATH, only_if_is_dirty=False)

after = movement.get_editor_property("transmission_setup")
unreal.log("SetGearbox: ratios %s, torque %s, up %s, down %s, saved %s" % (list(after.get_editor_property("forward_gear_ratios")),
    movement.get_editor_property("engine_setup").get_editor_property("max_torque"),
    after.get_editor_property("change_up_rpm"), after.get_editor_property("change_down_rpm"), SAVE))
