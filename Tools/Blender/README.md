# Squirrel rig and animations (Blender, headless)

Everything runs from the command line; Blender doesn't need to be open and no add-on is needed.
`B` below is `C:\Program Files\Blender Foundation\Blender 5.2\blender.exe`.

| Script | Runs in | Does |
|---|---|---|
| `rig_squirrel.py` | Blender | Rigs a model with the shared squirrel skeleton (heat weights, 4 influences), adds the animations from `squirrel_anims.py`, saves a `.blend` and the FBX for Unreal |
| `squirrel_anims.py` | (imported) | The animations, as functions of the loop phase; the comment at the top explains the rotation axes |
| `check_clipping.py` | Blender | How far the paws and tail go into the head, per animation (aim for 0) |
| `render_views.py` | Blender | Orthographic previews (side, 3/4, front, back, top) with the bones drawn; `weights` instead of an action colours faces by bone |
| `contact_sheet.py` | Blender | Tiles previews into one image |
| `inspect_mesh.py`, `list_islands.py` | Blender | Size, vertex spread and loose parts of a new model |
| `import_squirrel.py` | Unreal editor Python | Re-imports the FBX into `/Game/ThirdParty/PolyPizza/squirrel_n/Rigged/` (mesh, skeleton, `A_Squirrel_<Name>`) |
| `make_fur_material.py` | Unreal editor Python | Makes `/Game/Blueprints/M_SquirrelFur` |

## Change an animation
1. Edit `squirrel_anims.py`.
2. `"$B" --background --factory-startup --python Tools/Blender/rig_squirrel.py -- SourceArt/Squirrel/squirrel_n.rig.json SourceArt/Squirrel/SK_Squirrel.blend SourceArt/Squirrel/SK_Squirrel.fbx`
3. `"$B" --background --python Tools/Blender/check_clipping.py -- SourceArt/Squirrel/SK_Squirrel.blend`
4. Look at it: `render_views.py -- <blend> <out_prefix> <Action> 0,3,6,9`, then `contact_sheet.py`.
5. In the editor (nothing open on the squirrel assets): run `import_squirrel.py`. It empties the Rigged folder first, since UE 5.7's Interchange import skips the animations when the mesh already exists.

A new animation also needs a `UAnimSequence` slot and a line in `ANBSquirrel::UpdateModelAnimation`.

## A new or extra model
Put the model next to a copy of `squirrel_n.rig.json`, set `model`, and move the joints (model metres, x forward, z up; left side only, the right mirrors) until the `render_views.py` previews show the bones inside the right parts. Same bone names, so the Unreal skeleton and animations carry over. Credit the model in `CREDITS.md`.
