# ProjectI — "Squirrel Wheels" (working title)

20-day UE 5.7.1 co-op "friend slop" **prototype**: 2–4 tiny squirrels drive ONE human-sized, open-top, falling-apart car. Guiding principle: **viewer fun first, player fun second**, so the game spreads through streams and clips. Full design: `Docs/GDD.md`. Approved plan: `C:\Users\ishan\.claude\plans\for-the-first-project-woolly-wozniak.md`.

## Status (last updated 2026-10-05, end of Day 1)
- **M0 Foundations: mostly done.**
  - Chaos car driven by 2 players (Wheel seat steers, Pedals seat does gas/brake), verified in 2-client PIE.
  - Remaining for M0: package a build, then do the Steam + VOIP test on the user's 2 PCs (host / invite / join / voice).
- **Next: M1 core loop greybox (Thu 8 – Sun 11 Oct).** In order:
  1. Seat placement on the buggy, plus hopping between stations.
  2. `UNBInteractableComponent` with Hold / Mash / TimingRing / Push2 modes.
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
- `Car/NBSeatComponent` — replicated Wheel/Pedals seat; the squirrel attaches to it.
- `Player/NBSquirrel` — character.
  - Enhanced Input is built in code (no input assets): WASD, mouse look, Space, E = interact.
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
- Seat positions on the buggy are rough guesses; the squirrels are tiny and grey, so they're hard to see. Fix in M1.
- The client shows gear 0 / idle RPM (Chaos doesn't replicate engine state to an unpossessed car). Replicate RPM before adding engine audio.
- The seated camera doesn't auto-follow the car's heading yet (feel tweak).
- Possible remaining TSR shimmer on the tires; tune only if the user still sees it.
