# Roadmap to the itch demo

Living doc, last updated 2026-10-12. The schedule, the ranked backlog and the decisions waiting on you, from now to the public demo on **Sat 24 Oct**. It folds the two research notes from 2026-10-09 (`assets-and-improvements.md`, `free-assets-and-improvements.md`) into the approved plan and replaces that plan's schedule. Design detail lives in [GDD.md](GDD.md), asset sources and licences in [Assets.md](Assets.md), hands-on checks in [Playtest.md](Playtest.md).

Effort: **S** < 2 h, **M** 2–5 h, **L** > 5 h.

## Where we are (2026-10-12)
- M0, M1 and M2 are code-complete. M2 finished on 9 Oct, three days early. The end-screen stats (planned for M3a) and the sound/VFX slots are done too.
- All of it is verified only by Python scripts in PIE. Nobody has played a run by hand, and the only packaged build (6 Oct) predates M1 and M2.
- That build is a **1.04 GB download that installs to 3 GB**, though players need only about 0.7 GB of it ([Package size](#package-size-slim-it-before-the-playtest)).
- Every sound and VFX slot is empty. The squirrel is built from primitives: since 13 Oct it has a head, googly eyes, ears and a wobbly tail (#7).

## Rules for the last two weeks
1. **Playtest before adding anything.** Every number (spill threshold, failure rate, deadline, tire cling) is a guess until two people have played a run.
2. **Sound first.** Silent chaos reads as a bug on stream. Filling the slots is asset work, not code.
3. **Show the game state.** Viewers can't hear it. Acorns, failures and falls must read on screen in under 2 s with the sound off (the plan's viewer-fun check).
4. **Tier 2 waits for the playtest.** Nothing in it starts until the 2-PC test says the core loop is fun.
5. **Small download.** Nobody downloads a multi-GB jam game. The demo ships at ≤ 200 MB zipped, and every build's size gets written down.

## Schedule
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

### D. Optional: drop Shader Model 6 (only if B is accepted)
With Lumen, Nanite and virtual shadow maps off, nothing needs SM6. Targeting only SM5 (DX12 and DX11) leaves one small shader library: today's SM5 engine shaders are 16 MB against 202 MB for SM6. Check that the game launches on both DX12 and DX11.

Expected: about 160 MB zipped.

### Keep it small
- Write the zip and installed size into the playtest notes for every package. Anything that adds more than 20 MB needs a reason.
- Size rules for imported assets: [Assets.md](Assets.md#where-files-go).

## Backlog
Ranked within each tier. "Touches" says where the work lands, so a session can start without re-deriving it.

### Tier 1: before the friend test
| # | Item | Why it's fun | Effort | Touches |
|---|---|---|---|---|
| 1 | **2-PC playtest of a full run, then tune** | Nothing has been felt by hand. Tune for comedy on purpose too: a bigger `EjectKick`, a wobblier door, more body roll. | S | [Playtest.md](Playtest.md); values that feel right go in the [GDD tuning table](GDD.md#tuning-table) |
| 2 | **Fill the sound slots:** engine loop, crash, spill, break, fire loop, repair ding, flip, righted | Audio carries a stream. | S per slot, no code | `FNBFeedback` slots on BP_OffroadCar_Pawn and its part components ([slot map](Assets.md#slot-map)) |
| 3 | **Horn** (code done 13 Oct) | Endless dumb fun, a "hey!" that cuts through voice chat, and something to spam while muted. | S | New input on `ANBSquirrel` (H / gamepad Y, seated only, short cooldown) → server RPC → `ANBCar::Multicast_Feedback` with a new `HornFeedback` slot |
| 4 | **Squirrel screams and a landing thud** (plus a chatter when grabbing a part; code done 13 Oct, slots on BP_Squirrel) | A squirrel yelling as it flies out is the genre's funniest moment. | S–M | `FNBFeedback` UPROPERTYs on `ANBSquirrel`, played in `Eject` and when the ragdoll ends. Assigning assets needs a Blueprint child (`BP_Squirrel`) that the game mode loads with `FClassFinder` like the car; it must sit in an always-cooked directory ([Assets.md](Assets.md#where-files-go)) |
| 5 | **Visible acorn pile in the bed** (code done 13 Oct) | Viewers can't see an integer. A shrinking pile makes spills readable from the chase cam. | S–M | `ANBCar::Acorns` becomes `ReplicatedUsing=OnRep_Acorns`; an ISM of acorns in the bed hides one instance per acorn lost. Header change |
| 6 | **Readable failures:** big per-failure callouts ("ENGINE ON FIRE!"), a chunky font, camera shake on crashes (code done 13 Oct, except the font: it needs an imported OFL font) | Viewers can't hear game state, so the screen has to say it. | S–M | `ANBHUD` (today a small pulsing list); the crash event on `ANBCar` shakes the riders' cameras on each machine. Use a camera punch, not slow-mo: time dilation is global, so it would slow the server's physics for everyone |
| 7 | **Squirrel charm pass:** a head, big eyes with pupils, ears, a wobbly tail and googly pupils (code done 13 Oct) | Personality turns falls into character moments and makes clips recognisable. | M | `ANBSquirrel` constructor. New parts attach to `BodyVisual` so they ride the ragdoll and take the fur colour; the wobble is a cosmetic spring in Tick |
| 8 | **First-30-seconds onboarding:** WHEEL / PEDALS labels over empty seats until first used, and signs in THE PARK teaching E, Space and LMB (code done 13 Oct) | Friend testers won't read a controls hint. | S | `ANBHUD` (world-projected labels); `ANBRoute` TextRender signs |

### Tier 2: for the demo, only if the playtest agrees
| # | Item | Why | Effort | Touches |
|---|---|---|---|---|
| 9 | **The pedal squirrel can't see the road** (its camera sits low in the footwell) | The strongest co-op lever available: the Wheel squirrel has to shout GAS / BRAKE, and Acorn in Mouth on the Wheel seat becomes devastating. **Needs your OK: it changes the locked chase-camera rule for one seat.** | S–M | Seated camera setup in `ANBSquirrel::RefreshAttachedState`, Pedals seat only |
| 10 | **More end-screen awards**, one per squirrel: Frequent Flyer (most falls), Butterfingers (most acorns spilled), Firefighter (most fires out), Backseat Driver (most pings), Longest Flight (farthest single throw) | End-screen screenshots are free advertising. | S–M | The first three use existing `ANBPlayerState` stats; add `PingsSent` and `LongestFlight`; pick them in `EndRun`; draw them on the end screen |
| 11 | **Pick spilled acorns back up** and throw them into the bed | "Run back for them!" moments, and a risk/reward between score and time. | M | Spilled `ANBAcorn`s are already server-spawned replicated actors. Extend their 20 s lifespan, add carry/throw on `ANBSquirrel`, and a bed volume on `ANBCar` that adds to `Acorns` (capped at `MaxAcorns`) |
| 12 | **Fallen-squirrel cam:** while tumbling and while far behind, frame the car as well as the squirrel | A streamer who fell out still shows the chaos. The 8 s before respawn isn't idle (the squirrel can run after the car), so this frames the car rather than taking control away. | S–M | `ANBSquirrel` camera in the tumbling / on-foot states |
| 13 | **One hazard with a face:** a dog that chases the car on WIGGLY ROAD, or a sprinkler in THE PARK | Named, recurring moments ("the dog got us again") give runs a story. | M | `ANBRoute`; Quaternius pug (CC0) for the dog |
| 14 | **Gamepad parity:** horn on Y, pings on the D-pad (1–4) and LB + D-pad (5–8) | Co-op crowds use controllers, and pings are keys 1–8 only today. | S | `ANBSquirrel` input setup |
| 15 | **Shorter run for clips** (e.g. 5 sections, ~4 min), if the 2-PC run dragged at 8:00 | Viewers should see a whole run. **Needs your OK: the 6–10 min run is in the locked plan.** | S | Segment list and `DeadlineSeconds` in `ANBRoute` |
| 16 | **Crashes break parts**, if failures feel random in the playtest | The GDD always meant breakdowns to escalate with crash damage too, but today only the director's timer breaks parts. A hard crash breaking something gives viewers cause and effect. | S | `ANBCar`'s crash detection calls `Fail()` on a healthy part, within `UNBFailureDirector`'s concurrency cap |

### Tier 3: after the demo
| # | Item | Note |
|---|---|---|
| 17 | Twitch chat integration: chat votes to break a part or send a pothole | The best Steam-launch feature for "viewer fun first". Too big for 24 Oct. |
| 18 | Hats and cosmetics unlocked by awards | Retention, and streamers can tell their squirrel apart. |
| 19 | More parts: the horn breaks, the steering wheel falls off, seat springs launch you | Each new repair verb adds a lot. Only once the four are proven fun. |
| 20 | Route variety: shuffled sections, a daily route | Replayability for a paid version. |
| 21 | Kill-cam / instant replay of the run's worst crash | Great for clips, but a replay system is a big build. |
| 22 | Bird steals a squirrel; pothole frenzy | GDD events. Good clip material once Tier 1 is proven. |
| 23 | Key rebinding | Input is built in code, so a rebinding UI is a big build. |

## Release checklist (Sat 24)
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
| Ship the public itch build on **AppID 480 (Spacewar)**? Players see "Spacewar" in Steam and need Steam running, and 480 is Valve's test app. The free alternative, Epic Online Services, is a big late change. | Wed 21 | Keep 480 for the demo (no code change, proven on 2 PCs). Move to your own AppID with the Steam release. |
| **Lighting without Lumen** ([size phase B](#b-simpler-lighting-mon-12-it-changes-the-look-so-you-check-it)) | Mon 12, after the look check | Accept it: a smaller download, a much better frame rate on weak PCs, and flat-colour art doesn't need bounce light. |
| **Pedal-seat blind camera** (#9) | After the 2-PC test | Try it if the Pedals seat felt like a passenger's job. |
| **Run length** (#15) | After the 2-PC test | Keep 8:00 unless the test run dragged. |
| **Final title** | Before Sat 24 | — |
