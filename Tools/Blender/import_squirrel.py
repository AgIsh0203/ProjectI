# Run in the Unreal editor's Python: imports SourceArt/Squirrel/SK_Squirrel.fbx (the rigged squirrel
# from rig_squirrel.py) as a skeletal mesh with its skeleton and every animation, and gives it the
# static squirrel's material. Re-running replaces the assets.
import os

import unreal

TAG = "NBIMPORT"
project = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
fbx = os.path.join(project, "SourceArt/Squirrel/SK_Squirrel.fbx")
dest = "/Game/ThirdParty/PolyPizza/squirrel_n/Rigged"
material = "/Game/ThirdParty/PolyPizza/squirrel_n/Materials/blinn7SG"

ui = unreal.FbxImportUI()
ui.import_mesh = True
ui.import_as_skeletal = True
ui.mesh_type_to_import = unreal.FBXImportType.FBXIT_SKELETAL_MESH
ui.import_animations = True
ui.import_materials = False
ui.import_textures = False
ui.create_physics_asset = False
ui.skeletal_mesh_import_data.set_editor_property("import_morph_targets", False)
ui.skeletal_mesh_import_data.set_editor_property("normal_import_method", unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS)
ui.anim_sequence_import_data.set_editor_property("animation_length", unreal.FBXAnimationLengthImportType.FBXALIT_EXPORTED_TIME)
ui.anim_sequence_import_data.set_editor_property("remove_redundant_keys", False)

# A clean import every time: Interchange (UE 5.7's FBX path) skips the animations when the mesh
# already exists, so the folder is emptied first. Fine while nothing else references these assets;
# once a Blueprint does, delete only the A_Squirrel_ animations and re-import those.
for a in unreal.EditorAssetLibrary.list_assets(dest, recursive=False):
    obj = unreal.load_asset(a)
    unreal.get_editor_subsystem(unreal.AssetEditorSubsystem).close_all_editors_for_asset(obj)
    unreal.EditorAssetLibrary.delete_asset(a)

task = unreal.AssetImportTask()
task.filename = fbx
task.destination_path = dest
task.destination_name = "SK_Squirrel"
task.automated = True
task.replace_existing = True
task.save = True
task.options = ui
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
for p in task.imported_object_paths:
    unreal.log(f"{TAG} imported {p}")

mesh = unreal.load_asset(f"{dest}/SK_Squirrel")
if mesh:
    mat = unreal.load_asset(material)
    mats = mesh.get_editor_property("materials")
    for m in mats:
        m.set_editor_property("material_interface", mat)
    mesh.set_editor_property("materials", mats)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    skel = mesh.get_editor_property("skeleton")
    bounds = mesh.get_bounds()
    unreal.log(f"{TAG} mesh bones={mesh.get_num_bones() if hasattr(mesh, 'get_num_bones') else '?'} skel={skel.get_path_name() if skel else None} origin={bounds.origin} extent={bounds.box_extent}")
for a in unreal.EditorAssetLibrary.list_assets(dest, recursive=False):
    if "/SK_SquirrelArmature_" in a:
        new = a.split(".")[0].replace("/SK_SquirrelArmature_", "/A_Squirrel_")
        unreal.EditorAssetLibrary.rename_asset(a.split(".")[0], new)
        a = new
    obj = unreal.load_asset(a)
    if isinstance(obj, unreal.AnimSequence):
        obj.set_editor_property("enable_root_motion", False)
        unreal.EditorAssetLibrary.save_loaded_asset(obj)
        unreal.log(f"{TAG} anim {a} length={obj.get_play_length():.2f}s")
unreal.log(f"{TAG} done")
