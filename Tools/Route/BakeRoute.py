"""Bake the code-built route into an editable map, /Game/Maps/L_Route.

Run inside the editor (Python):  exec(open(r"D:/Games-Unreal/ProjectI/Tools/Route/BakeRoute.py").read())

What it does:
  1. Creates L_Route from L_TestTrack (sky, sun, fog, sky light) without the test-track pieces,
     or reopens it and deletes the previous bake (the "Route" Outliner folder) when REBAKE is True.
  2. Spawns an ANBRoute with terrain on (bUseTerrain: a rough dirt trail over mountains, cliffs and rivers),
     has it build a Landscape (BuildLandscape: material /Game/Maps/M_Terrain, the trail painted on its "Trail"
     layer, /Game/Maps/LI_Trail) and its walls, features, water, hidden pebbles, signs and props in the editor
     (BuildGeometryNow), and copies them into the map under the "Route" Outliner folder (the landscape is
     "Route/Terrain"): each prop
     becomes an ANBProp (mesh + its own collision box), small feature layers (bumps, rocks, pillars,
     ramps, boards, walls) become StaticMeshActors, the big layers (road strip, lines) become one
     ANBInstancedLayer each, and sign texts become TextRenderActors. The tinted basic-shape
     materials become MaterialInstanceConstant assets in /Game/Maps/L_Route_Materials.
  3. Replaces that builder with a placed ANBRoute whose bBuildGeometry is off: it only supplies the
     centreline (progress bar, section banners, finish gate), and the game plays the map as saved.

Re-baking overwrites hand edits to the "Route" folder, INCLUDING ANY SCULPTING OF THE LANDSCAPE; anything you
add outside the folder is kept. After sculpting, run SnapToGround.py instead of re-baking: it drops the props,
boards and markers back onto the ground. Each baked actor carries a "Ground=<cm>" tag, its height above the
ground, for that script.
"""
import unreal

MAP = "/Game/Maps/L_Route"
SOURCE_MAP = "/Game/Maps/L_TestTrack"
MATERIAL_DIR = "/Game/Maps/L_Route_Materials"
TERRAIN_MATERIAL = "/Game/Maps/M_Terrain"
TRAIL_LAYER = "/Game/Maps/LI_Trail"
QUAD_SIZE = 200.0      # cm per landscape quad (4 m halves the landscape's size on disk)
LANDSCAPE_MARGIN = 30000.0  # cm of landscape beyond the road on every side
# Layers ANBRoute builds without collision.
NO_COLLISION_LAYERS = ("Road", "Lines", "Water")
# Props the car knocks over instead of hitting (ANBProp::bKnockable).
KNOCKABLE_MESHES = ("SM_construction_cone",)
# Layers that sit on the road and follow its profile: SnapToGround leaves them alone.
ROAD_LAYERS = ("Road", "Lines", "Bumps", "Rocks", "Pillars", "Hills", "Planks", "Water", "Pebbles", "Boundary")
REBAKE = globals().get("REBAKE", False)
# L_TestTrack actors that belong to the test track, not the route map.
TEST_TRACK_PREFIXES = ("Bump_", "Ramp_", "Floor", "Wall_", "TestPad_", "PlayerStart")

eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assets = unreal.EditorAssetLibrary


def log(msg):
    unreal.log("BAKEROUTE " + msg)


# --- 1. The map -------------------------------------------------------------------------------
if not assets.does_asset_exist(MAP):
    # "Save As" from the test track (duplicate_asset on a map leaves the copy loaded, and opening
    # it then fails the editor's world-leak check).
    les.load_level(SOURCE_MAP)
    for actor in eas.get_all_level_actors():
        if actor.get_actor_label().startswith(TEST_TRACK_PREFIXES):
            eas.destroy_actor(actor)
    unreal.EditorLoadingAndSavingUtils.save_map(unreal.EditorLevelLibrary.get_editor_world(), MAP)
    les.load_level(MAP)
