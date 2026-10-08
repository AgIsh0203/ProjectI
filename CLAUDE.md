# ProjectI — "Squirrel Wheels" (working title)

20-day UE 5.7.1 co-op "friend slop" **prototype**: 2–4 tiny squirrels drive ONE human-sized, open-top, falling-apart car. Guiding principle: **viewer fun first, player fun second**, so the game spreads through streams and clips. Full design: `Docs/GDD.md`. Approved plan: `C:\Users\ishan\.claude\plans\for-the-first-project-woolly-wozniak.md`.

## Status (last updated 2026-10-07)
- **M0 Foundations: done.**
  - Chaos car driven by 2 players (Wheel seat steers, Pedals seat does gas/brake), verified in 2-client PIE.
  - 2-PC Steam test passed on 2026-10-07: host / invite / join, voice works, no physics stutter.
  - That test found ejected squirrels falling through the floor; fixed in `58658fb`, after the 2026-10-06 build. Development builds are packaged to `D:\Games-Unreal\ProjectI_Builds\<date>\Windows\`.
- **Next: M1 core loop greybox (Thu 8 – Sun 11 Oct).** In order:
  1. ~~Seat placement on the buggy, plus hopping between stations.~~ Done (verified in 2-client PIE via Python; the user still needs to eyeball it in play).
  2. ~~`UNBInteractableComponent` with Hold / Mash / TimingRing / Push2 modes.~~ Done. Server logic verified in PIE; the HUD hasn't been seen on screen yet.
  3. ~~4 parts, each with a unique repair: Engine = mash, Brakes = timing ring, Tire = hold outside the car, Door = slam.~~ Done. Verified in PIE via Python; tire clinging and the driving effects still need a hands-on test.
  4. ~~Ejection ragdoll and respawn (8 s).~~ Done. Verified in PIE via Python; how the tumble looks and feels still needs a hands-on check.
  5. ~~Two-squirrel flip-up.~~ Done. The full cycle was verified in PIE via Python; the push still needs a hands-on 2-player check. **M1 is code-complete.**
- **M2 in progress (2026-10-09):** run loop, failure director, deadline, total wreck, acorn cargo + score and end screen are written and compile; **not yet tested in PIE**. Still to do: Steam lobby polish, proximity voice, Acorn-in-Mouth mute + chat wheel.
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
  - Template auto-flip-reset is disabled (`FlipCheckMinDot = -2`). Our own flip-up replaces it:
    - A 0.25 s server timer (`CheckFlipped`) checks the car. If its up axis is below `FlipUpDot` (0.35) and it's slower than 200 cm/s for 1.5 s, it counts as flipped (replicated `bFlipped`).
    - Flipping ejects every rider, zeroes the inputs and closes the seats (`FindNearestFreeSeat` / `FindNextFreeSeat` return null, so respawns land beside the car).
    - It also enables `FlipSpot`: Push2, 2 users, on foot, Range 360, at the middle of the body.
    - Completing the push lifts the car 120 cm and sets it upright, keeping its yaw. If the car rolls back onto its wheels by itself (up axis > 0.8), the flipped state clears.
    - Dev console: `NBFlip` rolls the car onto its roof. The HUD shows a "CAR FLIPPED!" banner.
  - Damage hooks: `SetEnginePowerScale`, `SetBrakePowerScale`, `SetWheelGripScale` (scale the base values cached in BeginPlay).
- `Car/NBSeatComponent` — replicated seat (Wheel / Pedals / Rider); the squirrel attaches to it, so its location is the capsule centre.
  - The car has 5: WheelSeat (perched on the steering wheel), PedalSeat (footwell), HoodSeat (front hood), PassengerSeat, DeckSeat (engine deck). Positions were measured from `SM_Offroad_Body` vertices; the comment in the `ANBCar` constructor lists the landmarks.
  - Hop order is the car's `Seats` array (filled in BeginPlay).
- `Player/NBSquirrel` — character.
  - Enhanced Input is built in code (no input assets): WASD, mouse look, Space, E = interact.
  - On foot, E takes the nearest free seat. Seated, E hops to the next free seat (`HopToNextSeat`) and Space jumps out on the seat's side of the car.
  - Each player gets a unique, replicated fur color (`ColorIndex` → `FurColors`, applied via the BasicShapeMaterial `Color` param).
  - Falling: `Eject(velocity)` / `EjectFromCar()` (the car's velocity plus `EjectKick` out of the seat's side and up).
    - The "ragdoll" detaches `BodyVisual` (the tail rides on it) as a physics ball. Each machine simulates it from the replicated `FNBRagdollState` (start + velocity), and the capsule follows it in Tick.
    - After `RagdollSeconds` (2.5) the server gets the squirrel up where its body landed (`SnapTo` + `Client_SnapTo`, since the owning client runs its own movement).
    - If it isn't seated `RespawnDelay` (8 s) after the fall, `RespawnAtCar` puts it in the first free seat (else beside the car). Entering any seat cancels that.
    - Space in a car going ≥ `BailSpeed` (600 cm/s) ejects instead of hopping out. `FellOutOfWorld` respawns instead of destroying.
    - `RefreshAttachedState` is the single place that sets collision, movement mode and the camera for the riding / tumbling / on-foot states.
- `Player/NBPlayerState` — replicated `Falls` and `Respawns` (M2 turns respawns into a score penalty; the end screen shows falls). Set as `PlayerStateClass`.
  - LMB / gamepad B = action. Press and release go to the server (`Server_ActionPressed` carries the client's server-time estimate), which uses `UNBInteractableComponent::FindBestFor`.
- `Interaction/NBInteractableComponent` — the shared "work on this" spot (repairs, door, flip-up).
  - Modes: Hold, Mash, TimingRing, Push2. Server-authoritative; progress, users, enabled and the sweet spot replicate.
  - A squirrel uses the nearest enabled one within `Range` of its actor location, so seated squirrels reach parts near their seat.
  - The timing ring's phase is `frac(ServerTime / RingPeriod)`. Presses are judged at the client timestamp, clamped to the last 0.3 s.
  - `OnCompleted` fires on the server. With `bDisableOnComplete`, the owner re-enables it.
  - `NBInteractionSubsystem` (a world subsystem) is the registry.
- `UI/NBHUD` — greybox canvas HUD: a pulsing failure list at the top, a "!" marker over each broken part, prompt, progress bar, push count, timing ring with NICE/MISS, and a controls hint. Set as `HUDClass` in the game mode.
- `Parts/NBCarPartComponent` — derives from the interactable: a part is its own repair spot.
  - Replicated `bFailed`. While failed, the interactable is on and the subclass's `ApplyFailedEffect(seconds since failure)` escalates every tick. Completing the interaction repairs it.
  - `Parts/NBCarParts`:
    - Engine (deck, Mash): power fades to 35 % over 10 s, stalls at 16 s.
    - Brakes (hood, TimingRing): 25 %, down to 0 over 12 s.
    - Tire ×4 (outside each wheel, Hold 3 s, `bRequiresOnFoot` + `bAttachUser`, so the squirrel clings and gets dragged): that wheel's grip drops to 30 %.
    - Door (passenger side, Mash 3 = slam): a greybox door panel on `DoorHinge` flaps open on all machines, and every 3 s above 300 cm/s it throws the passenger out (`EjectFromCar`).
  - Part ranges are tuned so the wheel/pedal seats reach nothing; each rider seat reaches exactly one part.
  - Dev console (server): `NBFail Engine|Brakes|Door|TireFL|…|Tire|All`, `NBChaos` toggles a random failure every 12 s, and `NBFlip` rolls the car over. These stand in until M2's failure director.
- `Dev/NBInteractTestPad` — throwaway labelled block with one interactable; re-arms 1.5 s after completing. Four of them (one per mode) sit in L_TestTrack at y = -550.
  - Seated: movement and collision off, the move axis goes to the server via `Server_SetSeatInput`, and the camera boom has collision off and is pulled back.
- `Game/NBRunGameMode` — global default game mode; spawns the car from `/Game/VehicleTemplate/Blueprints/OffroadCar/BP_OffroadCar_Pawn`. That Blueprint is **reparented to ANBCar** and holds the mesh, tire sockets and curves.
- `Game/NBRunGameMode` also runs the run: Waiting (needs 2 squirrels; `NBStart` skips) -> Countdown 5 s -> Driving (150 s deadline) -> Finished (12 s end screen, then `ServerTravel("?Restart")`; does nothing in PIE). Ends on delivery, time up, or a total wreck (3+ parts failed for 10 s). Score = acorns x 10 + 2 per second left - 25 per respawn; bad endings score 0. The car is brake-locked (`SetRunLocked`) outside Driving.
- `Game/NBRunGameState` — replicated phase, result, clock (`GetSecondsLeft`), wreck timer, score breakdown.
- `Game/NBFailureDirector` — component on the game mode. Fails a random part every 14 s -> 6 s (ramps with run progress), up to 1 -> players+1 failed at once, 8 s no-rebreak cooldown after a repair.
- `Game/NBFinishZone` — greybox drop-off box; the run is delivered when the car's centre is inside. If the level has none, the game mode spawns one 80 m ahead of the car.
- `Car/NBAcorn` + cargo on `NBCar`: 40 acorns (replicated). A velocity change > 350 cm/s per 0.1 s spills acorns as physics props; a flip dumps 25 %.
- `Game/NBSessionSubsystem` — game-instance subsystem for Steam lobbies (a stopgap until there's a menu).
  - H / `NBHost` creates a lobby tagged `NBGAME=SquirrelWheels` (so searches on AppID 480 only find ours), then reopens L_TestTrack with `?listen`.
  - J / `NBJoin` finds a lobby and joins it. An accepted Steam invite, or "Join Game" from the friends list, joins automatically.
  - Each step destroys a stale session first. The HUD's top-left line shows OFFLINE / HOST / CONNECTED, the online subsystem and the session status.
- `TP_VehicleAdv/` — imported C++ Vehicle template, compiled inside the ProjectI module. Its duplicate `IMPLEMENT_PRIMARY_GAME_MODULE` was removed; `Build.cs` adds its folders to the include path.
- Map: `/Game/Maps/L_TestTrack` (greybox floor, ramps, bumps). It's the default and startup map.

## Config gotchas (already applied; keep them)
- **Physics substepping ON** (≤8.3 ms, 12 max, MaxPhysicsDeltaTime 0.1). Without it, Chaos tires popped up to ~16 cm at uneven frame rates.
- **Physics Prediction OFF.** Turning it on stops the unpossessed Chaos car from moving: it switches vehicles to the network-prediction input path.
- **The net driver is SteamSockets** (plugin enabled, `bUseSteamNetworking`, relays on). Without Steam (e.g. PIE) it falls back to IpNetDriver.
  - UE 5.7 no longer has `OnlineSubsystemSteam.SteamNetDriver`; the old config silently fell back to IP.
  - Voice is open mic (`bRequiresPushToTalk=false`).
- **Packaging:** `/Game/VehicleTemplate` and `/Engine/BasicShapes` are always cooked, because only C++ constructors reference them.
  - Package with: `RunUAT BuildCookRun -platform=Win64 -clientconfig=Development -build -cook -stage -pak -iostore -archive -archivedirectory=...`
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
