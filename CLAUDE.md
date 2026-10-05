# ProjectI — "Squirrel Wheels" (working title)

20-day UE 5.7.1 co-op "friend slop" **prototype**: 2–4 tiny squirrels drive ONE human-sized, open-top, falling-apart car. Guiding principle: **viewer fun first, player fun second**, so the game spreads through streams and clips. Full design: `Docs/GDD.md`. Approved plan: `C:\Users\ishan\.claude\plans\for-the-first-project-woolly-wozniak.md`.

## Status (last updated 2026-10-05, Day 1 evening)
- **M0 Foundations: mostly done.**
  - Chaos car driven by 2 players (Wheel seat steers, Pedals seat does gas/brake), verified in 2-client PIE.
  - Remaining for M0: package a build, then do the Steam + VOIP test on the user's 2 PCs (host / invite / join / voice).
- **Next: M1 core loop greybox (Thu 8 – Sun 11 Oct).** In order:
  1. ~~Seat placement on the buggy, plus hopping between stations.~~ Done (verified in 2-client PIE via Python; the user still needs to eyeball it in play).
  2. ~~`UNBInteractableComponent` with Hold / Mash / TimingRing / Push2 modes.~~ Done. Server logic verified in PIE; the HUD hasn't been seen on screen yet.
  3. 4 parts, each with a unique repair: Engine = mash, Brakes = timing ring, Tire = hold outside the car, Door = slam.
  4. Ejection ragdoll and respawn (8 s).
  5. Two-squirrel flip-up.
- **Schedule:**
  - M2 (12–16 Oct): route, failure director, deadline, total wreck, acorn cargo + score, Steam lobby, proximity voice, Acorn-in-Mouth mute + chat wheel.
  - M3a (17–20 Oct): Kenney art, VFX/SFX, end-screen stats, private itch page; **friend test Tue 20 Oct**.
  - M4: public free itch demo **Sat 24 Oct**.
- **Budget:** about 65 h total. Weekdays 2–3 h, weekends 5–6 h.

## Locked decisions (don't relitigate)
- **Players and camera:** 2-player minimum, no bots. Third-person chase camera, wider while seated. Arcade feel. Repair juggling is the heart of the game.
- **Score:** acorns delivered (they spill from the open bed) + time bonus.
- **Bad endings:** the deadline runs out, or a total wreck (3+ parts failed for 10 s).
- **Acorn in Mouth:** every 75–105 s, weighted toward the Wheel seat or a squirrel at a failing part. Muted for 20 s, can still hear, has an 8-ping chat wheel.
- **Code quality:** foundation-quality C++ for systems (car, parts, seats, sessions, voice). Throwaway Blueprints/data for content.
- **Online:** Steam OSS, listen server, AppID 480 in dev. Proximity voice via VOIPTalker.
- **Release:** free public itch demo. Steam only after a "go" signal (needs $100 fee + 2-week Coming Soon page).
- **Title:** a squirrel/wheels pun, decided before release. The C++ module stays `ProjectI`.
- **Assets:** Epic template content (the Chaos vehicle) is committed publicly. The user decided this.

## Code map (`Source/ProjectI/`)
- `Car/NBCar` — derives from `ATP_VehicleAdvOffroadCar` (Chaos).
  - Never possessed: the server feeds Chaos from `SetSeatInput`.
  - Physics replication mode is `PredictiveInterpolation`.
  - `SetRequiresControllerForInputs(!HasAuthority())`, so clients use Chaos's `ReplicatedState`. This fixed client wheel jitter.
  - Template auto-flip-reset is disabled (`FlipCheckMinDot = -2`).
  - Damage hooks: `SetEnginePowerScale`, `SetBrakePowerScale`, `SetWheelGripScale` (scale the base values cached in BeginPlay).
- `Car/NBSeatComponent` — replicated seat (Wheel / Pedals / Rider); the squirrel attaches to it, so its location is the capsule centre.
  - The car has 4: WheelSeat (perched on the steering wheel), PedalSeat (footwell), PassengerSeat, DeckSeat (engine deck). Positions were measured from `SM_Offroad_Body` vertices; the comment in the `ANBCar` constructor lists the landmarks.
  - Hop order is the car's `Seats` array (filled in BeginPlay).
