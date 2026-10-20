"""Drop L_Route's props, boards, markers and sign texts back onto the landscape after you sculpt it.

Run inside the editor (Python) with L_Route open:
    exec(open(r"D:/Games-Unreal/ProjectI/Tools/Route/SnapToGround.py").read())

Every actor with a "Ground=<cm>" tag (BakeRoute.py adds one to everything it bakes except the road and the
things on it) is moved up or down so it sits that far above the landscape at its X/Y again. X, Y and
rotation are kept, so hand-placed positions survive. A prop, mesh or text you add inside the "Route" folder gets
a tag with its current height above the landscape the first time, and is snapped on later runs; actors outside
the folder are only touched if you give them a "Ground=<cm>" tag yourself.

The road itself (road strip, lines, bumps, rocks, pillars, ramps, the bridge) follows the route's height
profile, not the landscape: sculpting under the road needs a change in ANBRoute and a re-bake instead.
"""
import unreal

eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

routes = [a for a in eas.get_all_level_actors() if isinstance(a, unreal.NBRoute)]
if not routes:
    raise RuntimeError("No NBRoute in this level: open L_Route first")
route = routes[0]
ROAD_LAYERS = ("Road", "Lines", "Bumps", "Rocks", "Pillars", "Hills", "Planks", "Water", "Pebbles", "Boundary")

moved = tagged = 0
for actor in eas.get_all_level_actors():
    folder = str(actor.get_folder_path())
    if isinstance(actor, (unreal.NBRoute, unreal.Landscape, unreal.NBInstancedLayer)) or folder.startswith(("Route/Terrain",) + tuple("Route/" + l for l in ROAD_LAYERS)):
        continue
    if not isinstance(actor, (unreal.NBProp, unreal.StaticMeshActor, unreal.TextRenderActor)):
        continue
    loc = actor.get_actor_location()
    # Python gets a bare float back (the bool return is folded away), or (found, z) in some versions.
    result = route.get_landscape_height(loc.x, loc.y)
    found, ground = result if isinstance(result, tuple) else (result is not None, result)
    if not found:
        continue
    tags = [str(t) for t in actor.get_editor_property("tags")]
    offset = next((float(t[7:]) for t in tags if t.startswith("Ground=")), None)
    if offset is None:
        if not folder.startswith("Route"):
            continue  # not ours and never tagged: leave it alone
        # Setting tags re-runs the actor's construction, which resets a mesh actor's collision: put it back.
        smc = actor.static_mesh_component if isinstance(actor, unreal.StaticMeshActor) else None
        collision = (smc.get_collision_profile_name(), smc.get_collision_enabled()) if smc else None
        actor.set_editor_property("tags", list(actor.get_editor_property("tags")) + [unreal.Name("Ground=%.1f" % (loc.z - ground))])
        if collision:
            smc.set_collision_profile_name(collision[0])
            smc.set_collision_enabled(collision[1])
        tagged += 1
        continue
    if abs(loc.z - (ground + offset)) > 0.5:
        actor.set_actor_location(unreal.Vector(loc.x, loc.y, ground + offset), False, False)
        moved += 1

les.save_current_level()
unreal.log("SNAPTOGROUND moved %d actors, tagged %d new ones" % (moved, tagged))
