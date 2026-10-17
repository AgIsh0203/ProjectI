"""Create (or recreate) /Game/Maps/M_Terrain, the L_Route landscape's material. No textures, so nothing to licence:
grass on flat ground (a little lighter higher up), dirt on slopes, grey rock on the cliffs (the world normal's Z),
and the dirt trail wherever the landscape's "Trail" layer is painted (BakeRoute.py paints it along the route).

Run inside the editor (Python):  exec(open(r"D:/Games-Unreal/ProjectI/Tools/Route/MakeTerrainMaterial.py").read())
BakeRoute.py runs it on every bake.
"""
import unreal

PATH = "/Game/Maps/M_Terrain"
mel = unreal.MaterialEditingLibrary
assets = unreal.EditorAssetLibrary

if assets.does_asset_exist(PATH):
    material = assets.load_asset(PATH)
    mel.delete_all_material_expressions(material)
else:
    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "M_Terrain", "/Game/Maps", unreal.Material, unreal.MaterialFactoryNew())


def node(cls, x, y, **props):
    expression = mel.create_material_expression(material, cls, x, y)
    for key, value in props.items():
        expression.set_editor_property(key, value)
    return expression


def color(r, g, b, x, y):
    return node(unreal.MaterialExpressionConstant3Vector, x, y, constant=unreal.LinearColor(r, g, b, 1))


# Height tint: grass gets lighter from the valleys (0 m) to the tops (160 m).
world = node(unreal.MaterialExpressionWorldPosition, -1400, -300)
world_z = node(unreal.MaterialExpressionComponentMask, -1200, -300, r=False, g=False, b=True, a=False)
mel.connect_material_expressions(world, "", world_z, "")
height = node(unreal.MaterialExpressionDivide, -1050, -300, const_b=16000.0)
mel.connect_material_expressions(world_z, "", height, "A")
height01 = node(unreal.MaterialExpressionSaturate, -900, -300)
mel.connect_material_expressions(height, "", height01, "")
grass = node(unreal.MaterialExpressionLinearInterpolate, -700, -350)
mel.connect_material_expressions(color(0.12, 0.25, 0.08, -900, -450), "", grass, "A")
mel.connect_material_expressions(color(0.24, 0.36, 0.13, -900, -400), "", grass, "B")
mel.connect_material_expressions(height01, "", grass, "Alpha")

# Slope from the world normal's Z: 1 = flat.
normal = node(unreal.MaterialExpressionVertexNormalWS, -1400, 100)
normal_z = node(unreal.MaterialExpressionComponentMask, -1200, 100, r=False, g=False, b=True, a=False)
mel.connect_material_expressions(normal, "", normal_z, "")


def ramp(low, high, x, y):
    """saturate((normal_z - low) / (high - low)): 0 at or below low, 1 at or above high."""
    shifted = node(unreal.MaterialExpressionSubtract, x, y, const_b=low)
    mel.connect_material_expressions(normal_z, "", shifted, "A")
    scaled = node(unreal.MaterialExpressionMultiply, x + 150, y, const_b=1.0 / (high - low))
    mel.connect_material_expressions(shifted, "", scaled, "A")
    clamped = node(unreal.MaterialExpressionSaturate, x + 300, y)
    mel.connect_material_expressions(scaled, "", clamped, "")
    return clamped


# Grass on the flat (steeper than ~25 degrees turns to dirt), rock on the cliffs (steeper than ~45 degrees).
soil = node(unreal.MaterialExpressionLinearInterpolate, -450, -150)
mel.connect_material_expressions(color(0.31, 0.22, 0.14, -650, -150), "", soil, "A")
mel.connect_material_expressions(grass, "", soil, "B")
mel.connect_material_expressions(ramp(0.86, 0.93, -1050, 0), "", soil, "Alpha")
ground = node(unreal.MaterialExpressionLinearInterpolate, -250, 0)
mel.connect_material_expressions(color(0.36, 0.35, 0.33, -450, 50), "", ground, "A")
mel.connect_material_expressions(soil, "", ground, "B")
mel.connect_material_expressions(ramp(0.62, 0.74, -1050, 200), "", ground, "Alpha")

# The trail: packed dirt wherever the "Trail" layer is painted.
trail = node(unreal.MaterialExpressionLandscapeLayerSample, -450, 300, parameter_name="Trail", preview_weight=0.0)
base = node(unreal.MaterialExpressionLinearInterpolate, -50, 100)
mel.connect_material_expressions(ground, "", base, "A")
mel.connect_material_expressions(color(0.42, 0.32, 0.21, -250, 250), "", base, "B")
mel.connect_material_expressions(trail, "", base, "Alpha")
mel.connect_material_property(base, "", unreal.MaterialProperty.MP_BASE_COLOR)
mel.connect_material_property(node(unreal.MaterialExpressionConstant, -50, 300, r=0.95), "", unreal.MaterialProperty.MP_ROUGHNESS)

mel.recompile_material(material)
assets.save_asset(PATH)
unreal.log("TERRAINMAT saved " + PATH)