else:
    if not REBAKE:
        raise RuntimeError("L_Route already exists: set REBAKE = True to replace its Route folder (hand edits there are lost)")
    les.load_level(MAP)
    for actor in eas.get_all_level_actors():
        if str(actor.get_folder_path()).startswith("Route") or isinstance(actor, unreal.NBRoute):
            eas.destroy_actor(actor)

# --- 2. Build and copy -------------------------------------------------------------------------
builder = eas.spawn_actor_from_class(unreal.NBRoute, unreal.Vector(0, 0, 0))
builder.set_editor_property("use_terrain", True)
for key, value in globals().get("TERRAIN_PROPS", {}).items():  # tuning without a rebuild
    builder.set_editor_property(key, value)
builder.build_geometry_now()

exec(open(r"D:/Games-Unreal/ProjectI/Tools/Route/MakeTerrainMaterial.py").read())
trail_layer = unreal.NBRoute.create_trail_layer_info(TRAIL_LAYER)
assets.save_asset(TRAIL_LAYER)
landscape = builder.build_landscape(assets.load_asset(TERRAIN_MATERIAL), trail_layer, QUAD_SIZE, LANDSCAPE_MARGIN)
landscape.set_actor_label("Terrain")
landscape.set_folder_path("Route/Terrain")
log("landscape at %s" % landscape.get_actor_location())
for line in builder.check_profile().splitlines():
    log("profile " + line)


def tag_ground(actor):
    """Remember the actor's height above the ground, for SnapToGround.py."""
    loc = actor.get_actor_location()
    tags = [t for t in actor.get_editor_property("tags") if not str(t).startswith("Ground=")]
    actor.set_editor_property("tags", tags + [unreal.Name("Ground=%.1f" % (loc.z - builder.get_ground_height(loc.x, loc.y)))])

tools = unreal.AssetToolsHelpers.get_asset_tools()
mic_cache = {}


