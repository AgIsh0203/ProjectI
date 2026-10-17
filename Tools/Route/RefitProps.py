"""Refresh L_Route's props in place, keeping every hand edit (position, rotation, scale, added props).

Run inside the editor (Python) with L_Route open:
    exec(open(r"D:/Games-Unreal/ProjectI/Tools/Route/RefitProps.py").read())

For each ANBProp it re-reads the collision settings for its mesh from ANBRoute's prop table (trees and
lamps get their box on the pivot) and refits the box, then makes props, instanced layers and the
baked feature actors Static. Props whose mesh isn't in the table keep their own settings.
"""
import unreal

eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

table = eas.spawn_actor_from_class(unreal.NBRoute, unreal.Vector(0, 0, -100000))
refit = static = 0
for actor in eas.get_all_level_actors():
    if isinstance(actor, unreal.NBProp):
        mesh_component = actor.get_editor_property("mesh")
        mesh = mesh_component.static_mesh
        fraction = table.get_prop_block_fraction(mesh)
        if mesh and fraction > 0.0:
            actor.set_prop(mesh, fraction, table.get_prop_block_at_pivot(mesh))
            refit += 1
        for component in (mesh_component, actor.get_editor_property("blocker")):
            component.set_mobility(unreal.ComponentMobility.STATIC)
        static += 1
    elif isinstance(actor, unreal.NBInstancedLayer):
        actor.get_editor_property("instances").set_mobility(unreal.ComponentMobility.STATIC)
        static += 1
    elif isinstance(actor, unreal.StaticMeshActor) and str(actor.get_folder_path()).startswith("Route"):
        actor.static_mesh_component.set_mobility(unreal.ComponentMobility.STATIC)
        static += 1
eas.destroy_actor(table)
les.save_current_level()
unreal.log("REFITPROPS refit %d props, %d actors static" % (refit, static))
