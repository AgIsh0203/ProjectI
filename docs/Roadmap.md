# Roadmap to the itch demo

Living doc, last updated 2026-10-18. The plan to friend test 3 (**Sat 28 Nov**), the ranked backlog and the decisions waiting on you. It folds the two research notes from 2026-10-09 (`assets-and-improvements.md`, `free-assets-and-improvements.md`) into the approved plan and replaces that plan's schedule. Design detail lives in [GDD.md](GDD.md), asset sources and licences in [Assets.md](Assets.md), hands-on checks in [Playtest.md](Playtest.md).

Effort: **S** < 2 h, **M** 2–5 h, **L** > 5 h.

## Plan to friend test 3 (decided 18 Oct)
The 24 Oct demo is dropped. Friend test 3 on **Sat 28 Nov** checks whether the redesign (mountain trail, earned damage, failures that matter, the new seat) is fun. It is a test, not a launch: the public demo comes after a round of fixes.

- **The test:** 4 players if possible, at least 2 who have never played (the only real check of onboarding and the who-is-who HUD), 3 runs each (is it still fun once the route is known?).
- **Next week (19–25 Oct):** more testers play the **17 Oct build** (`2026-10-17-fix`), told the known problems first ([Playtest.md](Playtest.md#known-problems-in-the-17-oct-build-tell-testers-first)). New problems they find get extra time from the user; **the scope below is locked**.
- **Budget:** ~90 h to 28 Nov (~15 h a week): ~65 h planned, ~25 h spare for what testers find. All work done by **Sun 22 Nov**.
- **No hands-on run by the user is guaranteed before 7 Nov** (a 2-PC run is hard to set up); 4-client PIE is the check until then.

| When | ~h | Work | Done when |
|---|---|---|---|
| 19 Oct – 6 Nov | 45 | In order: **1. Gears seat + handbrake** (G5), with the closer camera (G7, small). **2. Rigged squirrel** (A1; the user installs Blender 4.x LTS this week). **3.** #10 awards, #12 fallen-squirrel cam, #14 gamepad pings, #13 the dog, the roadblocks + a wrecking ball (G6). | All of it works in 4-client PIE. If the rig runs late it moves to the second half and the 7 Nov build goes without it. |
| **Sat 7 Nov** | 3 | **Mid-point check:** a Development build of `main`; **the user plus one friend online** (each on their own PC), against the Playtest.md checklists. | A run start to finish by hand; the worst problems listed. |
| 8 – 22 Nov | 20 + spare | The 7 Nov fixes first. Then the stretch, in order: **the new car** (A2), then **side tasks** (they get their own design session before any code). | Nothing open from 7 Nov. |
| 23 – 27 Nov | 4 | Package, a smoke test, update the questionnaire (gears, roadblocks, the title shortlist), send the build out. | Testers have the build. |
| **Sat 28 Nov** | — | **Friend test 3.** Record it; the form. | Recordings and answers collected. |
| 29 Nov – ~12 Dec | — | Fixes from the test, then the release work: Shipping build (size phases C and D), itch page, AppID 480, final title. | **Public demo ~Sat 12 Dec** (tentative, the user's call after the test). |

**Out of scope this month:** #9 (pedal-seat blind camera), #11 (pick acorns back up), more route length (wait for the 7 Nov run's time), Tier 3.

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

### D. Optional: drop Shader Model 6 (only if B is accepted)
With Lumen, Nanite and virtual shadow maps off, nothing needs SM6. Targeting only SM5 (DX12 and DX11) leaves one small shader library: today's SM5 engine shaders are 16 MB against 202 MB for SM6. Check that the game launches on both DX12 and DX11.

Expected: about 160 MB zipped.

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
Built in this order (see [the plan](#plan-to-friend-test-3-decided-18-oct)). The Tier 2 picks (#10, #12, #13, #14) are in their table below.

| # | Item | Why | Effort | Touches |
|---|---|---|---|---|
| G5 | **Gears seat** on the PassengerSeat (beside the wheel): W/S shift up / down. **Host option** "Manual gears: OFF / ON" on the host's lobby page next to Start the run (and on Practice alone), seen by every client, locked once the countdown starts. Off = automatic, as today. On = manual, and with the Gears seat empty the car **stays in its gear** (bogs on climbs, tops out on the flat) until someone hops over. Too high a gear bogs, over-revving wears the engine and can break it. The HUD shows the gear and an RPM needle with a red zone to everyone; "nobody!" flashes for an empty Gears seat as for the Wheel. Manual gears score **×1.25** ("Manual gears ×1.25" on the end screen; bad endings still score 0). **Handbrake** (the user, 18 Oct, after Luigi's review): the Gears seat also holds the handbrake, with manual gears on or off, so the seat always has a job. Hold a new rebindable `ENBBinding::Handbrake` (default Shift; gamepad RB) to lock the rear wheels: the Wheel squirrel calls it for THE HAIRPIN and the cliff bends, and pulling it at the wrong moment spins the car. The HUD shows "HANDBRAKE!" over the car while it's on. The pedals stay one seat (gas + brake): no split. | Gives a 3rd and 4th squirrel a real job; the 18 % climbs make the gear matter. Asked for in friend test 1 | M | `ANBCar` (Chaos manual transmission, `SetHandbrakeInput` from the seat, over-rev wear in `UpdateWear`), `ANBSquirrel` seated input, `UNBControlSettings` (new binding), `ANBRunGameState` (replicated option), `UNBLobbyMenu`, `ANBRunGameMode` score, `ANBHUD` |
| A1 | **Rigged squirrel** (the first of the [art upgrades](#art-upgrades-the-user-asked-for-14-oct)): rig the current Poly Pizza model with Blender's Python (spine, tail, head, 4 limbs, automatic weights), export FBX. **Cosmetic only**: the replicated ball stays the physics; bones are driven in code on each machine, no replication. Poses: flail while flying, grip while seated, a simple run cycle on foot, mash / tug while repairing. No physics-asset ragdoll. Fallback if the weights look bad: a cheap paid rigged model in `ThirdParty_Private`. | A squirrel flailing as it flies out is the clip | M–L | `ANBSquirrel` (`ModelVisual` becomes a skeletal mesh; the charm code drives the bones). The user installs Blender 4.x LTS |
| G6 | **Roadblocks on narrow roads:** a **broken-down car** on a cliff ledge at PINE PASS (Kenney Car Kit, CC0), a **fallen tree** across the inside of THE HAIRPIN, a **small rockslide** on ROCKY ROAD. Each leaves a gap of about a car width + 1 m beside a drop: slow and careful gets through; rushing it clips the block (crash damage) or goes over the edge (Hold R, −3 acorns). Placed as level actors in L_Route with `Ground=` tags (SnapToGround), **no re-bake**, so the hand edits survive. **Plus (Luigi, 18 Oct): one moving hazard,** a swinging wrecking ball (or log) over a narrow stretch, timed so a crew that waits gets through and one that rushes gets hit (crash damage, a squirrel knocked out). And **pinch the trail** at 2–3 cliff edges with rocks and fallen logs placed the same way, so falling off is a real risk without narrowing the baked trail (a narrower trail everywhere needs a re-bake, which loses the hand edits). | Makes Wheel and Pedals talk ("slow… slow… STOP"). "Incredibly easy" (Luigi) | S–M (+ S for the swinging hazard: a server-moved kinematic actor, replicated) | L_Route via MCP / Python; `ANBProp`; a small hazard actor |
| G7 | **A closer camera** (Luigi, 18 Oct: "unnecessarily distant"): try `SeatedArmLength` ~420 (650 today) and a lower pivot in 4-client PIE; keep the chase camera (locked). Add a "Camera distance" slider on Options > Controls if one value doesn't suit everyone. | The car and the squirrels read bigger on stream | S | `ANBSquirrel` defaults, `UNBControlSettings` |
| A2 | Stretch, after 7 Nov: **the new car** (steps under [art upgrades](#art-upgrades-the-user-asked-for-14-oct)). **Idea (Luigi, 18 Oct):** make it a makeshift squirrel car (acorn-shell or crate body, bottle-cap wheels, twig frame, Flintstones-style) instead of another normal car, so the setting says "squirrels". | Fixes the bolted-on door and the acorns on the engine deck | L | See the art upgrades |
| — | Stretch, after the car: **side tasks / vehicle swaps** (R.V. There Yet?). A design session comes first; take Luigi's review into it: **hop out to find spare parts** for the makeshift car, and the depot becomes the crew's **home tree** (the acorns are the winter stash). | "Too short" in 4 of 5 answers | L | — |

### Luigi's review (18 Oct): where it went
The review is in [Playtest.md](Playtest.md#luigis-extra-review-discord-18-oct-same-17-oct-package). It was played on the 17 Oct package: flat route, failures that didn't matter yet, no F1–F9. The mountain trail's cliffs, fords and gorge were already the answer to part of "incredibly easy"; nobody outside has played them.
- **One job each (gas / brake / steer / repair):** the pedals stay one seat (the user). The Gears seat gets the handbrake (G5).
- **Narrow track, falls, moving obstacles:** G6 (the pinch points and a wrecking ball). Judge the difficulty at the 7 Nov run before going further.
- **Closer camera:** G7. First person: no (the chase camera is locked).
- **Every repair from inside the car:** no. The tyre is on foot on purpose (clinging to the moving wheel), and getting out is what Luigi's setting idea wants too. Every other part can already be reached from a seat.
- **"RV There Yet?":** the 18 Oct terrain was built in that style; keep it in mind for the side-tasks design session.
- **The squirrel setting (tree branches, home, makeshift car, spare parts):** A2 and the side-tasks session for now. A route over giant tree branches is a bigger idea for after the public demo ([Tier 3](#tier-3-after-the-demo)).

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
| 10 | (friend test 3) **More end-screen awards**, one per squirrel: Frequent Flyer (most falls), Butterfingers (most acorns spilled), Firefighter (most fires out), Backseat Driver (most pings), Longest Flight (farthest single throw) | End-screen screenshots are free advertising. | S–M | The first three use existing `ANBPlayerState` stats; add `PingsSent` and `LongestFlight`; pick them in `EndRun`; draw them on the end screen |
| 11 | **Pick spilled acorns back up** and throw them into the bed | "Run back for them!" moments, and a risk/reward between score and time. | M | Spilled `ANBAcorn`s are already server-spawned replicated actors. Extend their 20 s lifespan, add carry/throw on `ANBSquirrel`, and a bed volume on `ANBCar` that adds to `Acorns` (capped at `MaxAcorns`) |
| 12 | (friend test 3) **Fallen-squirrel cam:** while tumbling and while far behind, frame the car as well as the squirrel | A streamer who fell out still shows the chaos. The 8 s before respawn isn't idle (the squirrel can run after the car), so this frames the car rather than taking control away. | S–M | `ANBSquirrel` camera in the tumbling / on-foot states |
| 13 | (friend test 3) **One hazard with a face:** a dog that chases the car on WIGGLY ROAD. It bites a tyre (wear, sometimes a flat) or grabs a squirrel on foot and drags it a few metres; **a honk scares it off** for 10 s | Named, recurring moments ("the dog got us again") give runs a story, and the horn gets a job. | M | `ANBRoute`; Quaternius pug (CC0) for the dog |
| 14 | (friend test 3; the horn on Y is done) **Gamepad parity:** horn on Y, pings on the D-pad (1–4) and LB + D-pad (5–8) | Co-op crowds use controllers, and pings are keys 1–8 only today. | S | `ANBSquirrel` input setup |
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
| **Lighting without Lumen** ([size phase B](#b-simpler-lighting-mon-12-it-changes-the-look-so-you-check-it)) | The 7 Nov session | Accept it: a smaller download, a much better frame rate on weak PCs, and flat-colour art doesn't need bounce light. |
| **Pedal-seat blind camera** (#9) | After the 2-PC test | Try it if the Pedals seat felt like a passenger's job. |
| **Run length** (#15) | Decided 17 Oct | Done: ~50 % longer (5.4 km, 15 sections, 9:00 deadline), baked into the editable map L_Route. |
| **Prop collision** (F7) | Done 17 Oct | Trees (trunks), rocks, logs, stumps, fences, lamps and barriers are solid; bushes, flowers, grass and cones aren't. Say if it feels annoying. |
| **Final title** | By friend test 3 (28 Nov): ask the testers | Shortlist from the tests so far: Squarrel, Go Nuts, Roll Deez Nuts, Nuts and Bolts, Rodent Racing |
| **Public demo date** | After friend test 3 | ~Sat 12 Dec, if the scores are good |
| **Side tasks** design | After 7 Nov | A design session before any code |
