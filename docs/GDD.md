# Squirrel Wheels (working title) — Prototype GDD

Living doc, last updated 2026-10-31. Prototype: 20 days (Mon 5 – Sat 24 Oct 2026), free demo on itch. **Viewer fun first, player fun second.**

Schedule and ranked backlog: [Roadmap.md](Roadmap.md). Asset sources and licences: [Assets.md](Assets.md). Hands-on checks: [Playtest.md](Playtest.md).

**Status tags:** **Built** = in the game and verified by script in PIE, not yet felt by hand. **Decided 31 Oct** = to build for friend test 3 (Sat 28 Nov; Roadmap J1–J15). **Demo** = for the public demo after friend test 3, if the test agrees. **Later** = after the demo. Numbers like (#5) are Roadmap backlog items.

## Pitch
2–4 tiny squirrels drive one human-sized, open-top, falling-apart car from the park to the depot. One holds the WHEEL, one jumps on the PEDALS, and everyone else (plus anyone who bails from a seat) keeps the car alive.

**Story (decided 29 Oct, Later: after friend test 3):** the squirrels steal **golden acorns** from a gourmet delivery truck, are carried far from home, build the junk jeep at the junkyard where the truck crashes, and race home before the first snow. It brings a stopwatch instead of the deadline, checkpoints, chapters joined by camps, road forks, helper vehicles (an excavator first) and a CB-radio voice. Everything is in [Story.md](Story.md). The sections below describe the game as built today.

## Core loop (6–10 min run, 9:00 par) — Built
- Both seats must be filled to drive. Squirrels hop between stations constantly.
- Breakdowns come faster as the run goes on. Each repair has its own readable action:

  | Part | Failure | Repair |
  |---|---|---|
  | Engine | overheats → power fades → stall | mash to cool (14 presses) |
  | Brakes | fade to nothing | timing-ring pump (3 hits) |
  | Tire ×4 | flat (grip loss) | jump out and hold for 3 s, clinging to the moving wheel |
  | Door | swings open, throws the passenger out | slam it shut (3 presses) |

- **Wear (built 17 Oct, after the friend test):** failures are still random, but how you drive tilts which part goes. Speeding wears the engine and rear tires, steering hard at speed wears the front tires, hammering the brakes (or never touching them for 30 s) wears the brakes, and hopping seats rattles the door loose. A worn part is up to 6× likelier to be the next failure and can break on its own past 60 % wear; everything heals slowly when driven gently. A repair buys a full interval of calm, and a failure ignored for 25 s stops holding the others off. Since friend test 2 (18 Oct) damage is earned more than rolled: a hard sideways or head-on hit wears the part on that side and a very hard one breaks it, rough ground wears the tyres, nothing breaks by itself while the car stands still, and timed failures lean harder on worn parts.
- **Decided 31 Oct, to build for friend test 3 (Roadmap J6–J9):** breakdowns ramp with distance along the trail, not with time (with no deadline, the time ramp stayed at full past par for the rest of the run); about half as many random failures (the door leaves the random pick, the four tyres count as one, and the gaps stretch so the engine and brakes keep their rate); the door opens only from seat hops and from crashes on its side; flats get rarer but nastier (a ~900 cm/s cap, a steady pull, a front flat halves the steering, a rear flat lets the back end slide).
- Anyone who falls out ragdolls, gets up 2.5 s later, then chases the car or respawns at it after 8 s with a score penalty.
- Flipped car: 2 squirrels push at once to right it.
- **Score:** acorns still in the open bed at the depot (10 each), plus 2 per second left, minus 25 per respawn. The 40 acorns spill on bumps, hard braking and crashes; a flip dumps a quarter of them.
- **Bad ending:** a total wreck (3+ parts failed for 10 s) scores 0. **No deadline since 31 Oct** (a review: "the timer SUCKS", it felt frustrating, not hard): the clock is a stopwatch, and a delivered run scores 2 points per second under par (9:00; 11:00 decided 31 Oct, J5: the user's solo drive took ~7 min, plus 4 min for repairs); past par the clock dims and the bonus is 0, nothing else happens. Every run ends on the stats screen.
- **With the story (Later, [Story.md](Story.md)):** the stopwatch gets per-chapter par times (Golden Acorn / Acorn / Late medals). Score = acorns × 10 + a bonus for beating par − respawns. Running out of time no longer loses: you just get the late ending. A total wreck rewinds to the last checkpoint with +30 s instead of ending the run.

## Route — Built
The park-to-depot trail: about 5.4 km in 17 named sections, each announced by a roadside sign and a HUD banner. Par is 9:00 (the deadline until 31 Oct; 11:00 decided 31 Oct, J5). It was 3.5 km / 8:00 until friend test 1 (17 Oct), where run 1 finished with half the deadline left and everyone called it too short; the sections added then are marked *new*.

**Since 18 Oct it's a mountain trail ("RV There Yet?" style, the user's call):**
- a rough dirt track over a real Landscape, 27–156 m high;
- climbs and descents up to 18 % (6 % in THE PARK);
- cliffs where the trail cuts into a slope;
- two fords and a river in a gorge under THE BRIDGE.

**Decided 31 Oct, to build for friend test 3 (Roadmap J1–J3), in one re-bake:** a hairpin between THE HUMPS, two switchbacks on HOME STRETCH's rocky descent, and **BUCK'S CUT**, a fork at the top of WIGGLY ROAD: a short, steep, switchbacked construction road down to COBBLE CURVE, faster and riskier (the excavator only comes with the story). Obstacles go on the straights that are left; the bridge and the jump run-ups stay straight. If the turns don't work, the climbs become a switchbacked hill road.

A squirrel on foot in a river is swept away, so nobody can hop out mid-ford to fix a tyre. Hidden pebbles in the fords chew the tyres. The full design is in [Terrain.md](Terrain.md).

The trail is generated by `ANBRoute` (`bUseTerrain`) and then **baked into the map `L_Route`** (`Tools/Route/BakeRoute.py`), landscape included. Every board, ramp, rock and tree is an ordinary actor you can move, delete or copy in the editor, and the game plays the map as saved. After sculpting the landscape, `Tools/Route/SnapToGround.py` puts the props back on the ground (a rebake would erase the sculpting).

Heights below are the trail's range in each section, and the grade is the steepest climb / descent (the 18 Oct bake).

| # | Section | Length | Height, grade | What happens |
|---|---|---|---|---|
| 1 | THE PARK | 400 m | 68–86 m, ≤ 6 % | Warm-up and a gentle S-bend; the control boards. |
| 2 | SQUIRREL CREEK *(18 Oct)* | 120 m | 87–97 m, 18 % / 18 % | Down into the first ford (60 cm of water, hidden pebbles) and out again. |
| 3 | COBBLE LANE | 160 m | 73–90 m, 18 % | Extra-lumpy trail, partly on a bend: acorns jump out. |
| 4 | ACORN HILL | 320 m | 83–111 m, 18 % | A big hump on top of the climb. |
| 5 | PINE PASS *(new)* | 460 m | 90–121 m, 18 % | Sweeping bends through the trees, two of them rocky. |
| 6 | THE HAIRPIN | 250 m | 73–89 m, ≤ 10 % | A 170° turn (~50 m radius): brakes and steering have to cooperate. |
| 7 | THE BRIDGE | 300 m | 76–81 m, level on the deck | An 8 m-wide deck with no rails, 9 m over a river in a gorge. |
| 8 | THE HUMPS *(new)* | 440 m | 67–110 m, 18 % | Two humps (4 m and 5 m) on the climb. |
| 9 | ROCKY ROAD | 340 m | 89–146 m, 18 % | The big climb between rock walls, with rocks on the trail: the roughest section (the autopilot tips 30° here). |
| 10 | SQUIRREL SLALOM | 260 m | 143–156 m, ≤ 10 % | Pillars alternating left and right every 28 m, on the summit. |
| 11 | THE BIG JUMP | 160 m | 138–142 m, ≤ 3 % | A 2.2 m kicker in the middle of the trail; there's room to go round it. A clean landing keeps the acorns and earns air points. |
| 12 | LITTLE JUMPS *(new)* | 400 m | 119–138 m, 18 % down | Two smaller kickers (1.4 m, 1.6 m), then the descent. |
| 13 | WIGGLY ROAD | 660 m | 70–126 m, 18 % | Six 60° bends down the mountain, two of them cobbled. |
| 14 | COBBLE CURVE *(new)* | 360 m | 77–95 m, 18 % | A long lumpy 90° left-hander. |
| 15 | HOME STRETCH | 450 m | 27–76 m, 18 % | A gentle bend, then a rocky descent where the brakes matter. |
| 16 | THE WASH *(18 Oct)* | 180 m | 36–59 m, 18 % | The second ford, at the bottom of the valley. |
| 17 | THE DEPOT | 120 m | 59–62 m, ≤ 2 % | The drop-off gate, then the depot shed. |

With the story (Demo), THE PARK, HOME STRETCH and THE DEPOT become THE JUNKYARD, THE DESCENT and THE CAMPGROUND, and a fork at the top of WIGGLY ROAD adds BUCK'S CUT ([Story.md](Story.md#chapter-1-the-crash-site-the-demo)).

The autopilot (`NBAuto`, no breakdowns, 16 m/s) drove the whole trail in 7:52 with the template car and in 6:40 at 435 Nm (`-game` window). With the 80 Nm car (18 Oct, slower pickup) it gets stuck on the rubble of the 18 % climbs, so there is no full timing; the sections it drove took ~25 % longer than at 435 Nm, which suggests ~8:20 with no breakdowns. The user's solo drive (31 Oct, no breakdowns) took ~7 min; par becomes the solo time + 4 min for repairs, 11:00 (J5), measured again after the re-bake.

## Events
- **Acorn in Mouth — Built.** Every 75–105 s while driving, weighted toward the WHEEL seat and squirrels next to a failed part. That squirrel's voice is muffled for 20 s (a 450 Hz low-pass everyone hears; it was muted until 31 Oct, when Luigi found it an annoyance rather than a challenge: [the chat](Review_2026-10-31_VoiceAndAcornInMouth.md)), it still hears everyone, and keeps 8 pings on keys 1–8: LEFT!, RIGHT!, GAS!, BRAKE!, HELP!, FIX IT!, JUMP OUT!, NICE! On a gamepad (20 Oct, #14) the D-pad sends the first four (left, right, up, down) and LB + D-pad the other four. Anyone can ping, muted or not.
- **One hazard with a face — Built 20 Oct (#13), greybox:** a big brown dog sleeps by its kennel 30 m into WIGGLY ROAD. During a run, while the car is on its stretch (from 50 m before WIGGLY ROAD to COBBLE CURVE), it chases at 1450 cm/s (a car flat out escapes): it hangs off the nearest tyre for 2.5 s (wear 0.15/s, a 35 % chance of a flat when it lets go, then 5 s before the next bite), or grabs a squirrel on foot within 9 m and flings it ("THE DOG GOT YOU!"). A honk within 25 m scares it off for 10 s. WOOF! / CHOMP! / YIPE! over it, and "THE DOG'S GOT A TYRE!  HONK!" as a callout. Checked by script in PIE only. With the story (Demo) it becomes TINY: chained in THE JUNKYARD, then a chase out of it that can't bite, then this ambush from COBBLE CURVE down THE DESCENT ([Story.md](Story.md#cobble-curve-and-the-descent-tinys-ambush)). **Decided 31 Oct (J10–J12):** the horn only works on a biting dog (it lets go, drops back and comes again; on 30 Oct a pre-emptive honk ended the whole encounter); it bites only from its own side and on open ground, and the callout names the tyre, with an arrow to the dog; it moves after the merge, from COBBLE CURVE to THE WASH.
- **Roadblocks and a wrecking ball — Built 20 Oct (G6), placed in L_Route:** a broken-down sports car across PINE PASS (1445 m, 26 m drop beside it), a fallen pine across THE HAIRPIN (1550 m, 56 m drop), a rockslide on ROCKY ROAD (2465 m), and three pinch points of rocks and logs at the biggest drops elsewhere (COBBLE LANE 560 m, WIGGLY ROAD 3660 m, HOME STRETCH 4700 m). Each covers the trail from the wall side and leaves 3.5 m (pinch points 4.5 m) open along the drop: slow and careful gets through, rushing clips the block (crash damage) or goes over the edge (Hold R, −3 acorns). On THE BRIDGE (1850 m, the 8 m deck has no rails) a wrecking ball swings across the deck every 4.5 s: it shoves the car (in PIE it knocked the autopilot's car off the bridge into the gorge) and knocks out any squirrel it touches ("WRECKING BALL!"). Checked by script in PIE; nobody has driven through them.
- **Bird steals a squirrel; pothole frenzy — Later (#22).**

## The squirrels
- **Built:** a low-poly squirrel model (CC-BY, Poly Pizza), rigged in Blender (18 Oct) and tinted with a unique fur colour per player, wearing the googly eyes on its head bone; the primitives stay as the fallback and the ragdoll ball. Animations, picked from what the squirrel is doing: Idle, Run (rate follows speed), Hop (seat to seat along an arc instead of teleporting, and in the air on foot), Grip (Wheel and Pedals seats), Flail (thrown out), Mash (door, brakes), Tug (clinging to a tire, pushing the flipped car), TailFan (the engine fire: back to the flames, fanning them with its tail). **Friend test:** do the hop and TailFan read on stream?
- **Charm pass — Built (#7):** a head, big eyes with googly pupils (they sag under gravity, slosh with acceleration and bounce off the rim), cone ears, a cream belly, a pink nose, and a tail on a spring that lags behind acceleration and sways when idle. The head, belly and tail ride the tumbling body in a fall.
- **Voice — Built, sounds in (#4):** a scream when thrown out, a thud each time the tumbling body hits something hard (louder for harder hits), a chatter when grabbing a part. The slots are on `BP_Squirrel`.
- **Horn — Built, bicycle horn (#3):** any seated squirrel honks with H or gamepad Y (0.25 s cooldown). A "HONK!" pops over the car for 0.6 s, so viewers see it with the sound off.

## Readability and juice
Viewers can't hear game state, so every failure must read on screen in under 2 s with the sound off.
- **Built:** a pulsing failure list, "!" markers over broken parts, the timing ring with NICE / MISS, a "CAR FLIPPED!" banner, section banners, and sound/VFX slots on every car and part event (sounds filled 14 Oct with CC0 clips; VFX still empty), menu and run music, countdown beeps and end jingles.
- **Built (#5, #6):** a big outlined callout per failure ("ENGINE ON FIRE!", "THE BRAKES ARE GOING!", "FLAT TIRE!", "THE DOOR FLEW OPEN!") that slams in, shakes and fades over 2.4 s, newest biggest, at most 3. A camera punch on crashes: full for riders, fading out to 25 m for squirrels on foot. An acorn pile on the engine deck, one sphere per acorn, that empties from the top as acorns spill.
- **Built:** the callouts, "HONK!" and the seat labels use Lilita One, a chunky OFL font.
- **Built (14 Oct):** toon VFX from code-built spheres and cubes: crash sparks and dust, an acorn-brown spill puff, flip dust, "righted" stars, engine fire with a smoke column (bigger as it gets worse), brake sparks while the brakes are failing, tire smoke, green repair stars, a landing puff. Sound is in every slot (#2).
- **No slow-mo:** time dilation is global in multiplayer and would slow everyone's physics. A camera punch does the job instead.
- **Tuned for comedy** at the 2-PC test (#1): bigger ejection kicks, a wobblier door, more body roll.

## Onboarding — Built (#8)
- WHEEL ("steers") / PEDALS ("gas + brake") labels bob over the two driving seats until someone first sits in each (per map load).
- Four boards on the left of THE PARK, in the order they are needed: "E: GET IN / HOP SEAT" (in view from the start), "LMB: FIX BROKEN PARTS", "SPACE: JUMP OUT", "H: HONK!". Keyboard only.
- The controls hint stays on the HUD (Built), as key or Xbox button icons that follow whichever device was used last (14 Oct).

## Gears seat — Built (18 Oct, scripted 2-client PIE only) (G5)
The PassengerSeat beside the wheel. It always holds the **handbrake** (hold Shift / RB: the rear wheels lock, for THE HAIRPIN and the cliff bends, or a spin at the wrong moment). With the host's **Manual gears** option on it also shifts (W/S), and an empty Gears seat leaves the car stuck in its gear; manual runs score ×1.25. The pedals stay one seat (gas + brake). The squirrel there can still slam the door. Spec: [Roadmap.md](Roadmap.md#friend-test-3-scope-locked-18-oct).
- **What PIE showed (18 Oct):** with the template's gear ratios 1st alone reached ~2200 cm/s and the automatic box never left it, so shifting didn't matter (the user, solo drive: "comfortably able to move at good speed without changing gear"). The gears were respaced the same day, and the power cut twice: first for a lower top speed (~1890 cm/s, was ~3000), then for a slower pickup (the user: most of the game in 3rd–4th, 2nd for slow parts, 5th only after 10+ s on a straight). The engine's torque curve is 0 from 5000 rpm, so the ratios set each gear's top and the torque how fast you get there. At 85 Nm the automatic reaches 2nd at 1.8 s, 3rd 3.4 s, 4th 7.2 s, 5th 9.9 s; 80 Nm is the setting, so a bit later. Top speed ~1650 on a long straight; 18 % climbs go in 1st/2nd. The red zone starts at 65 % of the rev range (~4900 rpm); 1st at its top speed sits near 6000 rpm, so staying there wears the engine fast.

## Co-op levers — Demo, if the playtest agrees
- **The pedal squirrel can't see the road (#9).** Its camera sits low in the footwell, so the WHEEL squirrel has to shout GAS / BRAKE. Asymmetric information forces talking, and talking is what makes streams funny. **Not decided:** it bends the locked chase-camera rule for one seat.
- **Pick spilled acorns back up (#11)** and throw them into the bed: "run back for them!" moments.
- **Fallen-squirrel cam (#12) — Built 20 Oct:** while tumbling, or on foot more than 12 m from the car during a run, the camera turns to look past the squirrel at the car (so W runs back to it) and pulls back with distance (320 → 750 cm by 40 m). Moving the mouse/stick holds it off for 1.5 s. Checked by script in 1-player PIE; not felt by hand.

## End screen
Built for screenshots.
- **Built:** the score breakdown; each squirrel's falls, fires put out, acorns spilled and drive time; MVP and "Most useless", picked by a helpfulness score (+3 per fire, +1 per 30 s driving, −1 per fall, −2 per respawn, −0.5 per acorn spilled). Not seen on screen yet.
- **Built 20 Oct (#10):** one funny award per squirrel, in its fur colour under MVP / Most useless: Frequent Flyer (most falls), Butterfingers (most acorns spilled), Firefighter (most fires out), Backseat Driver (most pings), Longest Flight (farthest single throw, launch to landing; a river sweep doesn't count). Each award goes to whoever holds the biggest share of its stat, so a squirrel best at two things takes the one it dominates and the other goes to someone else; nobody gets one for a stat of 0. Checked in 1-player PIE only (the log, not the screen).

## Presentation
- **Camera — Built:** third-person chase camera, pulled wide while seated; it swings behind the car while driving.
- **Art:** Kenney and Quaternius CC0 low-poly (flat colour) to dress the route and the depot.
- **The junk jeep — Built 26 Oct (A2):** the car is a broken toy jeep the squirrels patched with trash, and every part that breaks is a piece of trash you can see: the **engine** is a car battery strapped on the hood (soda-can exhaust), the **brakes** a sponge jammed behind the rear-right tyre on a twig lever, the **door** a cardboard flap taped on the passenger side (it's right-hand drive, so the door is on the left), tape on the fenders, bottle-cap headlights, twigs over the cracked windscreen, and the acorns ride in a cardboard crate across the back. Jobs: the hood seat fixes the battery, the back seat the sponge, the Gears (passenger) seat the door. It drives exactly like the buggy (same skeleton, wheels and physics). `?Car=Buggy` brings the old buggy back. Checked in 1-player PIE and an autopilot drive; played online on 30 Oct (the 27 Oct build).
- **Lighting:** a sky light and sun with regular shadow maps. Lumen, ray tracing and virtual shadow maps go if the Mon 12 look check agrees: they cost download size and frame rate, and flat-colour art doesn't need bounce light.
- **Audio:** one cheap, rattly engine loop pitched by RPM (a lawnmower or moped beats a real car); impacts, clangs, fire crackle, repair dings, acorn pops. Music is CC0 / CC-BY with no Content ID, so streams stay unmuted.
- **UI:** a chunky rounded font (Lilita One, in for the shout text; the rest of the HUD still uses the engine font) and Kenney input prompts. **Decided 31 Oct (J14, J15):** a HUD pass in one style, mock-up first, with a crew list (who's doing what); every repair locks to one squirrel, and the others see a tag over the part instead of the repair panel.
- **Proximity voice — Built** (Steam VOIP): full volume within 8 m, silent about 25 m further.

## Online
- **Built:** Steam, listen server, up to 4 squirrels; host / invite / join / start / restart / leave from the lobby menu; AppID 480 in dev.
- **Open question:** AppID 480 for the public itch build ([Roadmap.md](Roadmap.md#decisions-waiting-on-you)).

## Story mode — Demo (Chapter 1, after friend test 3) / Later (Ch2–6)
Design in [Story.md](Story.md); build order in [Roadmap.md](Roadmap.md#story-demo-after-friend-test-3).

| Feature | Status |
|---|---|
| Stopwatch and par replace the deadline (medals: Later) | Built 31 Oct |
| Checkpoints at each section start and camp; a wreck rewinds to the last one (+30 s); host Continue | Demo |
| A camp that waits for the jeep and every squirrel ("WAITING FOR <name>") | Demo |
| CB radio lines (Dispatch, Foreman Buck, Gravy Train, Night Owl), subtitled, event-triggered: 114 for Chapter 1 in [story/RadioScript.md](story/RadioScript.md) | Demo |
| THE PARK becomes THE JUNKYARD (the crashed Acornucopia truck, TINY chained up, the deer); the jeep starts in pieces and building it is the tutorial; an intro card on the loading screen | Demo |
| Section renames: THE PARK → THE JUNKYARD, HOME STRETCH → THE DESCENT, THE DEPOT → THE CAMPGROUND | Demo |
| Road fork at the top of WIGGLY ROAD: BUCK'S CUT, where the excavator (Tracks / Boom / Bucket) knocks down a ranger station, or WIGGLY ROAD with an Acornucopia picnic hamper (the acorn cache) | The road and the hamper: friend test 3 (J2, J13, decided 31 Oct); the excavator and the ranger station: Demo |
| TINY (the dog) re-staged: chained in THE JUNKYARD, a chase out of it at the start that can't bite, then the ambush from COBBLE CURVE down THE DESCENT | The ambush stretch: friend test 3 (J12, decided 31 Oct); the rest: Demo |
| Golden acorns (the cargo re-themed; count may rise from 40) | Demo |
| Snow creeping in when behind par | Demo |
| Demo ending: the honk test (Dispatch asks the crew to honk, and answers the count), a teaser of the coming machines, "Follow on itch.io" | Demo |
| Chapters 2–6 (one day each), seamless travel between chapter maps, the sand pickup, school bus, crane truck and snowplow, the endings | Later |

## Later (after the demo)
Twitch chat integration (chat breaks a part or sends a pothole), hats and cosmetics from awards, more parts (the horn breaks, the steering wheel falls off, seat springs launch you), route variety, a kill-cam / instant replay, key rebinding. Order and notes: [Roadmap.md](Roadmap.md#tier-3-after-the-demo).

## Tech
- **Car:** the UE Vehicle template's offroad buggy on Chaos Vehicles (`ANBCar`), wearing the junk jeep body (`UNBCarBody`). Nobody possesses it: the server drives Chaos from the two driving seats' inputs, and clients follow its replicated physics state.
- **Seats** are attachment points on the car (`UNBSeatComponent`), so there's no free walking inside the car.
- **Repairs, the door and the flip-up** share one server-authoritative interaction component with four modes: Hold, Mash, TimingRing and Push2.
- **Online:** Steam OSS over SteamSockets, AppID 480 in dev. PIE falls back to IP.
- **Download budget:** ≤ 200 MB zipped for the demo, ≤ 300 MB for test builds ([Roadmap.md](Roadmap.md#package-size-slim-it-before-the-playtest)).
- The code map and gotchas are in `CLAUDE.md`.

## Tuning table
The values in the code today. None has been felt by hand yet; when a playtest confirms one, add "✓ <date>". Car and part values can be overridden on BP_OffroadCar_Pawn.

| Area | Values | Where |
|---|---|---|
| Run | par 540 s on the 5.4 km route (150 s on the test track), 2 points per second under it; a deadline until 31 Oct (540 s, 480 s on the old 3.5 km route); countdown 5 s; end screen 12 s | `ANBRoute::ParSeconds`, `ANBRunGameMode` |
| Score | 10 per acorn, 2 per second left, −25 per respawn, ×1.25 with Manual gears; 0 on a bad ending | `ANBRunGameMode` |
| Audio | engine loop asset volume 0.35 (✓ 14 Oct: the user found 1.0 too loud); music 0.35 | `SW_Engine_Loop` volume, `ANBHUD::MusicVolume` |
| Total wreck | 3 parts failed for 10 s | `ANBRunGameMode` |
| Failure director | storms and calms: a 25 s storm (timed failures at 0.7× the interval), then a calm with no timed failures (40 s early in the run, 20 s by the end; only parts worn past 85 % can break), alternating; first failure after 8 s; then every 14 s, easing to 6 s by par; at most 1, rising to players + 1, failed at once; a repaired part can't break again for 8 s; a repair that frees a slot restarts the interval; a failure unfixed for 25 s stops counting against the limit; at least 3 s between failures. Friend test 17 Oct: "too many breakdowns", and fixing one let the next in within 1.5 s (fixed) | `UNBFailureDirector` |
| Wear | pick weight 0.3 + 10 × wear² (nothing breaks by itself below 300 cm/s); rough ground 0.05/s per tyre (suspension rate 1→4 /s); crash: ≥ 600 cm/s horizontal hit wears the part on that side up to 0.7, ≥ 1200 breaks it (not within 4 s of its repair), 1 s between hits; past 0.6 wear, up to 8 %/s to break early. Per second: engine 0.03 × speed factor (1000→2000 cm/s) × (½ + ½ throttle); rear tires 0.02 × speed factor; front tires 0.035 × steer × (speed / 800 cm/s); brakes 0.08 × brake × (speed / 1500 cm/s), or 0.02 after 30 s above 600 cm/s without braking; door +0.15 per seat hop; all heal 0.006/s. In PIE, flat out (2250 cm/s) the engine reached 0.84 and broke at 40 s | `ANBCar` "Car\|Wear", `UNBFailureDirector` "Director\|Wear" |
| Engine | power fades to 35 % over 10 s, stalls at 16 s; repair: 14 presses (progress drains 0.2/s) | `UNBEnginePart` |
| Brakes | drop to 25 %, fading to 0 by 12 s; while failed they also seize (brake 0.8 at full torque for 0.35 s every 2.5–5 s above 300 cm/s: a lurch even if nobody brakes; PIE: −50 to −175 cm/s per grab); repair: 3 ring hits, 1.3 s period, 16 % sweet spot, a miss costs 34 % | `UNBBrakePart` |
| Tire | grip 30 %; pulls towards its side (front 0.3, rear 0.15 of full lock) with a ±0.08 wobble; the engine cuts out above 1400 cm/s (−300 per extra flat). PIE: a flat front-left at full throttle drifted 25° left in 9 s and topped out ~1500 cm/s; repair: hold 3 s, on foot | `UNBTirePart` |
| Door | throws a squirrel on its side out every 2 s above 250 cm/s (was 3 s / 300); with nobody on that side, 2 acorns fall out instead; repair: 3 presses | `UNBDoorPart` |
| Flip | counts as flipped when the up axis is below 0.35 and speed below 200 cm/s for 1.5 s; push: 2 squirrels for 2 s; lifts 120 cm | `ANBCar` |
| Acorns | 40; a velocity change above 350 cm/s per 0.1 s spills 1, plus 1 per 120 cm/s more; a flip dumps 25 %. Jumps: airborne ≥ 0.5 s and landing upright at ≥ 600 cm/s is a clean landing: only sideways jolts spill (during the flight and 0.3 s after), and it earns 25 air points per second airborne (score line "Air bonus") | `ANBCar` |
| Off the trail | grip of an off-trail wheel down to 0.45 after 5 s fully off (back in 2 s), tyres 0.05/s, engine 0.015/s, 20 %/s speed lost at full level | `ANBCar` "Car\|Offroad" |
| Crash feedback | from a 500 cm/s change per 0.1 s; full intensity at 1600 | `ANBCar` |
| Squirrel | ragdoll 2.5 s; respawn 8 s; jumping out above 600 cm/s throws you; eject kick 350 out + 450 up (cm/s) | `ANBSquirrel` |
| Camera | arm 320 cm on foot; seated 420 cm, pivot 110 cm above the seat, follow pitch -12° (650 / 150 until Luigi's review, 18 Oct: "unnecessarily distant" (G7); 1100 cm / 420 cm / level until friend test 1: the car was a speck at the bottom of the screen); swings behind the car after 1 s without look input, above 150 cm/s | `ANBSquirrel` |
| Handling | steering fades from full at 500 cm/s to 40 % at 1500 (the user, 18 Oct: bends were too easy at top speed; full lock at 1300 cm/s now turns 16° in 2 s, was 33°). Wheels sweep a sphere for the suspension (Spherecast), so trail rocks and ford pebbles jolt the car (with the template's ray they didn't; the user: "much better") | `ANBCar` "Car\|Handling", `TP_VehicleAdvOffroadWheelFront/Rear` |
| Gears | Manual gears: starts in 1st, W/S shift 1–5 (gear ratios 16.4 / 10.2 / 7.5 / 5.9 / 4.85, final 2.81, max torque 80 Nm, set by `Tools/Car/SetGearbox.py`; automatic up at 4700 rpm, down at 2600; redline 7000, idle 900, torque curve 0 from 5000 rpm; launch (PIE, 85 Nm) 0–500 cm/s 2.0 s, 0–1000 ~5 s, 5th at ~10 s, top ~1650 cm/s); braking still drops to reverse and back; over-rev wear from 65 % of the rev range up to 0.25/s at the redline (x2 in 5th, where the HUD says EASE OFF! instead of SHIFT UP!); score ×1.25. Handbrake: the Chaos rear-wheel handbrake, any mode (PIE: 2100 cm/s with a little steer swung the car 24° in 1 s) | `ANBCar` "Car\|Wear", BP_OffroadCar_Pawn transmission, `ANBRunGameMode::ManualGearsScoreScale` |
| Acorn in Mouth | every 75–105 s, 20 s muffle (450 Hz low-pass); pings: 1 s cooldown, shown 4 s | `ANBRunGameMode`, `ANBPlayerState`, `UNBVoiceTalker::MuffleFrequency` |
| Voice | full volume within 800 cm, silent 2500 cm further, measured from the listening squirrel (it was the camera, 11 m back); voices 150 %, mic boost 150 % by default, both in the Sound and voice menu | `ANBSquirrel` |
| Terrain | mountains 0–160 m, cliff benches 25 m (strength 0.45); trail grade ≤ 18 % (6 % THE PARK, 10 % hairpin and slalom, 3 % near jumps, 2 % start and end, level on the bridge); trail lumps ± 15 cm, lean ≤ 5 % | `ANBRoute` "Route\|Terrain" (rebake L_Route after changing) |
| Rivers | 14 m half width + 6 m banks, 2.5 m deep; fords 60 cm over the trail; bridge 9 m over its river; up to 300 m each way | `ANBRoute` "Route\|Terrain" |
| In the water | squirrel on foot swept away at ≥ 30 cm (600 cm/s, back in a seat after 2.2 s); car loses 50 %/s of its speed at 60 cm, every tyre wears 0.05/s on the hidden stones | `ANBSquirrel` "Falling", `ANBCar` "Car\|Wear" |
