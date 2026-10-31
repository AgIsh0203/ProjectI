# Roadmap to the itch demo

Living doc, last updated 2026-10-31. The plan to friend test 3 (**Sat 28 Nov**), the ranked backlog and the decisions waiting on you. It folds the two research notes from 2026-10-09 (`assets-and-improvements.md`, `free-assets-and-improvements.md`) into the approved plan and replaces that plan's schedule. Design detail lives in [GDD.md](GDD.md), asset sources and licences in [Assets.md](Assets.md), hands-on checks in [Playtest.md](Playtest.md).

Effort: **S** < 2 h, **M** 2–5 h, **L** > 5 h.

## Plan to friend test 3 (decided 18 Oct, replanned 31 Oct)
Friend test 3 on **Sat 28 Nov** checks whether the redesign (mountain trail, earned damage, failures that matter, the new seat) is fun. It is a test, not a launch: the public demo comes after a round of fixes.

- **The test:** 4 players if possible, at least 2 who have never played (the only real check of onboarding and the who-is-who HUD), 3 runs each (is it still fun once the route is known?). Time every delivered run: those times set par for good (J5). It is also the muffle's first test (Acorn in Mouth).
- **Done by 31 Oct, ahead of the 18 Oct plan:** its whole first block (G5 + G7, A1, #10, #12, #13, #14, G6), the stretch's new car (A2, 26 Oct), then R1, R2, H4 and the muffle. The **27 Oct build** was played online on 30 Oct (the user and friends, 3 runs), and reviews came out of it ([Playtest.md](Playtest.md#30-oct-online-session-the-27-oct-build)).
- **Replanned 31 Oct**, in a question-by-question review of the plan with the user: the [friend test 3 work](#friend-test-3-work-decided-31-oct-j1j15) below replaces the 18 Oct plan's second half. The 7 Nov mid-point check is dropped (the 30 Oct session did its job, on the 27 Oct build), and so are side tasks (the story's helper vehicles take that idea, after the test).
- **No test before friend test 3** (the user: the next one is 2 weeks to a month away), unless the user calls one earlier.
- **Budget:** ~65 h of work by estimate (route ~25 h, fixes ~20 h, HUD ~11 h, checks and packaging ~8 h) against ~45 h to Sun 22 Nov at ~15 h a week. Recent items landed faster than their labels, so it may fit; if not, the cut order below, then the date.

| When | Work | Done when |
|---|---|---|
| 1 – ~11 Nov | **The route first:** it's the biggest piece, par, the dog's new stretch, the hamper and the obstacles all sit on it, and it's the likeliest to run late. J1 the new turns and J2 BUCK'S CUT in one re-bake, the roadblocks and pinch points re-placed, J3 the obstacles, J4 the autopilot. The small fixes (J6–J11, J14, H1, H2) fill the gaps while the user drives the new route. The J15 HUD mock-up goes up early, for the user to look at. | The autopilot drives both roads end to end; the user has driven both solo and the turns feel right (if not, the climbs become a switchbacked hill road: J1). |
| **Sun 15 Nov** | **Check-in:** is the route work done? | If not, friend test 3 moves to **Sat 5 Dec** rather than going out half-done. |
| ~12 – 22 Nov | The rest on the new route: J5 par (the user's solo time + 4 min), J12 the dog's new stretch, J13 the hamper; then the J15 HUD build. H3 only if time is left. | Everything works in 4-client PIE. |
| 23 – 27 Nov | Package, a smoke test, update the questionnaire (gears, roadblocks, the fork, the title shortlist), send the build out. | Testers have the build. |
| **Sat 28 Nov** | **Friend test 3.** Record it; the form; the run times. | Recordings, answers and times collected. |
| 29 Nov – ~12 Dec | Fixes from the test and the [talking design session](Review_2026-10-31_VoiceAndAcornInMouth.md), then the release work: Shipping build (size phases C and D), itch page, AppID 480, final title. | **Public demo ~Sat 12 Dec** (tentative, the user's call after the test). |

**Cut order if behind** (first to go): H3 → J3 (the obstacles on the straights) → J13 (the picnic hamper) → J15 shrinks to the crew list and the repair tags (the rest after the test). After that, the date moves.

**Out of scope until friend test 3:** #9 (pedal-seat blind camera), #11 (pick acorns back up), R3 (with the golden acorns, S6), side tasks, Tier 3, and the story beyond J2, J12 and J13 (no excavator, ranger station, renames or junkyard yet).

## Story demo (after friend test 3)
Decided and expanded 29 Oct ([Story.md](Story.md): the bible, with the Chapter 1 beat sheet and a checklist of what it needs built). **Friend test 3 takes three pieces of it** (31 Oct): BUCK'S CUT's road (J2; S5's excavator, ranger station and parking bay stay here), the picnic hamper (J13) and TINY's ambush stretch (J12; the chained start stays in S9). The rest starts after 28 Nov. If friend test 3 passes, the public demo becomes **Chapter 1, The Crash Site**, and the ~12 Dec date may slip for it (the user's call). Build order:

| # | Item | Effort | Note |
|---|---|---|---|
| S1 | Stopwatch and par medals instead of the deadline (stopwatch + par bonus done 31 Oct, R1; medals left); new score (acorns × 10 + par bonus − respawns) | M | `ANBRunGameMode`, `ANBRunGameState`, the HUD clock and end screen. Changes a locked decision (done in CLAUDE.md). |
| S2 | Checkpoints (section starts + camp) and the wreck rewind (+30 s); host Continue from a save | L | Snapshot the car, acorns, parts and stopwatch; reuse `ANBCar::TeleportCar`. The camp waits for the jeep and every squirrel. |
| S3 | CB radio line system: triggers, priorities, variants, subtitles; every Chapter 1 line shows as a subtitle until the recordings arrive | M | The lines and the rules: [story/RadioScript.md](story/RadioScript.md) (114 for Chapter 1). The voice actor reads the script now and records once this works. |
| S4 | THE JUNKYARD: re-theme THE PARK (the crashed Acornucopia truck, spilled crates, TINY's kennel, the deer); the jeep starts in pieces and building it is the tutorial; the intro card; the three section renames; Buck & Sons dressing on THE BRIDGE | L | Keeps the control boards as the tutorial. The renames (THE PARK → THE JUNKYARD, HOME STRETCH → THE DESCENT, THE DEPOT → THE CAMPGROUND) go in `ANBRoute`'s segment list and, by hand, in L_Route's baked signs: a re-bake would lose hand edits. |
| S5 | The fork at the top of WIGGLY ROAD: BUCK'S CUT (a new road; check the 18 % grade limit) with the excavator (Tracks / Boom / Bucket seats, a hydraulic-hose part), a jeep parking bay and a destructible ranger station; an Acornucopia picnic hamper (the acorn cache) on WIGGLY ROAD | L | The biggest item. Needs a CC0/CC-BY excavator model ([Assets.md](Assets.md)). |
| S6 | Golden acorns (tint or a new model; maybe more than 40) | S | |
| S7 | Snow creep when behind par | M | Cosmetic. |
| S8 | Demo ending: the honk test (count the horn after Dispatch's line; five outcomes), a teaser card of the 4 coming machines, "Follow on itch.io" | S | The counting rules: [RadioScript.md](story/RadioScript.md#the-honk-test-end-of-chapter-1). |
| S9 | TINY re-staged: chained in THE JUNKYARD; at the start, a chase out of it that can't bite; then the ambush from COBBLE CURVE down THE DESCENT, stopping at THE WASH | S | `ANBDog`, `ANBRoute::SpawnDog` (`DogSection`). |

## History: where we were (2026-10-12)
- M0, M1 and M2 are code-complete. M2 finished on 9 Oct, three days early. The end-screen stats (planned for M3a) and the sound/VFX slots are done too.
- All of it is verified only by Python scripts in PIE. Nobody has played a run by hand, and the only packaged build (6 Oct) predates M1 and M2.
- That build is a **1.04 GB download that installs to 3 GB**, though players need only about 0.7 GB of it ([Package size](#package-size-slim-it-before-the-playtest)).
- Every sound slot is filled with CC0 clips (14 Oct); VFX are code-built toon puffs (14 Oct). The squirrel is built from primitives: since 13 Oct it has a head, googly eyes, ears and a wobbly tail (#7).

## Rules
1. **Playtest before adding anything.** Every number (spill threshold, failure rate, deadline, tire cling) is a guess until two people have played a run.
2. **Sound first.** Silent chaos reads as a bug on stream. Filling the slots is asset work, not code.
3. **Show the game state.** Viewers can't hear it. Acorns, failures and falls must read on screen in under 2 s with the sound off (the plan's viewer-fun check).
4. **Tier 2 waits for the playtest.** Nothing in it starts until the 2-PC test says the core loop is fun.
5. **Small download.** Nobody downloads a multi-GB jam game. The demo ships at ≤ 200 MB zipped, and every build's size gets written down.

## History: the old schedule to 24 Oct (dropped 18 Oct)
Hours follow the plan's calendar (weekdays ~2.5 h, weekends ~5.5 h): about 41 h from Mon 12 to Sat 24. The 9 Oct notes guessed ~25 h. If that's nearer the truth, drop Tier 2 and the route dressing first.

| Dates | ~h | Block | Who | Done when |
|---|---|---|---|---|
| **Mon 12** | 2.5 | **Slim the package:** phases A and B of [Package size](#package-size-slim-it-before-the-playtest), then package the playtest build. | Claude; you check the look | A Development build ≤ 300 MB zipped that looks acceptable. |
| Tue 13 | 2.5 | **M2 gate.** A full 2-PC Steam run with the slim build and [Playtest.md](Playtest.md); start on blockers; tune numbers live. | You play; Claude fixes | A run goes start to finish on 2 PCs, and you've listed the five things that felt worst. |
| Wed 14 – Fri 16 | 7.5 | **Blockers first**, then the friend-test code: horn (#3), squirrel sound slots (#4), acorn pile (#5), readable failures (#6). Batch the header changes into one editor-closed build. | Claude | All four work in 2-client PIE. |
| by Fri 16 | — | **Downloads:** the [shopping list](Assets.md#shopping-list). | You | The files are on disk, ready to import. |
| Sat 17 – Sun 18 | 11 | **M3a art and juice:** sounds into every slot, VFX (engine fire, crash sparks, spill puff), private itch page + butler upload. Route dressing only if time is left. | Claude; you for itch | A packaged build with sound. |
| Mon 19 | 2.5 | Fresh slim build, a short 2-PC smoke test, upload to the private itch page, send friends the link. | Both | Friends can download it, and it's still ≤ 300 MB zipped. |
| **Tue 20** | — | **Friend test.** Record (OBS/Discord), then the 3-question form ([Playtest.md](Playtest.md#friend-test-tue-20-oct)). | You | Recordings and answers collected. |
| Wed 21 – Fri 23 | 7.5 | **M3b:** the top-3 friend-test issues first, then the Tier 2 picks the test supports. | Claude | Top-3 issues fixed. |
| **Sat 24** | 5.5 | **M4 ship:** the Shipping build (size phase C), itch page, final title, public release ([release checklist](#release-checklist-sat-24)). | Both | The demo is public on itch. |

The size pass takes Mon 12 from the old M2-gate block, so blocker fixes now spill into Wed – Fri. If that squeezes the friend-test code, the cut order applies.

**Cut order if behind** (first item goes first): route dressing → VFX beyond engine fire and crash sparks → the charm pass shrinks to eyes only → seat labels (keep the PARK signs) → camera shake. The plan's original cut order (door, then a 1-player flip-up, then 3 players, then the ring becomes a hold) now applies only if a built system fails the playtest.

**Never cut:** the plan's list (2 seats, 3 failures with distinct repairs, acorn cargo, proximity voice + Acorn in Mouth, end-screen stats), plus a 2-PC test before the friend test, sound in the slots, and the size pass.

## Package size: slim it before the playtest
Nobody downloads a multi-GB jam game. The 6 Oct Development build is a **1.04 GB download that installs to 3 GB**, and players need only about 0.7 GB of it. Measured from that build and its cook output (`Saved/Cooked/Windows`):

| What | Size | Phase |
|---|---|---|
| `ProjectI.pdb` debug symbols | 2,321 MB (575 MB zipped) | A1 |
| `ProjectI.exe`, Development | 303 MB (111 MB zipped) | C |
| Cooked content (`.ucas`), already compressed | 278 MB (272 MB zipped). Before compression it holds: | |
| · DX12 (SM6) engine shaders, swollen by ray tracing, Lumen, virtual shadow maps and Substrate | 202 MB | B, D |
| · NNE denoiser models (an AI denoiser for the path tracer, from an engine plugin that's on by default) | 107 MB | A2 |
| · unused template content (the sports car, the VehicleBasic map) and editor-only engine content | ~20 MB | A3, A4 |
| DLLs: NNE runtimes (32 MB), Vulkan debug layers (37 MB), the D3D12 debug layer, Aftermath | 91 MB (~40 MB zipped) | A2, C |
| VC++ installers, x64 and ARM64 | 36 MB (35 MB zipped) | A5 |

**Targets:** playtest and friend-test builds (Development) **≤ 300 MB zipped**. The demo build (Shipping) **≤ 200 MB zipped and ≤ 500 MB installed**; 150 MB is the stretch. The estimates below are rough, so measure after each phase.

### A. Free cuts, no visual change (Mon 12)
1. **No debug symbols in the build:** add `-nodebuginfo` to BuildCookRun. The PDB stays in the project's `Binaries/Win64`; copy it to `ProjectI_Builds\<date>\Symbols\`, outside the zip, so crash dumps from friends can still be read.
2. **Disable engine plugins the game doesn't use** in `ProjectI.uproject`. Start with the NNE family (NNEDenoiser, NNERuntimeORT, and any other NNE runtime that gets staged). Then do the default plugins whose content got cooked for nothing: Paper2D, HairStrands, DeformerGraph, and ControlRig if nothing needs it. One batch at a time: the editor must still start and the cook must pass. Fewer plugins also means a smaller exe.
3. **Cook only what the game uses:** narrow `DirectoriesToAlwaysCook` from `/Game/VehicleTemplate` to `/Game/VehicleTemplate/Blueprints/OffroadCar`. BP_OffroadCar_Pawn is the only `/Game` asset C++ loads, and its meshes and materials follow by reference. `MapsToCook` stays L_TestTrack only. This drops the sports car and the template's VehicleBasic map.
4. **No editor content:** `bSkipEditorContent=True`.
5. **App-local VC++ runtime instead of the installers:** `IncludePrerequisites=False`, `IncludeAppLocalPrerequisites=True`. The engine has the DLLs in `Engine/Binaries/ThirdParty/AppLocalDependencies`.
6. **English only:** `InternationalizationPreset=English`, and stage only `en`.

Expected: about 1 GB → about 320 MB zipped.

### B. Simpler lighting (Mon 12; it changes the look, so you check it)
- Ray tracing off: `r.RayTracing=False`, plus its proxies setting. Nothing needs hardware ray tracing.
- Lumen off: `r.DynamicGlobalIlluminationMethod=0`, `r.ReflectionMethod=2` (screen-space reflections), then `r.GenerateMeshDistanceFields=False`.
- Virtual shadow maps off: `r.Shadow.Virtual.Enable=0`, so regular cascaded shadow maps.
- `r.Substrate=False` and `r.SkinCache.CompileShaders=False`.
- Retune the sky light and sun in L_TestTrack so the greybox doesn't go flat or dark.

It pays off beyond size: Lumen and virtual shadow maps are the most expensive things in the frame. Without them the game runs on the weak laptops friend groups actually own, and flat-colour low-poly art doesn't need bounce light. If you reject the look, the playtest uses the phase A build.

Expected: about 250 MB zipped. This is the playtest build.

### C. Shipping build for players (the demo, Sat 24)
The friend-test build stays Development, so its logs and console still help with bugs.
- `-clientconfig=Shipping`: a smaller exe, and no Vulkan validation layers, D3D12 debug layer or other debug DLLs.
- Steam on AppID 480: UE only writes `steam_appid.txt` in non-Shipping builds, so ship one (containing `480`) next to the exe. If Steam still won't start, ship the slim Development build instead.
- Optionally, a higher Oodle compression level for the release paks.

Expected: about 200 MB zipped.

### D. Drop Shader Model 6 (done 23 Oct)
With Lumen, Nanite and virtual shadow maps off, nothing needs SM6. Targeting only SM5 (DX12 and DX11) leaves one small shader library: today's SM5 engine shaders are 16 MB against 202 MB for SM6. Check that the game launches on both DX12 and DX11.

**23 Oct:** `DefaultEngine.ini` now targets PCD3D_SM5 only (DX12 and DX11) and SF_VULKAN_SM5 on Linux. Measured from the 20 Oct cook: SM6 was 143 MB of shaders (124 MB global, mostly MegaLights ~37 MB, Nanite ~16 MB, Lumen screen probes ~12 MB, virtual shadow maps 4 MB: the engine compiles them whenever the platform could run them, whatever the project settings), SM5 16 MB. Still to check in the next package: the cook passes, the game starts on DX12 (log: `RHI D3D12 ... SM5`) and with `-dx11`, the look is unchanged, and the zip size. If B is reverted, Lumen still works at SM5 (software); Nanite would not.

Expected: about 160 MB zipped.

### E. Textures and audio (23 Oct, measured from the 20 Oct cook)
Non-shader cooked content is ~99 MB uncompressed. Our textures are 18.5 MB, all ≤ 1024²; 15 MB of that is the template cars (offroad body, wheels, seats, suspension; the sports car the roadblocks use), and the normal maps are 11 MB of it. Audio is 3 MB (two thirds music, quality 80); nothing to gain. Done 23 Oct (in the slim build): the calibration view-mode materials no longer cook (`DefaultEngine.ini`, ~9 MB of textures), and neither do Interchange's `New_LUT` (8.7 MB) and SpeedTree's content (1.4 MB) (`DirectoriesToNeverCook`; nothing cooked references them). Waiting on you: capping the car textures at 512 (`MaxTextureSize`) would save ~11 MB uncompressed and makes the car blurrier up close.

**Result, 23 Oct (D–F plus the editor-only plugins):** the slim Development build is **164 MB zipped, 376 MB installed** (20 Oct: 270 / 511). The exe went 317 → 277 MB, the content `.ucas` 141 → 41 MB. The exe is now most of the download; Shipping (phase C) is the next big cut.

### F. Meshes and the map (23 Oct, measured from the 20 Oct cook with the `PkgInfo` commandlet)
- **Meshes, ~10 MB**, nearly all template car. 4 MB of it is cooked triangle-mesh collision nothing uses (body 2.5 MB, tire 0.97, sports car 0.58): `Tools/Car/StripMeshCollision.py` removes it (run 23 Oct). The sports car's Nanite data already goes with SM5 (the cook strips Nanite for platforms without it); the roadblock then draws its fallback mesh, so look at it in the build.
- **L_Route, 13.3 MB:** landscape collision 6.15 MB (126 components x 49 KB), landscape height/weight textures 5.6 MB (already stored compressed), everything else (1,601 props, 476 mesh actors, signs) under 1 MB. Halving the collision resolution (`CollisionMipLevel` 1) would save ~4.5 MB but gives the wheels 4 m bumps under a 2 m trail, so no. Possible later: no collision on components nobody can reach (beyond the invisible boundary walls), or a smaller `LANDSCAPE_MARGIN` in `BakeRoute.py` (a re-bake loses hand edits).

### Keep it small
- Write the zip and installed size into the playtest notes for every package. Anything that adds more than 20 MB needs a reason.
- Size rules for imported assets: [Assets.md](Assets.md#where-files-go).

## Backlog
Ranked within each tier. "Touches" says where the work lands, so a session can start without re-deriving it.

### Friend test 1 fixes (17 Oct): before the next test
The test is written up in [Playtest.md](Playtest.md#friend-test-1-results-sat-17-oct-build-dc7593b). Fun scored 4 / 4 / 3. The core is there, but nobody could read their role, voice chat failed, and the failures felt like noise.

Done 17 Oct, in the `2026-10-17-fix` build: **the restart crash** (every client crashed when the next run loaded); **the director bug** (a repair let the next failure in within 1.5 s, and leaving a cheap failure unfixed held all the others off); **part wear** (how you drive tilts which part breaks: [GDD](GDD.md#core-loop-610-min-run-800-deadline--built)).

| # | Item | Why | Effort | Touches |
|---|---|---|---|---|
| F1 | (code done 17 Oct) **Voice you can hear:** louder voice by default, then a settings panel with mic and speaker device, voice volume and game volume | Everyone fell back to Discord, which kills proximity voice and Acorn in Mouth, two of the never-cut features | S (gain), M (panel) | `ANBSquirrel` VOIPTalker settings; a settings page in `UNBLobbyMenu`; the device lists come from the online voice interface / audio mixer |
| F2 | (code done 17 Oct) **Who am I, who's driving:** a YOU arrow and name tags in each fur colour; "You: WHEEL" on the HUD; every seat's occupant shown; fix the WHEEL / PEDALS labels drawing on top of each other | "Am I even controlling it?", "I thought I was blue". The top confusion in every recording | S–M | `ANBHUD` (world-projected tags, seat panel); `ANBSquirrel::ColorIndex` |
| F3 | (code done 17 Oct) **Seated camera closer:** the arm drops from 11 m to ~6 m, lower and centred on the car | The car is a speck at the bottom of the screen; "the camera angle was not very good" | S + your feel check | `ANBSquirrel` seated boom length / offset |
| F4 | (code done 17 Oct) **Failures that matter:** a flat tire pulls the car to its side and caps speed; an open door throws people more often; failed brakes do something even when nobody brakes (they grab, so the car lurches); the engine mash shows "MASH [LMB] 9/14" by the part | "What difference does a flat tyre make", "the door doesn't matter", "we didn't even press the brake"; engine mash was the worst repair ×3 | M | `UNBTirePart` / `UNBDoorPart` / `UNBBrakePart` effects; the HUD repair panel |
| F5 | (code done 17 Oct) **Damage meter:** a wear bar per part (green → red) so players can see what's about to go and why | Turns wear into a readable cause and effect; asked for in the chat | S–M | `ANBHUD`; `UNBCarPartComponent::GetWear()` already replicates |
| F6 | (code done 17 Oct) **THE BIG JUMP pays:** a clean wheels-down landing spills nothing, and airtime gives bonus points | "What's the use of the jump if we lose the nuts anyway?" | S | `ANBCar` spill check (ignore the vertical jolt when all wheels touch down), score in `ANBRunGameMode` |
| F7 | (done 17 Oct) **Prop collision** on trees, rocks, lamps and fences (not bushes or flowers) | The car drives through trees. One player thought colliders could be annoying, so it's your call | S | `ANBRoute::PropLayer`: a per-kind collision flag in `PropKinds`; check the Kenney meshes have simple collision |
| F8 | (done 17 Oct: "NAME IS ON IT" when someone else is on the part) **"Widgets are shared":** check whether the repair prompt and progress show on squirrels who aren't repairing | Reported by two players; not confirmed from the video | S | `ANBHUD` prompt / progress drawing |
| F9 | (done 17 Oct) **Pacing:** a calm cruise, then a storm (e.g. 40 s quiet, 25 s pressure), instead of a steady drip | "I'd prefer a cruising phase … calm before the storm" | S | `UNBFailureDirector` |

### Friend test 2 answers (18 Oct): what they add
Two more answers on the 17 Oct package ([Playtest.md](Playtest.md#friend-test-2-answers-18-oct-the-package-sent-on-17-oct)); fun 1 and 3. Voice, roles and collision repeat F1, F2 and F7, which that package didn't have yet. New:

| # | Item | Why | Effort | Touches |
|---|---|---|---|---|
| G1 | (done 18 Oct, PIE) **Earned damage:** a crash wears (a hard one breaks) the part nearest the impact; rough trail, rocks and fords wear the tyres; timed failures only while the car moves, leaning harder on worn parts | Both: "random damage … should be a punishment for hitting stuff"; "tires popping for no reason … while stationary". Makes driving skill matter and gives viewers a cause they can see | M | `ANBCar::CheckSpill` (impact direction from the velocity change), `ANBCar::UpdateWear` (route roughness under each wheel), `UNBFailureDirector` (speed gate, `WearBias`) |
| G2 | (done 18 Oct) **Say why you fell out:** a big line for the ejected squirrel (DOOR FLUNG YOU OUT! / CRASH! / BAILED! / SWEPT AWAY! / CAR FLIPPED!), and the countdown explains itself ("Get back in! 8 s, or respawn −25") | "No idea why I get thrown out … don't see the point in needing to return the car quickly" | S | `ANBSquirrel::Eject` callers pass a reason (replicated with the ragdoll state); `ANBHUD` respawn line |
| G3 | (done 18 Oct: hint + game volume 0.7, voices no longer scaled by the game slider; no ducking) **Find the sound settings:** "Esc: sound and voice" on the HUD while waiting and in the countdown; lower default game volume (or duck it while someone talks) | Both drowned in game sound and couldn't find any setting | S (hint, default), M (ducking) | `ANBHUD` waiting/countdown text; `UNBAudioSettings` defaults |
| G4 | (done 18 Oct) **Wrong seat for that key:** A/D on the Pedals seat flashes "the WHEEL steers: you're gas and brake", W/S on the Wheel the reverse | Luna tried to steer from the pedals; steering was found by chance | S | `ANBSquirrel` seated input → a local HUD hint |

**Run length went the other way:** all three said "too short", and run 1 was delivered with half the deadline left. That argues against #15 and for more route, harder sections, or side tasks; it needs your decision below.

**Terrain** (done 18 Oct, PIE only): the user redirected it mid-way from rolling hills to **"RV There Yet?"-style mountains**:
- cliffs and climbs up to 18 %;
- a rough dirt track;
- two fords with hidden pebbles, where squirrels on foot are swept away;
- a river in a gorge under THE BRIDGE.

Design and tools: [Terrain.md](Terrain.md). Hands-on checks: [Playtest.md](Playtest.md#mountain-trail-18-oct-check-by-hand).

Ideas from the test, after the demo unless you pull one forward (the Gears seat was pulled forward as G5): **branching sections**, **side tasks / vehicle swaps** (R.V. There Yet?), **roadside distractions** (billboards, ads), a **Linux build**.

### Friend test 3 scope (locked 18 Oct)
Built in this order (see [the plan](#plan-to-friend-test-3-decided-18-oct-replanned-31-oct)). The Tier 2 picks (#10, #12, #13, #14) are in their table below.

| # | Item | Why | Effort | Touches |
|---|---|---|---|---|
| G5 | **Gears seat** on the PassengerSeat (beside the wheel): W/S shift up / down. **Host option** "Manual gears: OFF / ON" on the host's lobby page next to Start the run (and on Practice alone), seen by every client, locked once the countdown starts. Off = automatic, as today. On = manual, and with the Gears seat empty the car **stays in its gear** (bogs on climbs, tops out on the flat) until someone hops over. Too high a gear bogs, over-revving wears the engine and can break it. The HUD shows the gear and an RPM needle with a red zone to everyone; "nobody!" flashes for an empty Gears seat as for the Wheel. Manual gears score **×1.25** ("Manual gears ×1.25" on the end screen; bad endings still score 0). **Handbrake** (the user, 18 Oct, after Luigi's review): the Gears seat also holds the handbrake, with manual gears on or off, so the seat always has a job. Hold a new rebindable `ENBBinding::Handbrake` (default Shift; gamepad RB) to lock the rear wheels: the Wheel squirrel calls it for THE HAIRPIN and the cliff bends, and pulling it at the wrong moment spins the car. The HUD shows "HANDBRAKE!" over the car while it's on. The pedals stay one seat (gas + brake): no split. | Gives a 3rd and 4th squirrel a real job; the 18 % climbs make the gear matter. Asked for in friend test 1 | M | **Done 18 Oct** (scripted 2-client PIE: gears hold and shift, bog in 4th, back to the chosen gear after reversing, over-rev wear, handbrake spin, ×1.25 on the end screen). Still to see: a hand-driven run in 4-client PIE, the lobby toggle clicked, gamepad RB. Found: automatic never leaves 1st on the flat, so shifting up makes the car faster (GDD) |
| A1 | **Rigged squirrel** (the first of the [art upgrades](#art-upgrades-the-user-asked-for-14-oct)): rig the current Poly Pizza model with Blender's Python (spine, tail, head, 4 limbs, automatic weights), export FBX. **Cosmetic only**: the replicated ball stays the physics; bones are driven in code on each machine, no replication. Poses: flail while flying, grip while seated, a simple run cycle on foot, mash / tug while repairing. No physics-asset ragdoll. Fallback if the weights look bad: a cheap paid rigged model in `ThirdParty_Private`. | A squirrel flailing as it flies out is the clip | M–L | **Done 18 Oct** (1-player PIE only): `ModelMesh` + `UNBSquirrelAnimInstance`, 8 animations, the seat hop arc; see `Tools/Blender/README.md`. Still to see: a client's view (2-client PIE), and the look in a build |
| G6 | **Roadblocks on narrow roads:** a **broken-down car** on a cliff ledge at PINE PASS (Kenney Car Kit, CC0), a **fallen tree** across the inside of THE HAIRPIN, a **small rockslide** on ROCKY ROAD. Each leaves a gap of about a car width + 1 m beside a drop: slow and careful gets through; rushing it clips the block (crash damage) or goes over the edge (Hold R, −3 acorns). Placed as level actors in L_Route with `Ground=` tags (SnapToGround), **no re-bake**, so the hand edits survive. **Plus (Luigi, 18 Oct): one moving hazard,** a swinging wrecking ball (or log) over a narrow stretch, timed so a crew that waits gets through and one that rushes gets hit (crash damage, a squirrel knocked out). And **pinch the trail** at 2–3 cliff edges with rocks and fallen logs placed the same way, so falling off is a real risk without narrowing the baked trail (a narrower trail everywhere needs a re-bake, which loses the hand edits). | Makes Wheel and Pedals talk ("slow… slow… STOP"). "Incredibly easy" (Luigi) | S–M (+ S for the swinging hazard: a server-moved kinematic actor, replicated) | **Done 20 Oct** (`Tools/Route/PlaceRoadblocks.py`, `Game/NBWreckingBall`; see the GDD's Events): the car is the template's `SM_SportsCar` (no Kenney Car Kit imported). Checked by script in PIE: props block, the ball knocks squirrels out and the car off the bridge. Still to see: driving through by hand, whether 3.5 m is too tight or too easy, the ball's timing. **The autopilot drives the centreline, so it now rams the roadblocks** (it already stuck on the 18 % climbs). Fix: J4 (31 Oct) |
| G7 | **A closer camera** (Luigi, 18 Oct: "unnecessarily distant"): try `SeatedArmLength` ~420 (650 today) and a lower pivot in 4-client PIE; keep the chase camera (locked). Add a "Camera distance" slider on Options > Controls if one value doesn't suit everyone. | The car and the squirrels read bigger on stream | S | **Done 18 Oct**: 420 cm, pivot 110 cm. No slider yet: add one if testers disagree |
| A2 | Stretch, after 7 Nov: **the new car** (steps under [art upgrades](#art-upgrades-the-user-asked-for-14-oct)). **Idea (Luigi, 18 Oct):** make it a makeshift squirrel car (acorn-shell or crate body, bottle-cap wheels, twig frame, Flintstones-style) instead of another normal car, so the setting says "squirrels". **Layout (the user, 18 Oct):** put the engine and its fire beside the rider seat that repairs it, not under it: on this car the fire puffs rise through the deck seat and hide the TailFan animation (damped while worked, as a stopgap). | Fixes the bolted-on door and the acorns on the engine deck | L | **Done 26 Oct (early, the user asked): the junk jeep.** A toy jeep (Poly Pizza, CC-BY) patched with trash, the breakable parts being the trash: battery engine on the hood, sponge brake, cardboard door, acorn crate (GDD). A garden wagon was tried first and dropped (the user: it "doesn't look like something with an engine / brakes / accelerator or doors"). Built as a `UNBCarBody` data asset on the buggy's skeleton, so the driving is unchanged and `?Car=Buggy` brings the buggy back. Checked in 1-player PIE and by autopilot; nobody has played it (Playtest.md) |
| — | Stretch, after the car: **side tasks / vehicle swaps** (R.V. There Yet?). A design session comes first; take Luigi's review into it: **hop out to find spare parts** for the makeshift car, and the depot becomes the crew's **home tree** (the acorns are the winter stash). | "Too short" in 4 of 5 answers | L | **Dropped from friend test 3 (31 Oct):** the story's helper vehicles and forks take the idea after the test |

### Luigi's review (18 Oct): where it went
The review is in [Playtest.md](Playtest.md#luigis-extra-review-discord-18-oct-same-17-oct-package). It was played on the 17 Oct package: flat route, failures that didn't matter yet, no F1–F9. The mountain trail's cliffs, fords and gorge were already the answer to part of "incredibly easy"; nobody outside has played them.
- **One job each (gas / brake / steer / repair):** the pedals stay one seat (the user). The Gears seat gets the handbrake (G5).
- **Narrow track, falls, moving obstacles:** G6 (the pinch points and a wrecking ball). Judge the difficulty at the 7 Nov run before going further.
- **Closer camera:** G7. First person: no (the chase camera is locked).
- **Every repair from inside the car:** no. The tyre is on foot on purpose (clinging to the moving wheel), and getting out is what Luigi's setting idea wants too. Every other part can already be reached from a seat.
- **"RV There Yet?":** the 18 Oct terrain was built in that style; keep it in mind for the side-tasks design session.
- **The squirrel setting (tree branches, home, makeshift car, spare parts):** A2 and the side-tasks session for now. A route over giant tree branches is a bigger idea for after the public demo ([Tier 3](#tier-3-after-the-demo)).

### Friend review (30 Oct): where it went
The review is in [Playtest.md](Playtest.md#friend-review-30-oct). Decided 31 Oct: H1 and H2 go into friend test 3 (with J8 and J15), H3 only if time is left (first in the cut order); H4 is done.

| # | Item | Why | Effort | Touches |
|---|---|---|---|---|
| H1 | **A sturdier door:** less wear per seat hop (0.15 → ~0.06) or only hops that pass the door's seat, and faster healing for the door | "Door is too fragile"; seat hops are the cheapest action in the game and each one loosens it | S + feel check | `ANBCar::DoorWearPerHop`, `NoteSeatHop`; per-part recovery in `UpdateWear` |
| H2 | **What breaks what:** a line under each part on the damage meter naming its live cause while wear rises (DOOR: "seat hops", TYRES: "rough ground", ENGINE: "over-revving"), plus a "what breaks what" card on the PARK boards and the Options page | "If I know what makes the door fly open, I'll try to avoid it." Turns wear into a cause players can play around | S–M | `ANBHUD::DrawDamageMeter`; each wear source in `ANBCar::UpdateWear` / `ApplyCrashDamage` / `NoteSeatHop` stamps a replicated last-cause byte per part |
| H3 | **Hold E: a seat wheel.** Tap E still hops to the next free seat; hold it (~0.25 s) to show the 5 seats round the car (taken ones greyed) and pick with the mouse / stick, release to jump there | Cycling past the seat you want costs a hop each time (and door wear) | M | `ANBSquirrel` (hold timer, `Server_HopToSeat(Seat)`, reusing `HopToNextSeat`'s checks), `ANBHUD` (the wheel), gamepad X held |
| H4 | **Pick the mic** (the user, 30 Oct: "just make it so we can choose which mic we use") | Voice is a never-cut feature | M | **Done 30 Oct** (an engine patch, see CLAUDE.md): Options > Sound > Microphone. Checked on Windows in `-game` runs with a Null session: the list, opening by name, switching live, a missing mic falling back to the default. Linux only compiles: nobody has run it (Playtest.md) |

The suggestions: **collect acorns** is #11 (out of scope this month) and the story's golden acorns (S6); **checkpoints instead of time attack** is already decided for the story demo (S1, S2); **tyres that wear out and need 2 squirrels to replace** and **fuel** go to the side-tasks design session (fuel could be a jerrycan to fetch on foot, a reason to leave the car).

### Second review (31 Oct): where it went
The review is in [Playtest.md](Playtest.md#second-review-31-oct).

| # | Item | Why | Effort | Status |
|---|---|---|---|---|
| R1 | **No deadline:** a stopwatch, a bonus per second under par (9:00), TIME'S UP removed | "The timer SUCKS"; the user, 31 Oct: "remove the timer" | S | **Done 31 Oct** (1-player PIE by script). Half of S1: the par medals come with the story. Par goes to 11:00 (J5) |
| R2 | **Squirrels can't flip or sink the jeep:** 4 kg squirrels, a push of 400 N instead of 750 kN, the on-foot capsule query-only; wheel sweeps skip hits from above (a squirrel on a fender) | The clip: a squirrel at the door threw the jeep; standing over a wheel sank it | S | **Done 31 Oct** (1-player PIE by script; needs the 2-player check in Playtest.md) |
| R3 | **5 recoverable acorns, launched by careless driving on cobbles** (instead of the door flinging squirrels out): fewer, bigger acorns that fly off with physics and can be fetched on foot and carried back | "Part breaking feels unfair and boring"; the door opening "for no reason" feels lame; saving the acorns is the good part | M–L | **Not before friend test 3** (31 Oct): it changes the cargo (40 → 5), the spill rules, the door part and the score, the core of what the test checks. Folded into the golden acorns (S6) |
| R4 | **More obstacles, fewer plain straights;** straights only where speed is needed (a gap to clear) | Straight-line driving is boring; the sharp turn was the best part | M (route + re-bake) | **In friend test 3** (31 Oct): new sharp turns (J1) and placed obstacles (J3) |

### Anya's review and the user's notes (31 Oct): where they went
Both are in [Playtest.md](Playtest.md#30-oct-online-session-the-27-oct-build), from the 30 Oct online session on the 27 Oct build.
- **Anya:** the door gets damaged too often (J8, with H1); show what each player is doing (J15's crew list); the repair panel shows for a part someone else is fixing, and it doesn't always lock (J14); a flat barely changes the handling (J9).
- **The user:** par is too tight because a tyre fix needs a stop (J5, J7, J9); a crew that knows the horn scares the dog off before it bites (J10); a dog bite with no dog in sight (J11, J12); the autopilot gets stuck on rubble and roadblocks (J4); and the fork goes in with the re-bake (J2).

### Friend test 3 work (decided 31 Oct): J1–J15
Decided in a question-by-question review of the plan with the user, from the reviews above. Order and dates: [the plan](#plan-to-friend-test-3-decided-18-oct-replanned-31-oct). Also in: H1 and H2 ([above](#friend-review-30-oct-where-it-went)); H3 only if time is left.

| # | Item | Why | Effort | Touches |
|---|---|---|---|---|
| J1 | **New sharp turns** (R4, part 1): a hairpin between THE HUMPS' two humps, and two switchbacks on HOME STRETCH's rocky descent; the bridge and the jump run-ups stay straight. One re-bake of L_Route with J2 (the user has no hand edits there to lose), then `PlaceRoadblocks.py` re-places the roadblocks and pinch points on the new drops. **If the turns don't work** on the user's solo drive: rebuild the climbs like a hill road, with many turns easing the grade | Straight-line driving is boring; THE HAIRPIN was the best part (31 Oct) | M | `ANBRoute`'s segment list (grades and self-crossing: `CheckProfile`), `Tools/Route/BakeRoute.py`, `PlaceRoadblocks.py`, `SnapToGround.py` |
| J2 | **BUCK'S CUT** (S5's road, pulled forward): a fork at the top of WIGGLY ROAD. The cut runs down the mountain to COBBLE CURVE: much shorter, steep, switchbacked to stay within 18 %, rough, with cones and barriers, so a faster run for more risk (drops, crash damage, spills). Two plain signs, WIGGLY ROAD and BUCK'S CUT; no ROAD CLOSED, ranger station or excavator until the story | The user, 31 Oct: the fork goes in with the re-bake. A choice of road, and a reason to play again | L | `ANBRoute`: a branch with its own centreline, its distances mapped onto the main road's (progress, J6, the reset to the trail, "depot in X km"); terrain and landscape with both trails; the bake |
| J3 | **Obstacles on the long straights** (R4, part 2): rocks, logs and a parked car, placed as level actors like G6 on the straights J1 leaves; the jump run-ups stay clear | "More obstacles, fewer plain straights" (31 Oct) | S–M | `PlaceRoadblocks.py` (a new kind, `Ground=` tags); no re-bake |
| J4 | **The autopilot gets through.** Done when it drives both roads end to end in a `-game` window with every roadblock, pinch point and obstacle in place. It looks ahead along its line and steers round anything solid instead of aiming at the centreline, reverses and tries another line when stuck, waits for the wrecking ball, honks at a biting dog, and takes the road given by `?Road=Cut` or `?Road=Wiggly`. A dev tool only (no bots is locked) | The user, 31 Oct: it gets stuck on the rubble and on everything placed since G6, so the route has no timing | M–L | `ANBRunGameMode::TickAutopilot` |
| J5 | **Par 11:00, then solo + 4 min.** The user's solo drive (31 Oct, no breakdowns) took ~7 min, and par is the solo time + 4 min for repairs. After the re-bake: the user's solo time on the main road + 4 min (BUCK'S CUT beats it: that's its reward). Friend test 3's delivered times set it for good | The user: 9:00 is too tight, as a tyre fix needs a stop. On 30 Oct the 27 Oct build's run 2 ran out the 9:00 clock | S | `ANBRoute::ParSeconds` (and any value saved on L_Route's placed route) |
| J6 | **Breakdowns ramp with distance, not time.** The director's ramp (a failure every 14 s easing to 6 s, at most 1 rising to players + 1 failed at once, calms 40 s shrinking to 20 s) follows how far along the trail the car is | Since R1 there's no deadline, and past par the time ramp stayed at full for the rest of the run (in storms, a failure every ~4 s): a slow crew got slower | S | `UNBFailureDirector` (progress from the route distance; BUCK'S CUT through J2's mapping) |
| J7 | **Fewer random failures.** The random timer stops picking the door (J8), the four tyres count as one entry, and the interval stretches so the engine and brakes break about as often as now: a bit under half of today's random failures. Damage you cause stays (crashes, rough ground, rivers, off the trail, seat hops, the dog) | 4 of 7 random failures were flats, each a stop; "part breaking feels unfair and boring" (31 Oct) | S | `UNBFailureDirector` (candidates, `StartInterval`, `EndInterval`) |
| J8 | **The door only opens for a reason**, with H1: out of the random timer, it opens from seat hops and from crashes on its side, and H2 names the cause | "Door too fragile" (30 Oct), "flying open for no reason" (31 Oct), "still too frequent" (Anya) | S | `UNBFailureDirector`, `ANBCar::NoteSeatHop` / `ApplyCrashDamage` |
| J9 | **Fewer but nastier flats.** One flat caps the speed at ~900 cm/s (mid-3rd; today the cut is at 1400, above 4th gear's top, so it only bit in 5th). A steady pull towards the flat, no jerks. A front flat also halves the Wheel squirrel's steering (one front wheel steers instead of two), so with the speed fade (40 % at 1500 cm/s) it can't be held straight at speed and makes you slow down. A rear flat: a smaller steady pull (15 %), full steering, and its 30 % grip lets the back end slide in bends | Anya: "better mishandling when tyres are gone, still in control"; the user: "the flat should act like a flat" | S | `ANBCar::ApplyInputsToVehicle` (`FlatTopSpeed`, a front-flat steering scale), `UNBTirePart` |
| J10 | **The horn only works on a biting dog.** A honk while it has a tyre makes it let go (no flat roll), drop back a few seconds, then come again while the car is on its stretch. Honking at a chasing dog does nothing | The user: a crew that knows honks early and never gets bitten (a honk scared it for 10 s, and it couldn't catch up before its stretch ended) | S | `ANBDog::UpdateServer` |
| J11 | **Every bite visible.** It bites only the tyre on its own side and goes round the car, never through it, and only standing on open ground, never on rocks or props. The callout names the tyre ("THE DOG'S GOT THE BACK-LEFT TYRE!"), and an arrow at the screen edge points at the dog when it's out of view | The user, a client on 30 Oct, got "the dog's got a tyre" with no dog near. Bites log only on the host. Suspects: a far-side bite behind the jeep; a bite at the WIGGLY ROAD pinch point, where its feet land ~1 m up on the rocks' collision boxes; the callout reaching someone looking elsewhere | M | `ANBDog::PickTire` / `TireSpot` / `SnapToGround`, `ANBHUD::UpdateCallouts`, an off-screen marker |
| J12 | **The dog moves after the merge** (S9's ambush, pulled forward): the kennel at the start of COBBLE CURVE, the chase down HOME STRETCH, giving up at THE WASH, so both roads meet it. The chained start in the junkyard waits for the story | On WIGGLY ROAD it would miss every crew that takes the cut | S | `ANBRoute::DogSection`, `SpawnDog` (the stretch ends at THE WASH) |
| J13 | **The picnic hamper on WIGGLY ROAD** (S5's acorn cache, pulled forward): at a lookout, it refills the acorns lost so far, up to 10, once per run ("+N ACORNS!") | Makes the long road worth taking: speed or acorns | S | a new actor with a trigger, `ANBCar` acorns, `ANBHUD` |
| J14 | **One squirrel per repair; the panel only for your own.** Every part locks to the squirrel working it (today only tyres do; two squirrels can mash the engine or the door, or hit the brake ring, together). Anyone else gets a tag over the part ("ANYA'S ON IT") instead of the bottom-right panel | Anya: while someone fixes a tyre "it sometimes doesn't lock, I can also try, or at least it shows its UI". F8's "NAME IS ON IT" panel still reads as a shared widget | S–M | `UNBInteractableComponent` (a lock for Mash and TimingRing), `ANBHUD::DrawInteractable` |
| J15 | **The HUD pass, mock-up first.** One visual style (Lilita One, fur colours, Kenney icons, outlined panels) over everything players read: a **crew list** top left (each squirrel in its fur colour and what it's doing now: WHEEL / PEDALS / GEARS / RIDING / FIXING THE ENGINE, A TYRE, THE DOOR or THE BRAKES / THROWN OUT (6 s) / ON FOOT; "nobody!" still flashes for an empty control seat), J14's tags, H2's damage meter with causes, the failure list and callouts, the gear gauge, the clock and par, and the progress bar (with the fork). A mock-up page for the user to approve, then built in the canvas HUD (no move to UMG) | Anya: "better indication to show what the player is doing, sometimes confusing"; the user: "the info is not very well told" | M (mock-up) + L | `ANBHUD` |

### Tier 1: before the friend test
| # | Item | Why it's fun | Effort | Touches |
|---|---|---|---|---|
| 1 | **2-PC playtest of a full run, then tune** | Nothing has been felt by hand. Tune for comedy on purpose too: a bigger `EjectKick`, a wobblier door, more body roll. | S | [Playtest.md](Playtest.md); values that feel right go in the [GDD tuning table](GDD.md#tuning-table) |
| 2 | **Fill the sound slots:** engine loop, crash, spill, break, fire loop, repair ding, flip, righted (done 14 Oct, plus music and run events; unheard) | Audio carries a stream. | S per slot, no code | `FNBFeedback` slots on BP_OffroadCar_Pawn and its part components ([slot map](Assets.md#slot-map)) |
| 3 | **Horn** (code done 13 Oct) | Endless dumb fun, a "hey!" that cuts through voice chat, and something to spam while muted. | S | New input on `ANBSquirrel` (H / gamepad Y, seated only, short cooldown) → server RPC → `ANBCar::Multicast_Feedback` with a new `HornFeedback` slot |
| 4 | **Squirrel screams and a landing thud** (plus a chatter when grabbing a part; code done 13 Oct, slots on BP_Squirrel) | A squirrel yelling as it flies out is the genre's funniest moment. | S–M | `FNBFeedback` UPROPERTYs on `ANBSquirrel`, played in `Eject` and when the ragdoll ends. Assigning assets needs a Blueprint child (`BP_Squirrel`) that the game mode loads with `FClassFinder` like the car; it must sit in an always-cooked directory ([Assets.md](Assets.md#where-files-go)) |
| 5 | **Visible acorn pile in the bed** (code done 13 Oct) | Viewers can't see an integer. A shrinking pile makes spills readable from the chase cam. | S–M | `ANBCar::Acorns` becomes `ReplicatedUsing=OnRep_Acorns`; an ISM of acorns in the bed hides one instance per acorn lost. Header change |
| 6 | **Readable failures:** big per-failure callouts ("ENGINE ON FIRE!"), a chunky font, camera shake on crashes (code done 13 Oct; the font is Lilita One, imported 13 Oct) | Viewers can't hear game state, so the screen has to say it. | S–M | `ANBHUD` (today a small pulsing list); the crash event on `ANBCar` shakes the riders' cameras on each machine. Use a camera punch, not slow-mo: time dilation is global, so it would slow the server's physics for everyone |
| 7 | **Squirrel charm pass:** a head, big eyes with pupils, ears, a wobbly tail and googly pupils (code done 13 Oct) | Personality turns falls into character moments and makes clips recognisable. | M | `ANBSquirrel` constructor. New parts attach to `BodyVisual` so they ride the ragdoll and take the fur colour; the wobble is a cosmetic spring in Tick |
| 8 | **First-30-seconds onboarding:** WHEEL / PEDALS labels over empty seats until first used, and signs in THE PARK teaching E, Space and LMB (code done 13 Oct) | Friend testers won't read a controls hint. | S | `ANBHUD` (world-projected labels); `ANBRoute` TextRender signs |

### Tier 2: for the demo, only if the playtest agrees
| # | Item | Why | Effort | Touches |
|---|---|---|---|---|
| 9 | **The pedal squirrel can't see the road** (its camera sits low in the footwell) | The strongest co-op lever available: the Wheel squirrel has to shout GAS / BRAKE, and Acorn in Mouth on the Wheel seat becomes devastating. **Needs your OK: it changes the locked chase-camera rule for one seat.** | S–M | Seated camera setup in `ANBSquirrel::RefreshAttachedState`, Pedals seat only |
| 10 | (friend test 3) **More end-screen awards**, one per squirrel: Frequent Flyer (most falls), Butterfingers (most acorns spilled), Firefighter (most fires out), Backseat Driver (most pings), Longest Flight (farthest single throw) | End-screen screenshots are free advertising. | S–M | **Done 20 Oct** (1-player PIE, log only): `ENBAward` on `ANBPlayerState`, `PingsSent` and `LongestFlight` added, picked in `PickAwards`, drawn on the end screen. Still to see: the screen in 4-client PIE |
| 11 | **Pick spilled acorns back up** and throw them into the bed | "Run back for them!" moments, and a risk/reward between score and time. | M | Spilled `ANBAcorn`s are already server-spawned replicated actors. Extend their 20 s lifespan, add carry/throw on `ANBSquirrel`, and a bed volume on `ANBCar` that adds to `Acorns` (capped at `MaxAcorns`) |
| 12 | (friend test 3) **Fallen-squirrel cam:** while tumbling and while far behind, frame the car as well as the squirrel | A streamer who fell out still shows the chaos. The 8 s before respawn isn't idle (the squirrel can run after the car), so this frames the car rather than taking control away. | S–M | **Done 20 Oct** (1-player PIE, by script): `ANBSquirrel::UpdateFallenCamera`. Still to see: how it feels by hand, and on a client |
| 13 | (friend test 3) **One hazard with a face:** a dog that chases the car on WIGGLY ROAD. It bites a tyre (wear, sometimes a flat) or grabs a squirrel on foot and drags it a few metres; **a honk scares it off** for 10 s | Named, recurring moments ("the dog got us again") give runs a story, and the horn gets a job. | M | **Done 20 Oct** as a greybox (basic shapes, trot animated in code): `Game/NBDog`, spawned by `ANBRoute::SpawnDog`. Checked in 1-player PIE by the log (wakes, bites, a flat, a honk scares it, flings a squirrel on foot). Still to see: the look, a client's view, how often it bites in a hand-driven run. Its sound slots are empty; the Quaternius pug (CC0) can replace the greybox. **31 Oct:** J10–J12 (the horn only works on a biting dog, every bite visible, moved after the merge) |
| 14 | (friend test 3; the horn on Y is done) **Gamepad parity:** horn on Y, pings on the D-pad (1–4) and LB + D-pad (5–8) | Co-op crowds use controllers, and pings are keys 1–8 only today. | S | **Done 20 Oct** (1-player PIE: an injected D-pad press sends a ping): D-pad left / right / up / down = LEFT! / RIGHT! / GAS! / BRAKE!, with LB held HELP! / FIX IT! / JUMP OUT! / NICE!; the muted banner and legend name the pad. Still to see: a real pad, LB held |
| 15 | **Shorter run for clips** (e.g. 5 sections, ~4 min), if the 2-PC run dragged at 8:00 | Viewers should see a whole run. **Needs your OK: the 6–10 min run is in the locked plan.** | S | Segment list and `DeadlineSeconds` in `ANBRoute` |
| 16 | (done 18 Oct as G1) **Crashes break parts**, if failures feel random in the playtest | The GDD always meant breakdowns to escalate with crash damage too, but today only the director's timer breaks parts. A hard crash breaking something gives viewers cause and effect. | S | `ANBCar`'s crash detection calls `Fail()` on a healthy part, within `UNBFailureDirector`'s concurrency cap |

### Art upgrades the user asked for (14 Oct)
- **A rigged squirrel.** The squirrel is now a static low-poly model (CC-BY, Poly Pizza #3). A rigged one would let it flail while flying and grip the wheel. No free rigged CC0/CC-BY squirrel has turned up yet (the Sketchfab ones are NC or detailed, and the itch one needs payment). Options: rig the current model in Blender (a spine, tail and 4 limbs is enough for a wobbly ragdoll), or buy a cheap licence. Effort M–L.
- **A new car.** The buggy stays for the demo, but it has two problems: the door is a greybox panel bolted on (the template has no real door), and there is no bed to carry the nuts (the pile sits on the engine deck). The replacement should be an open-top pickup or a farm truck: a cab door that can really flap open, an open bed for the acorn pile, room for the 5 seats. Steps:
  1. Pick a model: Kenney **Car Kit** (CC0, the "truck"/"pickup" bodies), Quaternius **LowPoly Cars** (CC0), or a Fab vehicle (kept in `ThirdParty_Private`).
  2. Chaos needs a skeletal mesh with a bone per wheel: split the wheels off and add 4 wheel bones in Blender (the template's `SK_Offroad` is the reference for bone names and orientation), plus a physics asset.
  3. Make a new BP from `BP_OffroadCar_Pawn` (reparented to `ANBCar`), swap the mesh, then re-measure the seats, part spots, `DoorHinge` and `AcornPileSlots` (the constructor's landmark comment says how).
  4. Retune the Chaos wheels and engine, then a full 2-PC run before it replaces the buggy.
  Effort L (6–10 h). After the demo, unless the playtest says the buggy is a blocker.

### Tier 3: after the demo
| # | Item | Note |
|---|---|---|
| 17 | Twitch chat integration: chat votes to break a part or send a pothole | The best Steam-launch feature for "viewer fun first". Too big for 24 Oct. |
| 18 | Hats and cosmetics unlocked by awards | Retention, and streamers can tell their squirrel apart. |
| 19 | More parts: the horn breaks, the steering wheel falls off, seat springs launch you | Each new repair verb adds a lot. Only once the four are proven fun. |
| 20 | Route variety: shuffled sections, a daily route | Replayability for a paid version. |
| — | **A squirrel's route** (Luigi, 18 Oct): getting home over giant tree branches and rivers, using the environment, getting out to scavenge parts | Ties the game to the squirrels. Needs a new route and likely the makeshift car (A2) first. |
| 21 | Kill-cam / instant replay of the run's worst crash | Great for clips, but a replay system is a big build. |
| 22 | Bird steals a squirrel; pothole frenzy | GDD events. Good clip material once Tier 1 is proven. |
| 23 | (done 18 Oct) Key rebinding | On the Options > Controls tab. |

## Release checklist (public demo, ~Sat 12 Dec)
- [ ] A 15–20 s GIF of a squirrel thrown out of the car, from the friend-test recordings. It's the itch page's most important asset.
- [ ] Top of the page: "2–4 players (needs 2). Voice chat recommended. Steam must be running."
- [ ] Every CC-BY asset is credited on the page and in the game ([CREDITS.md](../CREDITS.md)).
- [ ] Music is CC0 / CC-BY only, with no Content ID registrations, so streamers' VODs don't get muted.
- [ ] Final title.
- [ ] The AppID decision below is applied.
- [ ] [Size phase C](#c-shipping-build-for-players-the-demo-sat-24) is done: a Shipping build ≤ 200 MB zipped and ≤ 500 MB installed, and Steam starts in it.
- [ ] Low priority: a streamer-mode toggle that hides the HUD's top-left lobby line, if that line shows anything a streamer wouldn't want on screen.

## Decisions waiting on you
| Decision | By | Recommendation |
|---|---|---|
| Ship the public itch build on **AppID 480 (Spacewar)**? Players see "Spacewar" in Steam and need Steam running, and 480 is Valve's test app. The free alternative, Epic Online Services, is a big late change. | After friend test 3 | Keep 480 for the demo (no code change, proven on 2 PCs). Move to your own AppID with the Steam release. |
| **Lighting without Lumen** ([size phase B](#b-simpler-lighting-mon-12-it-changes-the-look-so-you-check-it)) | Friend test 3's build | Accept it: a smaller download, a much better frame rate on weak PCs, and flat-colour art doesn't need bounce light. |
| **Pedal-seat blind camera** (#9) | After the 2-PC test | Try it if the Pedals seat felt like a passenger's job. |
| **Run length** (#15) | Decided 17 Oct | Done: ~50 % longer (5.4 km, 15 sections, 9:00 deadline), baked into the editable map L_Route. Since 31 Oct: no deadline, par 11:00 (J5), and new turns and a fork in the next re-bake (J1, J2). |
| **Prop collision** (F7) | Done 17 Oct | Trees (trunks), rocks, logs, stumps, fences, lamps and barriers are solid; bushes, flowers, grass and cones aren't. Say if it feels annoying. |
| **Final title** | By friend test 3 (28 Nov): ask the testers | Shortlist from the tests so far: Squarrel, Go Nuts, Roll Deez Nuts, Nuts and Bolts, Rodent Racing |
| **Public demo date** | After friend test 3 | ~Sat 12 Dec, if the scores are good |
| **Side tasks** design | Dropped from friend test 3 (31 Oct) | The story's helper vehicles and forks take the idea after the test |
| **Friend test 3's date** | Sun 15 Nov | Sat 28 Nov if the route work (J1–J4) is done by then; otherwise Sat 5 Dec |
| **The talking design session** ([Luigi's chat](Review_2026-10-31_VoiceAndAcornInMouth.md)) | After friend test 3 | The muffle gets its first test there |
| **Story** | Decided and expanded 29 Oct | Golden acorn heist over 6 days; the demo is Chapter 1, with the excavator fork and the honk test ([Story.md](Story.md)). The voice actor has the Chapter 1 script to read through. |
