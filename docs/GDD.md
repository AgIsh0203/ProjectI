# Squirrel Wheels (working title) — Prototype GDD

Living doc. Prototype: 20 days (Mon 5 – Sat 24 Oct 2026), free demo on itch. **Viewer fun first, player fun second.**

## Pitch
2–4 tiny squirrels drive one human-sized, open-top, falling-apart car from the park to the depot. One holds the WHEEL, one jumps on the PEDALS, and everyone else (plus anyone who bails from a seat) keeps the car alive.

## Core loop (6–10 min run)
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