def baked_material(material):
    """A dynamic tint can't be saved: turn it into a MaterialInstanceConstant asset per colour."""
    if not isinstance(material, unreal.MaterialInstanceDynamic):
        return material
    color = material.get_vector_parameter_value("Color")
    key = "%02X%02X%02X" % (round(color.r * 255), round(color.g * 255), round(color.b * 255))
    if key not in mic_cache:
        path = MATERIAL_DIR + "/MI_Shape_" + key
        mic = assets.load_asset(path) if assets.does_asset_exist(path) else tools.create_asset(
            "MI_Shape_" + key, MATERIAL_DIR, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
        unreal.MaterialEditingLibrary.set_material_instance_parent(mic, material.get_editor_property("parent"))
        unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(mic, "Color", color)
        mic_cache[key] = mic
    return mic_cache[key]


def instance_transform(component, index):
    result = component.get_instance_transform(index, True)
    return result[1] if isinstance(result, tuple) else result


total = 0
# Big layers (the road strip and its lines) stay one instanced mesh each; a layer of thousands of
# actors ran the editor out of memory and would be miserable to edit.
BIG_LAYER = 300
for component in builder.get_components_by_class(unreal.InstancedStaticMeshComponent):
    layer = component.get_name()
    mesh = component.static_mesh
    count = component.get_instance_count()
    if not mesh or layer == "PropBlockers":
        continue  # ANBProp carries its own collision box
    transforms = [instance_transform(component, i) for i in range(count)]
    if layer.startswith("Prop"):
        # Trees, rocks, fences, lamps...: one ANBProp each, with its collision box fitted to it.
        fraction = builder.get_prop_block_fraction(mesh)
        for i, t in enumerate(transforms):
            prop = eas.spawn_actor_from_class(unreal.NBProp, t.translation, t.rotation.rotator())
            prop.set_actor_scale3d(t.scale3d)
            # Traffic cones get knocked over (a light physics body each) instead of blocking.
            knockable = mesh.get_name() in KNOCKABLE_MESHES
            prop.set_editor_property("knockable", knockable)
            prop.set_prop(mesh, fraction, builder.get_prop_block_at_pivot(mesh))
            prop_mesh = prop.get_editor_property("mesh")
            prop_mesh.set_cast_shadow(component.cast_shadow)
            if not knockable:
                prop_mesh.set_mobility(unreal.ComponentMobility.STATIC)
            prop.set_actor_label("%s_%d" % (mesh.get_name().replace("SM_", ""), i))
            prop.set_folder_path("Route/Props/" + mesh.get_name().replace("SM_", ""))
            tag_ground(prop)
        total += count
        log("%s (%s): %d props, block %.2f" % (layer, mesh.get_name(), count, fraction))
        continue
    materials = [baked_material(component.get_material(i)) for i in range(component.get_num_materials())]
    # Python reads every instanced layer as colliding, so the layers built without collision (ANBRoute::MakeLayer)
    # are named here: the water (the car drove on it) and the road paint. The red corner posts and sign boards
    # (Markers) collide since 18 Oct.
    collides = not layer.startswith(NO_COLLISION_LAYERS)

    def style(smc):
        smc.set_static_mesh(mesh)
        for slot, material in enumerate(materials):
            smc.set_material(slot, material)
        if layer.startswith("Water"):
            # Its own channel that everything ignores (wheels too: ANBCar), saved by name so it survives into the game.
            smc.set_collision_profile_name("Water")
        elif collides:
            smc.set_collision_profile_name("BlockAll")
        else:
            smc.set_collision_profile_name("NoCollision")  # by name: a bare override doesn't reach the game
        smc.set_cast_shadow(component.cast_shadow)

    if count > BIG_LAYER:
        holder = eas.spawn_actor_from_class(unreal.NBInstancedLayer, unreal.Vector(0, 0, 0))
        ism = holder.get_editor_property("instances")
        style(ism)
        ism.add_instances(transforms, False, True)
        holder.set_actor_label("Route_" + layer)
        holder.set_folder_path("Route")
        log("%s: %d instances in one layer" % (layer, count))
    else:
        for i, t in enumerate(transforms):
            actor = eas.spawn_actor_from_class(unreal.StaticMeshActor, t.translation, t.rotation.rotator())
            actor.set_actor_scale3d(t.scale3d)
            # Tag first: setting tags re-runs the actor's construction, which resets its collision.
            if not layer.startswith(ROAD_LAYERS):
                tag_ground(actor)
            smc = actor.static_mesh_component
            style(smc)
            smc.set_mobility(unreal.ComponentMobility.STATIC)
            actor.set_actor_label("%s_%d" % (layer, i))
            actor.set_folder_path("Route/" + layer)
            if layer == "Boundary":
                actor.set_actor_hidden_in_game(True)
        log("%s: %d actors" % (layer, count))
    total += count

signs = 0
for text in builder.get_components_by_class(unreal.TextRenderComponent):
    t = text.get_world_transform()
    actor = eas.spawn_actor_from_class(unreal.TextRenderActor, t.translation, t.rotation.rotator())
    tr = actor.text_render
    tr.set_text(text.get_editor_property("text"))
    tr.set_world_size(text.get_editor_property("world_size"))
    tr.set_text_render_color(text.get_editor_property("text_render_color"))
    tr.set_horizontal_alignment(text.get_editor_property("horizontal_alignment"))
    tr.set_vertical_alignment(text.get_editor_property("vertical_alignment"))
    actor.set_actor_label("Sign_%d" % signs)
    actor.set_folder_path("Route/Signs")
    tag_ground(actor)
    signs += 1

# --- 3. The data-only route --------------------------------------------------------------------
eas.destroy_actor(builder)
route = eas.spawn_actor_from_class(unreal.NBRoute, unreal.Vector(0, 0, 0))
route.set_editor_property("build_geometry", False)
route.set_editor_property("use_terrain", True)
for key, value in globals().get("TERRAIN_PROPS", {}).items():
    route.set_editor_property(key, value)
route.set_actor_label("Route (centreline, finish gate)")

assets.save_directory(MATERIAL_DIR)
les.save_current_level()
log("done: %d meshes, %d signs, %d materials" % (total, signs, len(mic_cache)))
