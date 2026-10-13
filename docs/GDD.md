# Squirrel Wheels (working title) — Prototype GDD

Living doc, last updated 2026-10-12. Prototype: 20 days (Mon 5 – Sat 24 Oct 2026), free demo on itch. **Viewer fun first, player fun second.**

Schedule and ranked backlog: [Roadmap.md](Roadmap.md). Asset sources and licences: [Assets.md](Assets.md). Hands-on checks: [Playtest.md](Playtest.md).

**Status tags:** **Built** = in the game and verified by script in PIE, not yet felt by hand. **Friend test** = planned for Tue 20 Oct. **Demo** = for Sat 24 Oct, if the playtest agrees. **Later** = after the demo. Numbers like (#5) are Roadmap backlog items.

## Pitch
2–4 tiny squirrels drive one human-sized, open-top, falling-apart car from the park to the depot. One holds the WHEEL, one jumps on the PEDALS, and everyone else (plus anyone who bails from a seat) keeps the car alive.

## Core loop (6–10 min run, 8:00 deadline) — Built
- Both seats must be filled to drive. Squirrels hop between stations constantly.
- Breakdowns come faster as the run goes on. Each repair has its own readable action:

  | Part | Failure | Repair |
  |---|---|---|
  | Engine | overheats → power fades → stall | mash to cool (14 presses) |
  | Brakes | fade to nothing | timing-ring pump (3 hits) |
  | Tire ×4 | flat (grip loss) | jump out and hold for 3 s, clinging to the moving wheel |
  | Door | swings open, throws the passenger out | slam it shut (3 presses) |

- Crashes don't break parts yet; only the failure director's timer does. Crash damage was always meant to add breakdowns too (#16).
- Anyone who falls out ragdolls, gets up 2.5 s later, then chases the car or respawns at it after 8 s with a score penalty.
- Flipped car: 2 squirrels push at once to right it.
- **Score:** acorns still in the open bed at the depot (10 each), plus 2 per second left, minus 25 per respawn. The 40 acorns spill on bumps, hard braking and crashes; a flip dumps a quarter of them.
- **Bad endings:** the deadline runs out, or a total wreck (3+ parts failed for 10 s). Both score 0. Every run ends on the stats screen.

## Route — Built
The park-to-depot road is built in code (`ANBRoute`): about 3.5 km of greybox road in 11 named sections, each announced by a roadside sign and a HUD banner. The deadline is 8:00. Target: a tidy crew (~12 m/s average) gets there in ~5 min; a typical crew, with breakdowns, flips and falls, in 6-8 min.

| # | Section | Length | What happens |
|---|---|---|---|
| 1 | THE PARK | 400 m | Warm-up straight and a gentle S-bend. |
| 2 | COBBLE LANE | 280 m | Speed-bump ridges every 9 m, partly on a bend: acorns jump out. |
| 3 | ACORN HILL | 320 m | 8 m-high plateau with sheer sides and 7.6° ramps. Falling off the side flips the car. |
| 4 | THE HAIRPIN | 250 m | A 170° turn (~50 m radius): brakes and steering have to cooperate. |
| 5 | THE BRIDGE | 300 m | 8 m-wide raised deck over a "river", no rails. |
| 6 | ROCKY ROAD | 340 m | Scattered rocks on a bend and a straight: weave or bounce. |
| 7 | SQUIRREL SLALOM | 260 m | Pillars alternating left and right every 28 m. |
| 8 | THE BIG JUMP | 160 m | A 2.2 m kicker in the middle of the road; there's room to go round it. |
| 9 | WIGGLY ROAD | 440 m | Four 60° bends, one with bumps. |
| 10 | HOME STRETCH | 630 m | A long right-hander, then a fast rocky straight where the brakes matter. |
| 11 | THE DEPOT | 120 m | The drop-off gate, then the depot shed. |

**Open question:** 8:00 may be long for clips. A shorter demo route (#15) is decided after the 2-PC test.

## Events
- **Acorn in Mouth — Built.** Every 75–105 s while driving (never in the last 20 s), weighted toward the WHEEL seat and squirrels next to a failed part. That squirrel is muted for 20 s, still hears everyone, and keeps 8 pings on keys 1–8: LEFT!, RIGHT!, GAS!, BRAKE!, HELP!, FIX IT!, JUMP OUT!, NICE! Anyone can ping, muted or not.
- **One hazard with a face — Demo (#13):** a dog that chases the car on WIGGLY ROAD, or a sprinkler in THE PARK.
- **Bird steals a squirrel; pothole frenzy — Later (#22).**

## The squirrels
- **Built:** made from primitives in C++, with a unique fur colour per player. No free, rigged, CC0 squirrel exists, and a stiff, wobbly primitive ragdolls funnier than a realistic one.
- **Charm pass — Built (#7):** a head, big eyes with googly pupils (they sag under gravity, slosh with acceleration and bounce off the rim), cone ears, a cream belly, a pink nose, and a tail on a spring that lags behind acceleration and sways when idle. The head, belly and tail ride the tumbling body in a fall.
- **Voice — Built, slots empty (#4):** a scream when thrown out, a thud each time the tumbling body hits something hard (louder for harder hits), a chatter when grabbing a part. The slots are on `BP_Squirrel`.
- **Horn — Built, slot empty (#3):** any seated squirrel honks with H or gamepad Y (0.25 s cooldown). A "HONK!" pops over the car for 0.6 s, so viewers see it with the sound off.

## Readability and juice
Viewers can't hear game state, so every failure must read on screen in under 2 s with the sound off.
- **Built:** a pulsing failure list, "!" markers over broken parts, the timing ring with NICE / MISS, a "CAR FLIPPED!" banner, section banners, and sound/VFX slots on every car and part event (all still empty).
- **Built (#5, #6):** a big outlined callout per failure ("ENGINE ON FIRE!", "THE BRAKES ARE GOING!", "FLAT TIRE!", "THE DOOR FLEW OPEN!") that slams in, shakes and fades over 2.4 s, newest biggest, at most 3. A camera punch on crashes: full for riders, fading out to 25 m for squirrels on foot. An acorn pile on the engine deck, one sphere per acorn, that empties from the top as acorns spill.
- **Friend test:** sound in every slot (#2); a chunky imported font for the callouts (they use the engine's large font for now); VFX for engine fire, crash sparks and spills.
- **No slow-mo:** time dilation is global in multiplayer and would slow everyone's physics. A camera punch does the job instead.
- **Tuned for comedy** at the 2-PC test (#1): bigger ejection kicks, a wobblier door, more body roll.

## Onboarding — Built (#8)
- WHEEL ("steers") / PEDALS ("gas + brake") labels bob over the two driving seats until someone first sits in each (per map load).
- Four boards on the left of THE PARK, in the order they are needed: "E: GET IN / HOP SEAT" (in view from the start), "LMB: FIX BROKEN PARTS", "SPACE: JUMP OUT", "H: HONK!". Keyboard only.
- The controls hint stays on the HUD (Built).

## Co-op levers — Demo, if the playtest agrees
- **The pedal squirrel can't see the road (#9).** Its camera sits low in the footwell, so the WHEEL squirrel has to shout GAS / BRAKE. Asymmetric information forces talking, and talking is what makes streams funny. **Not decided:** it bends the locked chase-camera rule for one seat.
- **Pick spilled acorns back up (#11)** and throw them into the bed: "run back for them!" moments.
- **Fallen-squirrel cam (#12):** while tumbling or far behind, the camera keeps the car in frame.

## End screen
Built for screenshots.
- **Built:** the score breakdown; each squirrel's falls, fires put out, acorns spilled and drive time; MVP and "Most useless", picked by a helpfulness score (+3 per fire, +1 per 30 s driving, −1 per fall, −2 per respawn, −0.5 per acorn spilled). Not seen on screen yet.
- **Demo (#10):** one funny award per squirrel: Frequent Flyer (most falls), Butterfingers (most acorns spilled), Firefighter (most fires out), Backseat Driver (most pings), Longest Flight (farthest single throw).

## Presentation
- **Camera — Built:** third-person chase camera, pulled wide while seated; it swings behind the car while driving.
- **Art:** Kenney and Quaternius CC0 low-poly (flat colour) to dress the route and the depot. The Epic offroad buggy stays. It's M3a work and the first thing cut.
- **Lighting:** a sky light and sun with regular shadow maps. Lumen, ray tracing and virtual shadow maps go if the Mon 12 look check agrees: they cost download size and frame rate, and flat-colour art doesn't need bounce light.
- **Audio:** one cheap, rattly engine loop pitched by RPM (a lawnmower or moped beats a real car); impacts, clangs, fire crackle, repair dings, acorn pops. Music is CC0 / CC-BY with no Content ID, so streams stay unmuted.
- **UI:** a chunky rounded font (Lilita One or Fredoka) and Kenney input prompts.
- **Proximity voice — Built** (Steam VOIP): full volume within 8 m, silent about 25 m further.

## Online
- **Built:** Steam, listen server, up to 4 squirrels; host / invite / join / start / restart / leave from the lobby menu; AppID 480 in dev.
- **Open question:** AppID 480 for the public itch build ([Roadmap.md](Roadmap.md#decisions-waiting-on-you)).

## Later (after the demo)
Twitch chat integration (chat breaks a part or sends a pothole), hats and cosmetics from awards, more parts (the horn breaks, the steering wheel falls off, seat springs launch you), route variety, a kill-cam / instant replay, key rebinding. Order and notes: [Roadmap.md](Roadmap.md#tier-3-after-the-demo).

## Tech
- **Car:** the UE Vehicle template's offroad buggy on Chaos Vehicles (`ANBCar`). Nobody possesses it: the server drives Chaos from the two driving seats' inputs, and clients follow its replicated physics state.
- **Seats** are attachment points on the car (`UNBSeatComponent`), so there's no free walking inside the car.
- **Repairs, the door and the flip-up** share one server-authoritative interaction component with four modes: Hold, Mash, TimingRing and Push2.
- **Online:** Steam OSS over SteamSockets, AppID 480 in dev. PIE falls back to IP.
- **Download budget:** ≤ 200 MB zipped for the demo, ≤ 300 MB for test builds ([Roadmap.md](Roadmap.md#package-size-slim-it-before-the-playtest)).
- The code map and gotchas are in `CLAUDE.md`.

## Tuning table
The values in the code today. None has been felt by hand yet; when a playtest confirms one, add "✓ <date>". Car and part values can be overridden on BP_OffroadCar_Pawn.

| Area | Values | Where |
|---|---|---|
| Run | deadline 480 s (150 s on the test track); countdown 5 s; end screen 12 s | `ANBRoute::DeadlineSeconds`, `ANBRunGameMode` |
| Score | 10 per acorn, 2 per second left, −25 per respawn; 0 on a bad ending | `ANBRunGameMode` |
| Total wreck | 3 parts failed for 10 s | `ANBRunGameMode` |
| Failure director | first failure after 8 s; then every 14 s, easing to 6 s by the deadline; at most 1, rising to players + 1, failed at once; a repaired part can't break again for 8 s | `UNBFailureDirector` |
| Engine | power fades to 35 % over 10 s, stalls at 16 s; repair: 14 presses (progress drains 0.2/s) | `UNBEnginePart` |
| Brakes | drop to 25 %, fading to 0 by 12 s; repair: 3 ring hits, 1.3 s period, 16 % sweet spot, a miss costs 34 % | `UNBBrakePart` |
| Tire | grip 30 %; repair: hold 3 s, on foot | `UNBTirePart` |
| Door | throws the passenger out every 3 s above 300 cm/s; repair: 3 presses | `UNBDoorPart` |
| Flip | counts as flipped when the up axis is below 0.35 and speed below 200 cm/s for 1.5 s; push: 2 squirrels for 2 s; lifts 120 cm | `ANBCar` |
| Acorns | 40; a velocity change above 350 cm/s per 0.1 s spills 1, plus 1 per 120 cm/s more; a flip dumps 25 % | `ANBCar` |
| Crash feedback | from a 500 cm/s change per 0.1 s; full intensity at 1600 | `ANBCar` |
| Squirrel | ragdoll 2.5 s; respawn 8 s; jumping out above 600 cm/s throws you; eject kick 350 out + 450 up (cm/s) | `ANBSquirrel` |
| Camera | arm 320 cm on foot, 1100 cm seated; swings behind the car after 1 s without look input, above 150 cm/s | `ANBSquirrel` |
| Acorn in Mouth | every 75–105 s, 20 s mute; pings: 1 s cooldown, shown 4 s | `ANBRunGameMode`, `ANBPlayerState` |
| Voice | full volume within 800 cm, silent 2500 cm further | `ANBSquirrel` |
