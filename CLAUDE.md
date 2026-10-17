# ProjectI — "Squirrel Wheels" (working title)

20-day UE 5.7.1 co-op "friend slop" **prototype**: 2–4 tiny squirrels drive ONE human-sized, open-top, falling-apart car. Guiding principle: **viewer fun first, player fun second**, so the game spreads through streams and clips.

## Docs
- `docs/Roadmap.md` — **before picking up new work or packaging**: the schedule to the 24 Oct demo, the ranked backlog (items are cited as #N), the package-size budget and how to meet it, and the decisions waiting on the user.
- `docs/GDD.md` — **design questions**: what each feature is, its status (Built / Friend test / Demo / Later), and the tuning table where playtested values get recorded.
- `docs/Assets.md` — **before importing, assigning or committing any asset**: licence rules, folders, the slot map, sources.
- `docs/Playtest.md` — **before a packaged build or a 2-PC test**: everything that still needs a hands-on check.
- `CREDITS.md` — **when an asset is imported**: add its line.
- The approved plan (`C:\Users\ishan\.claude\plans\for-the-first-project-woolly-wozniak.md`) holds the original scope and success test. The Roadmap replaces its schedule, and the code map below replaces its technical sketch.

## Status (last updated 2026-10-13)
- **M0, M1 and M2 are code-complete** (M2 on 2026-10-09, three days early). Built so far:
  - the Chaos car, its 5 seats and hopping;
  - 4 parts with distinct repairs, ragdoll + respawn, flip-up;
  - the run loop, failure director, deadline, total wreck, acorn cargo + score, end-screen stats with MVP / Most useless;
  - the code-built route, lobby menu, proximity voice, Acorn in Mouth + pings, and sound/VFX slots.
- **Only M0 has been played by hand** (2-PC Steam test on 2026-10-07: host / invite / join, voice, no physics stutter). Everything since is verified only by Python in PIE.
- The latest packaged build is the Development build of 2026-10-17 (`D:\Games-Unreal\ProjectI_Builds\2026-10-17\`, commit `dc7593b`, size phases A + B): a 258 MB zip that installs to 498 MB. It is the first build with the Tier 1 work, sounds, VFX, art and input prompts. It launches offline with no asset-load errors; nobody has played it yet, and the no-Lumen look still needs your check. New builds go to `D:\Games-Unreal\ProjectI_Builds\<date>\Windows\`.
- Art (2026-10-14): a low-poly squirrel and acorn (CC-BY, Poly Pizza) and Kenney nature/road props along the route.
- Every sound slot is filled (2026-10-14) with CC0 clips from Kenney and OpenGameArt (list in `CREDITS.md`), plus menu/run music and run-event sounds on the HUD. VFX (14 Oct) are code-built toon puffs (`FNBPuff`) on every slot that needs one. The font is Lilita One (OFL). None of it has been heard in a build yet.
- **Next** (details in `docs/Roadmap.md`):
  1. ~~Slim the package~~ (done: the 254 MB build above).
  2. 2-PC playtest with the slim build (Tue 13). That build predates item 3's code, so package a fresh one if the test should include it.
  3. Blockers from the playtest. All Tier 1 code was pulled forward and is done (2026-10-13): horn #3, squirrel sound slots #4, acorn pile #5, readable failures #6, charm pass #7, onboarding #8. The pile, callouts and charm parts are checked in 3-client PIE; the horn and the seat labels can't be (Python can't press keys or read the canvas). The #6 font (Lilita One) went in later the same day, compiled but not yet run. The 2026-10-17 build has all of it; none of it has been played by hand.
  4. ~~Sounds in the slots~~ (done 14 Oct, unheard). ~~VFX~~ (done 14 Oct as code-built puffs). ~~Input prompts~~ (done 14 Oct). Still to do: the private itch page (Sat 17 – Mon 19).
  5. **Friend test Tue 20 Oct**, then the top-3 fixes and Tier 2 picks.
  6. **Public itch demo Sat 24 Oct**, as a Shipping build ≤ 200 MB zipped.
- **Budget:** about 65 h total; weekdays 2–3 h, weekends 5–6 h. About 41 h remain from Mon 12 by the plan's calendar.

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

## Assets and licences (the repo is public)
- Commit only CC0, CC-BY and OFL / Apache font files, under `Content/ThirdParty/<Source>/`, with the `CREDITS.md` line in the same commit.
- Fab (including Epic's free monthly content), Sonniss and Pixabay files may ship in builds, but they stay out of the repo: they go in `Content/ThirdParty_Private/<Source>/`, which `.gitignore` excludes.
- Non-commercial (NC) licences would block the later Steam release; leave those files out.
- Rules, folders, the slot map and sources: `docs/Assets.md`.

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
  - Engine state: the server samples Chaos RPM and gear every tick into replicated `RepEngineRPM` (rounded to 10) / `RepGear`; clients smooth it (`GetEngineRPM`, `GetEngineRPMAlpha`, `GetEngineGear`).
  - Sound/VFX slots (all empty until assets are assigned in BP_OffroadCar_Pawn, category "Car|Feedback"): `EngineSound` on the `EngineAudio` component (float params RPM, Throttle, EngineFailed; pitch from RPM for a plain loop), `CrashFeedback` (velocity change ≥ 500 cm/s per 0.1 s, intensity up to 1600), `SpillFeedback`, `FlippedFeedback`, `RightedFeedback`, `HornFeedback`. Played through the unreliable `Multicast_Feedback`.
  - Horn: a seated squirrel's `Server_Honk` → `Honk()` (server, `HornCooldown` 0.25 s) → `Multicast_Feedback(Horn)`, which also stamps the local `GetLastHornTime()` for the HUD's "HONK!" bubble.
  - Crash camera punch: the Crash multicast calls `ShakeLocalCameras`, which punches each local squirrel's camera (`ANBSquirrel::AddCameraPunch`): full for squirrels attached to the car, fading to 0 at `CrashShakeRadius` (25 m) on foot. No time dilation: it's global.
- `Car/NBSeatComponent` — replicated seat (Wheel / Pedals / Rider); the squirrel attaches to it, so its location is the capsule centre.
  - The car has 5: WheelSeat (perched on the steering wheel), PedalSeat (footwell), HoodSeat (front hood), PassengerSeat, DeckSeat (engine deck). Positions were measured from `SM_Offroad_Body` vertices; the comment in the `ANBCar` constructor lists the landmarks.
  - Hop order is the car's `Seats` array (filled in BeginPlay).
- `Player/NBSquirrel` — character. The game mode spawns its Blueprint child `/Game/Blueprints/BP_Squirrel` (`FClassFinder`, falling back to the C++ class); assign squirrel assets there. `/Game/Blueprints` is always cooked.
  - Enhanced Input is built in code (no input assets): WASD, mouse look, Space = jump, E = interact, LMB = action, H = horn (seated only), keys 1–8 = pings, Esc = lobby menu (`NBMenu` in PIE, where Esc stops play). Gamepad: A jump, X interact, B action, Y horn, Start menu; pings have no pad binding.
  - Sound/VFX slots ("Squirrel|Feedback"): `ScreamFeedback` (played on the flying body when the ragdoll starts, on every machine; silent for a late joiner's old fall), `LandFeedback` (the ragdoll body's `OnBodyHit`, impact speed = impulse / mass from `LandMinSpeed` 250 to `LandMaxSpeed` 1500, `LandCooldown` 0.25 s), `GrabFeedback` (the unreliable `Multicast_Grabbed` on an action press, at most every 1.5 s so mashing doesn't machine-gun it).
  - Camera punch: `AddCameraPunch(0..1)` jitters `FollowCamera` by up to `CameraPunchDistance` (35 cm) and `CameraPunchRoll` (4°), squared, decaying at `CameraPunchDecay` per second. Local only.
  - On foot, E takes the nearest free seat. Seated, E hops to the next free seat (`HopToNextSeat`) and Space jumps out on the seat's side of the car.
  - Action press and release go to the server (`Server_ActionPressed` carries the client's server-time estimate), which uses `UNBInteractableComponent::FindBestFor`.
  - Seated: movement and collision off, the move axis goes to the server via `Server_SetSeatInput`, and the camera boom has collision off and is pulled back. Above 150 cm/s, after 1 s without look input, the camera swings behind the car (`SeatedFollowDelay`, `SeatedFollowSpeed`, `SeatedFollowMinSpeed`). On the Pedals seat the tail sticks up as a flag (`PedalTailTransform`) so the low footwell squirrel shows over the dash.
  - Each player gets a unique, replicated fur color (`ColorIndex` → `FurColors`, applied via the BasicShapeMaterial `Color` param to the body, tail, head and ears; `SetPartColor` tints the rest in BeginPlay).
  - Body parts are basic shapes built by `MakePart` in the constructor: `BodyVisual`, `BellyVisual`, `TailVisual`, and an unscaled `HeadRoot` holding `HeadVisual`, `NoseVisual`, `EarVisuals[2]`, `EyeVisuals[2]` and `PupilVisuals[2]`. While tumbling, every entry of `RideAlongParts` (tail, belly, head root) attaches to the body ball, then returns to its standing pose.
  - Model (14 Oct): `ModelVisual` shows `/Game/ThirdParty/PolyPizza/.../SM_Squirrel` (CC-BY, 1 m tall, Z-up fixed before import) at `ModelTransform` (scale 0.48), tinted with the fur colour. When it loads, the primitive body, belly, head, nose and ears are hidden (BodyVisual stays as the invisible ragdoll ball), the googly eyes move to `ModelHeadOffset`, and the tail cylinder only shows as the pedal flag. The model is static: a rigged squirrel is a later wish.
  - Charm (`UpdateCharm`, every machine but a dedicated server, cosmetic only): `SampleCharmAcceleration` takes the actor's smoothed acceleration in its own frame (clamped, reset on a > 3 m jump). It drives a tail spring that swings the tail about its root end, on top of `TailBaseTransform` (the curl or the pedal flag), plus an idle sway. It also drives googly pupils that sag under gravity and bounce off the eye's rim. Tuning is under "Squirrel|Charm".
  - Falling: `Eject(velocity)` / `EjectFromCar()` (the car's velocity plus `EjectKick` out of the seat's side and up).
    - The "ragdoll" detaches `BodyVisual` (the tail rides on it) as a physics ball. Each machine simulates it from the replicated `FNBRagdollState` (start + velocity), and the capsule follows it in Tick.
    - After `RagdollSeconds` (2.5) the server gets the squirrel up where its body landed (`SnapTo` + `Client_SnapTo`, since the owning client runs its own movement).
    - If it isn't seated `RespawnDelay` (8 s) after the fall, `RespawnAtCar` puts it in the first free seat (else beside the car). Entering any seat cancels that.
    - Space in a car going ≥ `BailSpeed` (600 cm/s) ejects instead of hopping out. `FellOutOfWorld` respawns instead of destroying.
    - `RefreshAttachedState` is the single place that sets collision, movement mode and the camera for the riding / tumbling / on-foot states.
- `Player/NBPlayerState` — replicated per-player run stats for the end screen. Set as `PlayerStateClass`.
  - `Falls`, `Respawns` (score penalty), `FiresPutOut` (every squirrel who moved a part's repair forward gets one when it completes; the interactable's `GetCompletedBy()`), `AcornsSpilled` (blamed on the Wheel seat, else the Pedals seat), `DriveSeconds` (Wheel or Pedals seat while Driving).
  - `GetHelpfulness()` = 3/fire + 1 per 30 s driving - 1/fall - 2/respawn - 0.5/acorn spilled. At `EndRun` the game mode picks the highest as MVP and the lowest as "Most useless" (replicated on the game state; none with fewer than 2 squirrels).
- `Interaction/NBInteractableComponent` — the shared "work on this" spot (repairs, door, flip-up).
  - Modes: Hold, Mash, TimingRing, Push2. Server-authoritative; progress, users, enabled and the sweet spot replicate.
  - A squirrel uses the nearest enabled one within `Range` of its actor location, so seated squirrels reach parts near their seat.
  - The timing ring's phase is `frac(ServerTime / RingPeriod)`. Presses are judged at the client timestamp, clamped to the last 0.3 s.
  - `OnCompleted` fires on the server. With `bDisableOnComplete`, the owner re-enables it.
  - `NBInteractionSubsystem` (a world subsystem) is the registry.
- `UI/NBLobbyMenu` — the lobby menu, a UMG widget built entirely in C++ (no widget asset). `ANBHUD` owns it, opens it by itself when standalone, and switches input to UI-only while it's open.
  - Offline: Host a game, Join a game, Practice alone (`DevStartRun`), Quit. Online: the crew list, Start the run (host, needs 2), Invite friends (Steam overlay invite dialog), Restart the run (host), Leave, Quit.
- `UI/NBHUD` — greybox canvas HUD: a pulsing failure list at the top, a "!" marker over each broken part, prompt, progress bar, push count, timing ring with NICE/MISS, the run clock, banners and end screen, and a controls hint. Set as `HUDClass` in the game mode.
  - Failure callouts: `UpdateCallouts` watches every part's `IsFailed()` and queues the part's `CalloutText` on each rising edge (a part seen for the first time only records its state). `DrawCallouts` draws them outlined (`DrawShoutText`, in Lilita One: the constructor finds the `UFontFace` and BeginPlay wraps it in a transient runtime `UFont`, because Python can't author a Font asset's typefaces; the engine large font is the fallback) for 2.4 s: newest big, at most 3.
  - `AlarmStackBottom`: the flip banner, the failure list and the wreck warning stack downward from the top, and the callouts start below them, so nothing overlaps.
  - "HONK!" floats over the car for 0.6 s after each honk.
  - Run audio (`UpdateRunAudio`, every frame, before the pawn check): menu music outside a run, silence plus a beep per second in the countdown, a "go" sound and the run music while driving, then the delivered or failed jingle. Sounds come from `/Game/ThirdParty` via `FObjectFinder` in the constructor; `PlayMusic` cross-fades over 1 s at `MusicVolume` (0.35). A late joiner gets no edge sounds for an old phase.
  - Onboarding: `DrawSeatLabels` bobs WHEEL / PEDALS over each free driving seat until it's first seen occupied (`UsedSeats`, per HUD, so per map load); hidden while the local squirrel is seated.
  - Input prompts: `UpdateInputDevice` flips `bUsingGamepad` on any pad button or stick past 0.35, back on any key or mouse move. `DrawPromptRow` draws [icon] label pairs (the bottom hint, the repair panel's title) from `KeyIcons` / `PadIcons` (Kenney CC0 textures, `ENBPrompt` order: Interact E/X, Jump Space/A, Work LMB/B, Horn H/Y, Menu Esc/Menu); text-only lines use `PromptName`. The PARK boards stay keyboard-only.
- `FX/NBFeedback` — `FNBFeedback` (optional sound + Niagara effect, volume/pitch/jitter/scale) and `NBFeedback::PlayAt` / `PlayAttached` / `StartLoop`. Every play sets the float param `Intensity` (0..1) on both. No-op on a dedicated server or with no assets, so events are wired in code and assets get dropped in later.
  - `Effect` is a `UNiagaraSystem`; legacy Cascade particle systems don't fit.
  - `Puffs` (`TArray<FNBPuff>`): toon particles built in code, the VFX in use. Each puff is a burst (or, in a loop, a stream at `Count`/s) of BasicShapes spheres or cubes in two colours that fly, swell, shrink and vanish; `Glow` > 0 uses the unlit `/Game/FX/M_PuffGlow` (always cooked). `FX/NBPuffFX` simulates them per machine as world-space ISM instances (no replication). Intensity scales a burst's count and speed; a loop's float parameter (Severity) scales its rate and size. The presets (crash sparks + dust, spill, flip dust, righted stars, engine fire + smoke, brake sparks, tire smoke, repair stars, landing dust) live in the Blueprints and were set by a Python script; Niagara templates expose no parameters and Python can't edit Niagara modules, hence code.
  - The run events (countdown, delivered, wrecked) and the music aren't `FNBFeedback` slots: they're local 2D sounds on `ANBHUD` (below).
- `Parts/NBCarPartComponent` — derives from the interactable: a part is its own repair spot.
  - Feedback slots on every part ("Part|Feedback"): `BreakFeedback`, `FailedLoopFeedback` (loops while broken, e.g. engine fire; its `Severity` param ramps 0→1 over `SeverityRampSeconds`, = stall/gone time for engine/brakes), `RepairFeedback`. One-shots only play for live changes, not the state a late joiner receives.
  - Replicated `bFailed`. While failed, the interactable is on and the subclass's `ApplyFailedEffect(seconds since failure)` escalates every tick. Completing the interaction repairs it.
  - `Parts/NBCarParts`:
    - Engine (deck, Mash): power fades to 35 % over 10 s, stalls at 16 s.
    - Brakes (hood, TimingRing): 25 %, down to 0 over 12 s.
    - Tire ×4 (outside each wheel, Hold 3 s, `bRequiresOnFoot` + `bAttachUser`, so the squirrel clings and gets dragged): that wheel's grip drops to 30 %.
    - Door (passenger side, Mash 3 = slam): a greybox door panel on `DoorHinge` flaps open on all machines, and every 3 s above 300 cm/s it throws the passenger out (`EjectFromCar`).
  - Part ranges are tuned so the wheel/pedal seats reach nothing; each rider seat reaches exactly one part.
  - Dev console: `NBSoundTour` plays every car, part and squirrel sound in turn, 4 s apart, labelled on screen (pauses the failure director; seats everyone so the flip throws them). `NBEnd Delivered|TimeUp|Wrecked` ends the run now (end jingle and end screen).
  - Dev console (server): `NBFail Engine|Brakes|Door|TireFL|…|Tire|All`, `NBChaos` toggles a random failure every 12 s, `NBFlip` rolls the car over, and `NBWarp` jumps the car to a route section.
- `Dev/NBInteractTestPad` — throwaway labelled block with one interactable; re-arms 1.5 s after completing. Four of them (one per mode) sit in L_TestTrack at y = -550.
- `Game/NBRunGameMode` — global default game mode; spawns the car from `/Game/VehicleTemplate/Blueprints/OffroadCar/BP_OffroadCar_Pawn`. That Blueprint is **reparented to ANBCar** and holds the mesh, tire sockets and curves.
- `Game/NBRunGameMode` also runs the run: Waiting (the host presses Start in the lobby menu once 2+ squirrels are in; after a restart, and always in PIE, it starts by itself at 2; `NBStart` skips) -> Countdown 5 s -> Driving (the route's 480 s deadline; 150 s on the test track) -> Finished (12 s end screen, then `RestartRun`: `ServerTravel("?Restart")` with **seamless travel**, so clients stay connected, in the lobby and in voice; PIE refuses seamless travel, so restart does nothing there). Ends on delivery, time up, or a total wreck (3+ parts failed for 10 s). Score = acorns x 10 + 2 per second left - 25 per respawn; bad endings score 0. The car is brake-locked (`SetRunLocked`) outside Driving.
- `Game/NBRunGameState` — replicated phase, result, clock (`GetSecondsLeft`), wreck timer, score breakdown.
- `Game/NBFailureDirector` — component on the game mode. Fails a random part every 14 s -> 6 s (ramps with run progress), up to 1 -> players+1 failed at once, 8 s no-rebreak cooldown after a repair.
- `Game/NBFinishZone` — greybox drop-off box with two gate posts; the run is delivered when the car's centre is inside. With the route on, the route spawns its own at the end of the road. On the test track (`?Route=0`) the game mode uses a placed one or spawns one 80 m ahead of the car, grounded by a trace. (The gate at x=8600 and the slalom described in `ad7b92f` were never saved into the committed L_TestTrack.umap.)
- `Game/NBRoute` — the run's course, built entirely in code so it needs no level edits. ~3.5 km, 11 named sections (THE PARK, COBBLE LANE, ACORN HILL, THE HAIRPIN, THE BRIDGE, ROCKY ROAD, SQUIRREL SLALOM, THE BIG JUMP, WIGGLY ROAD, HOME STRETCH, THE DEPOT); the segment list is in the constructor and in `docs/GDD.md`.
  - The game mode spawns it at `RouteOrigin` (0, 3 km, 0), away from L_TestTrack's floor, and spawns 4 player starts and the car at its start. `?Route=0` in the URL goes back to the test track.
  - Replicated + always relevant, but every machine builds the same geometry itself in BeginPlay (centreline in PostInitializeComponents): a walled ground slab (top at z=0, the only thing the wheels touch on flat road), a visual road strip with lines, and per-segment features as ISM layers on BasicShapeMaterial (Bumps, Rocks with a fixed seed, Slalom pillars, Hill/Bridge/Jump slabs), corner markers, roadside TextRender signs (`AddSign`, sized to the text; section names on the right, four control boards on the left of THE PARK) and a depot shed. `AddProps` dresses it with Kenney CC0 props (table `PropKinds` in the .cpp, one ISM per mesh, no collision, fixed seed): trees, bushes, rocks, flowers either side (never within reach of any road sample, clear of the boards), a fence and lamps in THE PARK, lamps on the HOME STRETCH, cones on every corner, road-works at the SLALOM and the JUMP. The server spawns the finish gate 30 m before the road's end.
  - `GetDistanceAlong` (windowed nearest-sample search, so the hairpin's legs don't swap) drives the HUD's progress bar, "depot in X km" and the section banner.
  - Dev console: `NBWarp <n|name>` teleports the car to a section start (riders come along, anyone on foot respawns at the car).
- `Car/NBAcorn` + cargo on `NBCar`: 40 acorns, drawn with the CC-BY acorn model (`SM_Acorn`, 1 m, so the 0.14 scale makes 14 cm; a spilled acorn keeps a hidden sphere for physics). `Acorns` is `ReplicatedUsing=OnRep_Acorns`, which (like the server's `SpillAcorns`) calls `RefreshAcornPile`: the `AcornPile` ISM on the engine deck shows exactly `Acorns` spheres, from `AcornPileSlots` (layers of 5 x 4, fixed seed, bottom first, so spills empty it from the top). A velocity change > 350 cm/s per 0.1 s spills acorns as server-spawned, replicated physics actors (20 s lifespan); a flip dumps 25 %.
- **Acorn in Mouth + chat wheel:** `Game/NBRunGameMode::GiveAcornInMouth` mutes one squirrel for 20 s every 75-105 s while driving (weights: Wheel seat +2, next to a failed part +2; skipped in the last 20 s). State lives on `ANBPlayerState` (`MuteEndTime`, last ping). The local `ANBSquirrel` calls `ToggleSpeaking(false/true)` to cut the open mic. Keys 1-8 send a ping, muted or not (1 s cooldown, shown 4 s left of screen); the HUD shows banners and a legend while muted.
- **Proximity voice:** every `ANBSquirrel` has a `UVOIPTalker` registered with its PlayerState (BeginPlay / PossessedBy / OnRep_PlayerState). Spatialised on the squirrel, full volume within 800 cm, fading to silence over 2500 cm more (`VoiceFullRadius`, `VoiceFalloff`).
- `Game/NBSessionSubsystem` — game-instance subsystem for Steam lobbies, driven by the lobby menu.
  - "Host a game" (or `NBHost`) creates a lobby tagged `NBGAME=SquirrelWheels` (so searches on AppID 480 only find ours), then reopens L_TestTrack with `?listen`.
  - "Join a game" (or `NBJoin`) finds a lobby and joins it. "Leave the game" destroys the session and reopens the map offline. A network failure (e.g. the host left) sets the menu's status line. An accepted Steam invite, or "Join Game" from the friends list, joins automatically.
  - Each step destroys a stale session first. The HUD's top-left line shows OFFLINE / HOST / CONNECTED, the online subsystem and the session status.
- `TP_VehicleAdv/` — imported C++ Vehicle template, compiled inside the ProjectI module. Its duplicate `IMPLEMENT_PRIMARY_GAME_MODULE` was removed; `Build.cs` adds its folders to the include path.
- Map: `/Game/Maps/L_TestTrack` (greybox floor, ramps, bumps). It's the default and startup map.

## Config gotchas (already applied; keep them)
- **Physics substepping ON** (≤8.3 ms, 12 max, MaxPhysicsDeltaTime 0.1). Without it, Chaos tires popped up to ~16 cm at uneven frame rates.
- **Physics Prediction OFF.** Turning it on stops the unpossessed Chaos car from moving: it switches vehicles to the network-prediction input path.
- **The net driver is SteamSockets** (plugin enabled, `bUseSteamNetworking`, relays on). Without Steam (e.g. PIE) it falls back to IpNetDriver.
  - UE 5.7 no longer has `OnlineSubsystemSteam.SteamNetDriver`; the old config silently fell back to IP.
  - Voice is open mic (`bRequiresPushToTalk=false`).
- **Packaging:** `/Game/VehicleTemplate/Blueprints/OffroadCar`, `/Game/Blueprints` (BP_Squirrel), `/Game/ThirdParty` (the HUD font) and `/Engine/BasicShapes` are always cooked, because only C++ constructors reference them. Anything new that only C++ references (a HUD font, an imported mesh) needs the same treatment; `docs/Assets.md` has the folder plan.
  - Package with: `RunUAT BuildCookRun -platform=Win64 -clientconfig=Development -build -cook -stage -pak -iostore -nodebuginfo -archive -archivedirectory=...`
  - Linking the monolithic `ProjectI.exe` with full debug info needs ~16 GB of RAM. With Firefox or a game open, UBA kills the link at its memory threshold and retries for ~20 min; close big apps before packaging. (`DebugInfoLineTablesOnly` in the Target.cs does nothing with MSVC; `DebugInfo = DebugInfoMode.ProjectOnly` would fix it at the cost of an engine recompile.)
  - `-nodebuginfo` keeps the 2.3 GB PDB out of the build. Copy the PDB from `Binaries/Win64` to `ProjectI_Builds\<date>\Symbols\` so crash dumps can still be read.
  - The size budget and the steps to meet it are in `docs/Roadmap.md` (Package size). Write each build's zip size in the playtest notes.
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
  - The user: feel/fun calls, 2-PC tests, the itch account, playtesters, downloading the asset shopping list (`docs/Assets.md`), and the decisions listed in `docs/Roadmap.md` (final title, AppID for the public build, and the rest).
  - Claude: code, config, Blueprints/levels via MCP, asset imports with their `CREDITS.md` lines, builds, tests, commits.
