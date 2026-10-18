# Run in the Unreal editor's Python: makes /Game/Blueprints/M_SquirrelFur, the rigged squirrel's
# material. Like BasicShapeMaterial (a "Color" vector parameter, tinted per player by
# ANBSquirrel::SetPartColor) but usable on skeletal meshes, which BasicShapeMaterial isn't.
# /Game/Blueprints is always cooked, so the C++-only reference survives packaging.
import unreal

PATH, NAME = "/Game/Blueprints", "M_SquirrelFur"
lib = unreal.MaterialEditingLibrary
if unreal.EditorAssetLibrary.does_asset_exist(f"{PATH}/{NAME}"):
    mat = unreal.load_asset(f"{PATH}/{NAME}")
    lib.delete_all_material_expressions(mat)
else:
    mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(NAME, PATH, unreal.Material, unreal.MaterialFactoryNew())
mat.set_editor_property("used_with_skeletal_mesh", True)
color = lib.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -400, 0)
color.set_editor_property("parameter_name", "Color")
color.set_editor_property("default_value", unreal.LinearColor(1.0, 0.35, 0.02, 1.0))
rough = lib.create_material_expression(mat, unreal.MaterialExpressionConstant, -400, 200)
rough.set_editor_property("r", 0.75)
lib.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)
lib.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
lib.recompile_material(mat)
unreal.EditorAssetLibrary.save_loaded_asset(mat)
unreal.log("NBFUR done")
