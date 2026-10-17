# Terrain plan

Status: **planned 18 Oct, not started.** Asked for in friend test 1 ("make the route more based on terrain instead of flat with some jolts"). The user chose:
- **A real Unreal Landscape in L_Route that the road follows**, not hills only beside a flat road.
- **Rolling hills:** terrain up to ~20 m high, road grades up to ~8 %.

Estimate: 6–10 h. The demo is Sat 24 Oct; if this runs long, cut step 7 (river) and keep the old box hills.

## Why the map is flat today
`ANBRoute` puts everything at z = 0: a walled ground slab is the only surface the wheels touch, the road strip and lines are visual pieces 1 cm above it, and the hills, bridge and jumps are boxes on top (`ENBRouteFeature::Hill/Bridge/Jump`). `Tools/Route/BakeRoute.py` copies that into L_Route, so L_Route is flat too.

## Design

### 1. One height model, in C++ (deterministic, so every machine and every bake agrees)
On `ANBRoute`:
- **`TerrainHeight(X, Y)`:** natural ground. A sum of 3 octaves of `FMath::PerlinNoise2D`: ~12 m at ~600 m wavelength, ~6 m at ~250 m, ~2 m at ~90 m, shifted and clamped to 0–20 m. A fixed seed offset, like the rocks.
- **`RoadHeight(Distance)`:** the road's profile along the centreline.
  1. Sample `TerrainHeight` every 5 m along `SamplePoints`.
  2. Smooth it with a moving average of about 80 m.
  3. Limit the grade with a forward and a backward pass so |dz/dd| ≤ 8 %. Stricter zones: ≤ 2 % for the first 60 m of THE PARK (the car spawns there), the last 120 m (finish gate, depot), and 30 m either side of each jump kicker; ≤ 4 % through THE HAIRPIN and SQUIRREL SLALOM; level across THE BRIDGE.
  4. Hill features (ACORN HILL, THE HUMPS) become smooth bumps added to the profile (cosine shape, the same heights as today) instead of box slabs with sheer sides. ACORN HILL's "falling off the side flips the car" goes away; keep the box version if that matters.
- **`GroundHeight(X, Y)`:** what the landscape actually gets. `RoadHeight` across the road plus 6 m shoulders each side, blending to `TerrainHeight` over the next 25 m. Use the nearest-sample search (`DistanceToRoad` already does it) to get the road distance for a point. Where two stretches of road are within blend range of each other (the hairpin legs come within ~90 m), take the road nearest in height, or just the nearest.
- Expose `GetRoadHeight(Distance)` and `GetGroundHeight(X, Y)` as BlueprintCallable for the bake scripts and the checker.

### 2. Road geometry follows the profile
- `GetTransformAt(Distance, Height)` and `SampleAt` add `RoadHeight(Distance)` to Z and pitch by the local grade. Everything that places itself along the road (road strip pieces, lines, corner markers, signs, the finish gate, the car spawn, `NBWarp`, the player starts) goes through these.
- Road strip and line pieces: pitched to the grade and about 3 % longer so slopes leave no gaps. They stay visual-only, 2 cm above the ground; the wheels drive on the landscape, which is flattened to the profile under the road (section 1).
- The ground slab is not built when terrain is on (a new `bUseTerrain`, default off so the runtime route in L_TestTrack stays flat). The perimeter walls become a tall fence following `GroundHeight`, or simply 30 m tall.
- Bumps, rocks, slalom pillars and jump kickers sit at `RoadHeight` and take the road's pitch.
- Bridge: the deck is level at the profile height. See step 7 for the river.

### 3. The landscape (editor-only C++, called by the bake)
Python can't create a landscape, so add `ANBRoute::BuildLandscape()`, editor-only (`#if WITH_EDITOR`, `"Landscape"` in the module's dependencies):
- Size: the route's extent is about 2.8 × 1.7 km. Add a 300 m margin and use **2 m per quad**: roughly 1650 × 1150 vertices. Components of 127 quads with 1 section each, about 13 × 9 = 117 components. Location and scale (X/Y 200, Z 50) so 32768 = z 0 and the range covers ±128 m.
- Heights: `GroundHeight` at each vertex, converted with `Value = 32768 + z_cm × 128 / ScaleZ`.
- `ALandscape::Import(...)` builds it. **Check that function's 5.7 signature in `C:\ERODEX\Engine\Source\Runtime\Landscape\Classes\Landscape.h` first;** it has changed between engine versions. Look at how `LandscapeEditorUtils` / the "Import from file" tool call it.
- Material `M_Terrain` in `/Game/Maps/` (Python `MaterialEditingLibrary`): grass green on flat ground blending to dirt brown on slopes (`VertexNormalWS.z` → Lerp), no textures, so nothing is added to the asset licence list.
- Landscape collision stays on (Chaos wheels use it). No Nanite, no grass or foliage types.

