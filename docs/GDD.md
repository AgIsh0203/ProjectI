# Squirrel Wheels (working title) — Prototype GDD

Living doc. Prototype: 20 days (Mon 5 – Sat 24 Oct 2026), free demo on itch. **Viewer fun first, player fun second.**

## Pitch
2–4 tiny squirrels drive one human-sized, open-top, falling-apart car from the park to the depot. One holds the WHEEL, one jumps on the PEDALS, and everyone else (plus anyone who bails from a seat) keeps the car alive.

## Core loop (6–10 min run, 8:00 deadline)
- Both seats must be filled to drive. Squirrels hop between stations constantly.
- Breakdowns escalate with time and crash damage. Each repair has its own readable action:

  | Part | Failure | Repair |
  |---|---|---|
  | Engine | overheats → power loss → stall | mash to cool |
  | Brakes | fade | timing-ring pump |
  | Tire | flat (grip loss) | jump out, hold while shaking |
  | Door | swings open, squirrels fall out | slam shut |

- Anyone who falls out ragdolls, then chases the car or respawns at it after 8 s with a score penalty.
- Flipped car: 2 squirrels push at once to right it.
- **Score:** acorns still in the open bed at the depot (they spill on bumps and hard braking), plus a time bonus.
- **Bad endings:** the deadline runs out, or a total wreck (3+ parts failed for 10 s). Every run ends on the stats screen.

## Route
The park-to-depot road is built in code (`ANBRoute`): about 3.5 km of greybox road in named sections, each announced by a roadside sign and a HUD banner. The deadline is 8:00. Target: a tidy crew (~12 m/s average) gets there in ~5 min; a typical crew, with breakdowns, flips and falls, in 6-8 min.

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

## Events
- **Acorn in Mouth:** every 75–105 s, weighted toward the WHEEL seat or a squirrel at a failing part. That squirrel is muted for 20 s, can still hear everyone, and gets an 8-ping chat wheel.
- Pothole frenzy. Bird steals a squirrel (stretch).

## Presentation
- Third-person chase camera, pulled wide while seated.
- Kenney/CC0 placeholder art.
- Proximity voice (Steam VOIP).
- End screen built for screenshots: falls, fires, acorns spilled, MVP and "Most useless" badges.

## Tech
- Arcade raycast car (`ANBCar`). The server owns the inputs; every machine runs the same force model, and physics replication corrects drift.
- Seats are socket attachments: no free walking inside the car.
- Steam OSS (AppID 480 in dev), with Null/IP for PIE.

## Tuning table
Car tuning lives on `ANBCar` (Car|Tuning). Values that feel right get recorded here.

| Param | Value | Notes |
|---|---|---|
| MaxSpeed | 2600 cm/s | ~94 km/h |
| EngineForce | 900000 | |
| LateralGrip | 9 | lower = driftier |
| SpringStrength / Damping | 26000 / 3200 | |
