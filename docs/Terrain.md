# Terrain

Status: **built 18 Oct; checked in single-player PIE only.** Asked for in friend test 1 ("make the route more based on terrain instead of flat with some jolts").

## What the user chose (18 Oct)
The first plan was rolling hills (≤ 20 m) with a smooth road at ≤ 8 %. Mid-way the user redirected:
- **Like "RV There Yet?"**: rugged mountains, cliffs, steep climbs, river crossings.
- **No pushing or winching**: squirrels can't push a car.
- **River crossings with pebbles nobody can see.** A squirrel can't hop out in a river to fix a tyre: it gets **swept away**.
- **The trail itself is rough**: lumps, sideways lean, dirt, no lane lines.
- There is **no 24 Oct demo any more**: testers liked the last build, and more testers next week will play that package.

## How it works (`Game/NBRoute`, `bUseTerrain`)
`bUseTerrain` is on in L_Route and off by default, so the runtime route on L_TestTrack stays flat. Everything comes from one deterministic height model in C++. Every machine and every bake gets the same numbers.

- **Mountains (`TerrainZ`):** 4 octaves of Perlin noise plus a ridge term, 0–`TerrainMaxHeight` (160 m). Benches `TerraceHeight` (25 m) tall with steep faces between them make the cliffs (`TerraceStrength` 0.45). A slow noise shifts the steps, so the faces wander instead of following the contours.
- **Trail profile (`BuildProfile`, heights in the centreline samples' Z):**
  1. The mountains under the centreline, averaged over `ProfileSmoothing` (30 m).
  2. ACORN HILL and THE HUMPS add cosine bumps.
  3. Grades are limited in both directions:
     - `MaxGrade` 18 % in general.
     - 6 % in THE PARK (tutorial).
     - 2 % for the first 60 m and the last 120 m.
     - 3 % within 30 m of the jumps.
     - 10 % in THE HAIRPIN and SQUIRREL SLALOM.
     - Level across THE BRIDGE.
  4. Fords are pinned (see Rivers below) and the limits run again.
  - `CheckProfile()` prints each section's steepest climb and descent and its height range, the rivers, and any step over its limit (it should say 0).
- **Ground (`SampleGround`, what the landscape gets):**
  - The trail's height across the trail and `ShoulderWidth` (2 m), blending to the mountains over `BlendWidth` (15 m). That short blend leaves a rock wall on the uphill side and a drop on the other.
  - Where two stretches of trail are within reach (the hairpin, the wiggles), they share the ground.
  - **Rough trail:** the trail surface leans sideways up to `TrailCamber` (5 %) and has lumps of ± `TrailRoughness` (15 cm). The cobbled sections get 2.5× the lumps and the fords 1.5×. The start, the end, the jumps and the bridge stay smooth.
  - **Trail paint:** a 0–1 weight on the landscape's "Trail" layer (`/Game/Maps/LI_Trail`).
- **Rivers (`FRiver`):** one crosses each Ford segment (SQUIRREL CREEK at 0.46 km, THE WASH at 5.14 km) and one runs under THE BRIDGE.
  - Each starts straight across the trail and then wanders. It runs up to `RiverReach` (300 m) each way, and stops before any other stretch of trail.
  - The water is level at the crossing. Downstream it follows the ground down; upstream it climbs gently and starts where the ground stops rising above it. The water boxes are tilted piece by piece, so it flows downhill.
  - The channel is `RiverHalfWidth` (14 m) plus `RiverBank` (6 m) and `RiverDepth` (2.5 m) deep. Low ground beside it is raised to just above the water, so the water never floats over a dip.
  - **A ford** is the trail's lowest point near the crossing. The trail across the channel is pinned `FordDepth` (60 cm) under the water, and both approaches come down into it.
  - **The bridge's river** runs `BridgeClearance` (9 m) under the deck, in a gorge.
- **Water (`GetWaterDepth`)** answers how deep the water is at a point and which way it flows.
  - A squirrel on foot with ≥ `SweepDepth` (30 cm) at its feet is **swept away**: it ragdolls downstream at `SweepSpeed` and is back in a seat after `SweptRespawnDelay` (2.2 s, less than the ragdoll time, so it never stands up in the river again). Logs `NBRiver: … swept away`.
  - The car in water loses speed (`RiverDrag`) and every tyre wears (`RiverTireWearRate`), so tyres tend to go flat mid-river, where nobody can fix them.
  - 18 hidden **pebbles** (collision boxes) sit under the opaque water at each ford.
- **Landscape (`BuildLandscape`, editor only, called by the bake):**
  - 2 m quads, 127-quad components (14 × 9), 300 m beyond the trail on every side. Z scale 100 (±256 m).
  - Heights from `SampleGround`; the trail weights go into the "Trail" layer.
  - Material `/Game/Maps/M_Terrain` (`Tools/Route/MakeTerrainMaterial.py`, no textures):
    - grass, lighter higher up;
    - dirt on slopes, rock on cliffs (world normal);
    - the dirt track where "Trail" is painted.
- **What changed on the road with terrain:**
  - No asphalt strip, lines or start line.
  - The cobble ridges are lumps in the trail now.
  - Rocks, pillars, markers, signs and props stand on the ground.
  - Props never sit in a river or on a slope steeper than about 37°.
  - The boundary walls are invisible and 185 m tall.

## Tools
- `Tools/Route/BakeRoute.py` (`REBAKE = True`) rebuilds everything in the Route folder, **including the landscape: a rebake erases any sculpting**. Every baked prop, board and marker carries a `Ground=<cm>` tag (its height above the ground).
- `Tools/Route/SnapToGround.py`: after sculpting, it drops the tagged actors back onto the landscape (keeping X/Y and rotation). The trail itself follows the profile in code: change it there and rebake.
- `NBAuto [speed]` (console) or `?Autopilot=<cm/s>` (URL): the server drives the car down the centreline, with no random failures. It slows for bends and backs out when stuck, logging per section ("NBAuto: …"): time, speed range, slowest climb, max tilt, acorns lost, air points, stuck count. Before a scripted drive in PIE, seat the local squirrel (a rider seat) so the person at the PC can watch.

## Still to check
- Two editor GPU hangs during the long autopilot drives (18 Oct), both inside UE's software variable-rate shading on the async compute queue. With it off (CLAUDE.md, Config gotchas) the next full drive ran 8 minutes without a hang. One clean run is evidence, not proof: if it happens again, check the breadcrumbs for every pipeline.
- Hands-on: are 18 % climbs fun or a slog? Is a broken engine on a climb a fun crisis or a dead stop? Do the fords' swept-away squirrels read on screen?
- Packaged 2-instance run and the package size (the landscape is ~2 M vertices with collision).
- Rocks and pillars are still grey boxes; the water is an opaque box per 10 m.
