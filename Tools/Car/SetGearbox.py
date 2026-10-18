# Sets the car's gearbox on BP_OffroadCar_Pawn (run in the editor's Python, then the Blueprint is saved).
#
# Why (18 Oct): with the template's ratios (4.25 / 2.52 / 1.66 / 1.22 / 1.0) 1st gear alone reached ~2200 cm/s,
# and the engine runs out of pull near 5100 rpm, so the automatic box (shifting up at 6000) never left 1st and
# a manual crew never needed to shift. The ratios below space the gears so each tops out near:
#   1st ~1000, 2nd ~1600, 3rd ~2200, 4th ~2800, 5th ~3400 cm/s at ~5100 rpm
# (speed = rpm / 60 * 2 pi * 50 cm wheel radius / (gear x final ratio 2.81)),
# and the automatic shift points sit under that, so automatic uses every gear too.
import unreal

FORWARD = [9.5, 5.94, 4.32, 3.39, 2.79]
CHANGE_UP_RPM = 4700.0
CHANGE_DOWN_RPM = 2600.0

BP_PATH = "/Game/VehicleTemplate/Blueprints/OffroadCar/BP_OffroadCar_Pawn"

bp_class = unreal.load_class(None, BP_PATH + ".BP_OffroadCar_Pawn_C")
cdo = unreal.get_default_object(bp_class)
movement = cdo.get_components_by_class(unreal.ChaosWheeledVehicleMovementComponent)[0]
setup = movement.get_editor_property("transmission_setup")
before = list(setup.get_editor_property("forward_gear_ratios"))
setup.set_editor_property("forward_gear_ratios", FORWARD)
setup.set_editor_property("change_up_rpm", CHANGE_UP_RPM)
setup.set_editor_property("change_down_rpm", CHANGE_DOWN_RPM)
movement.set_editor_property("transmission_setup", setup)

blueprint = unreal.load_asset(BP_PATH)
unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
unreal.EditorAssetLibrary.save_asset(BP_PATH, only_if_is_dirty=False)

after = movement.get_editor_property("transmission_setup")
unreal.log("SetGearbox: %s -> %s, up %s, down %s" % (before, list(after.get_editor_property("forward_gear_ratios")),
    after.get_editor_property("change_up_rpm"), after.get_editor_property("change_down_rpm")))