### 4. Bake changes (`Tools/Route/BakeRoute.py`)
- Call `BuildLandscape()` before copying geometry. On `REBAKE`, delete the old landscape first. **A rebake overwrites sculpting**: say so in the script's docstring and in CLAUDE.md.
- Props: drop each one onto the ground (`GetGroundHeight`, or a line trace once the landscape exists), sunk 10 cm. Trees stay upright; rocks and logs tilt to the ground normal.
- New `Tools/Route/SnapToGround.py`: after you sculpt, it re-drops every prop, board and feature actor onto the landscape with a downward trace, keeping X/Y and yaw. That way hand sculpting doesn't need a rebake.

### 5. Runtime and gameplay checks
- `GetDistanceAlong` is XY-only and stays as it is.
- Spills: hill crests make the car go light. Check that crests at 20 m/s don't spill the whole pile (`SpillMinDeltaV` 350); landings off crests count as jumps (≥ 0.5 s airborne), which is fun and fine.
- Engine: an 8 % climb on a failed engine (35 % power, then a stall) should become a crawl, which is the drama we want. Check that a healthy car still makes about 12 m/s uphill.
- Flip detection (up axis < 0.35) is unaffected by 8 % grades.
- Brakes: a downhill with failed brakes is the point. Check the brake grab doesn't flip the car on a slope.

### 6. Verification
- **Profile checker:** extend `route_check.js` (scratchpad; keep a copy in `Tools/Route/`), or a Python call into `GetRoadHeight`, to print the max grade per section, the min and max height, and the strict zones.
- **Autopilot drive in PIE:** a Python probe that steers toward the centreline sample 20 m ahead at a target speed, driving the whole route. Log time per section, the slowest speed on each climb, airtime events and acorns lost per section. It must finish with no stuck points.
- Screenshots from SceneCapture views (`route_start`, `high`, plus one per hilly section).
- 2-instance packaged run (as in the restart repro: `?AutoStart`, `-nosteam`), to check the landscape cooks and replicates and the client's car sits on the ground.
- **Package size:** the landscape plus its collision might add 20–60 MB. Measure it. The rule is a reason for anything over 20 MB; if it's too big, go to 4 m per quad.

### 7. Optional: a river under THE BRIDGE
Lower `GroundHeight` into a 6 m-deep channel under the bridge span, keep the existing water plane at the channel's floor plus 1 m, and keep the bridge deck level. Falling off the bridge then means water, a respawn and a funny clip.

## Order of work
1. Height model + profile + checker (no visuals yet). Tune until grades and strict zones pass.
2. Road geometry from the profile (works in the runtime route too, but without a landscape there's nothing under the wheels: test that only in step 4).
3. `BuildLandscape` + `M_Terrain`.
4. Bake: landscape, props dropped, `SnapToGround.py`. Rebake L_Route (lock it in LFS first: `git lfs lock Content/Maps/L_Route.umap`).
5. Autopilot drive, tune `SpillMinDeltaV` / grades, screenshots.
6. Packaged 2-instance check and size.
7. River (optional).
8. Docs: GDD route section (heights per section), CLAUDE.md code map, Playtest checks ("do the hills feel good, is a climb with a broken engine too slow"), Roadmap.

## Known risks
- `ALandscape::Import` is editor API with a signature that moves between versions; budget an hour to get the first landscape to appear.
- A landscape in a non–World Partition map is fine at this size; don't convert the map to World Partition.
- The hairpin legs and WIGGLY ROAD bends are close together, so their corridors overlap. The blend must pick one road height, or there will be a ridge between them.
- Rebakes erase sculpting; prefer `SnapToGround.py` after hand edits.