- `Player/NBSquirrel` — character.
  - Enhanced Input is built in code (no input assets): WASD, mouse look, Space, E = interact.
  - On foot, E takes the nearest free seat. Seated, E hops to the next free seat (`HopToNextSeat`) and Space jumps out on the seat's side of the car.
  - Each player gets a unique, replicated fur color (`ColorIndex` → `FurColors`, applied via the BasicShapeMaterial `Color` param).
  - LMB / gamepad B = action. Press and release go to the server (`Server_ActionPressed` carries the client's server-time estimate), which uses `UNBInteractableComponent::FindBestFor`.
- `Interaction/NBInteractableComponent` — the shared "work on this" spot (repairs, door, flip-up).
  - Modes: Hold, Mash, TimingRing, Push2. Server-authoritative; progress, users, enabled and the sweet spot replicate.
  - A squirrel uses the nearest enabled one within `Range` of its actor location, so seated squirrels reach parts near their seat.
  - The timing ring's phase is `frac(ServerTime / RingPeriod)`. Presses are judged at the client timestamp, clamped to the last 0.3 s.
  - `OnCompleted` fires on the server. With `bDisableOnComplete`, the owner re-enables it.
  - `NBInteractionSubsystem` (a world subsystem) is the registry.
- `UI/NBHUD` — greybox canvas HUD: prompt, progress bar, push count, timing ring with NICE/MISS, and a controls hint. Set as `HUDClass` in the game mode.
- `Dev/NBInteractTestPad` — throwaway labelled block with one interactable; re-arms 1.5 s after completing. Four of them (one per mode) sit in L_TestTrack at y = -550.
  - Seated: movement and collision off, the move axis goes to the server via `Server_SetSeatInput`, and the camera boom has collision off and is pulled back.
- `Game/NBRunGameMode` — global default game mode; spawns the car from `/Game/VehicleTemplate/Blueprints/OffroadCar/BP_OffroadCar_Pawn`. That Blueprint is **reparented to ANBCar** and holds the mesh, tire sockets and curves.
- `TP_VehicleAdv/` — imported C++ Vehicle template, compiled inside the ProjectI module. Its duplicate `IMPLEMENT_PRIMARY_GAME_MODULE` was removed; `Build.cs` adds its folders to the include path.
- Map: `/Game/Maps/L_TestTrack` (greybox floor, ramps, bumps). It's the default and startup map.

## Config gotchas (already applied; keep them)
- **Physics substepping ON** (≤8.3 ms, 12 max, MaxPhysicsDeltaTime 0.1). Without it, Chaos tires popped up to ~16 cm at uneven frame rates.
- **Physics Prediction OFF.** Turning it on stops the unpossessed Chaos car from moving: it switches vehicles to the network-prediction input path.
- **Motion blur OFF** (`r.DefaultFeature.MotionBlur=False`): it smeared the spinning tires into noise.
- **PIE runs 2 players as a listen server**, from `Config/DefaultEditorPerProjectUserSettings.ini`. The per-user `Saved/` ini can override it.

## Environment and workflow
- **Engine:** source fork `C:\ERODEX` (UE 5.7.1, origin `ishant4iZard/ERODEX`, upstream push disabled).
  - Build: `C:\ERODEX\Engine\Build\BatchFiles\Build.bat ProjectIEditor Win64 Development -Project="D:\Games-Unreal\ProjectI\ProjectI.uproject" -WaitMutex -MaxParallelActions=8`
  - Ignore the "banned MSVC 14.39" note; a newer toolchain is picked.
  - **Close the editor before a full build.** `.cpp`-only edits can use the `live_compile` MCP tool. Header/UFUNCTION changes need close → build → relaunch.
- **MCP:** `mcp-unreal` (Go binary at `C:\Users\ishan\tools\mcp-unreal\mcp-unreal.exe`, registered in `.mcp.json`; plugin in `Plugins/MCPUnreal`, port 8090).
  - Call `status` first. The editor must be running for the editor tools.
  - `execute_script` returns success even when the Python fails: read the results with `get_output_log` (use a unique tag + `pattern`).
  - In PIE, the 2nd client joins a few seconds after `pie_control start`. Check the squirrel count before seating anyone.
  - Get the server world with `[w for w in unreal.EditorLevelLibrary.get_pie_worlds(False) if unreal.GameplayStatics.get_game_mode(w)]`.
  - `LevelEditorPlaySettings` isn't exposed to Python; edit the ini instead.
  - The editor throttles to a few fps when unfocused, so per-frame samplers run slowly.
  - If the MCP server fails to connect (e.g. the editor wasn't running at session start), POST JSON straight to the plugin: `localhost:8090/api/editor/execute_script` `{"script": ...}`, then `/api/editor/output_log` `{"pattern": ..., "max_lines": N}` (or `{"category":"LogPython","verbosity":"error"}` for tracebacks). There's no jq; use node for JSON.
  - `capture_viewport` returns a stale frame while the editor is unfocused. For screenshots, spawn a `SceneCapture2D` in the editor world, `capture_scene()`, then `RenderingLibrary.export_render_target`. Python can't spawn actors into PIE worlds.
- **Machine:** memory is tight when Rider and the editor are both open (32 GB, commit limit ~47 GB). Build Go with `-p 1`.
- **Git** (repo `AgIsh0203/ProjectI`, public, branch `main`, LFS for `.uasset`/`.umap`/media):
  - Commit **as AgIsh0203** (local user config is already set). Never change the global git identity.
  - End commit messages with `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
  - The LFS lock check needs `lfs.<url>.access=basic` + `credential.https://github.com.username=AgIsh0203` (already set).
  - Push with `GCM_INTERACTIVE=never`. PowerShell prints git's stderr as errors; check for `main -> main`.
- **Who does what:**
  - The user: feel/fun calls, 2-PC tests, the itch account, playtesters, downloading Kenney/CC0 packs, final title.
  - Claude: code, config, Blueprints/levels via MCP, builds, tests, commits.

## Open items / ideas parked
- The pedal squirrel sits low in the footwell and may be hidden behind the steering wheel from the chase camera. Check this in play.
- The client shows gear 0 / idle RPM (Chaos doesn't replicate engine state to an unpossessed car). Replicate RPM before adding engine audio.
- The seated camera doesn't auto-follow the car's heading yet (feel tweak).
- Possible remaining TSR shimmer on the tires; tune only if the user still sees it.
