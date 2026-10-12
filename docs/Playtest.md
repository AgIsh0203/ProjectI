# Playtest checklist

Everything here has been verified only by Python scripts in PIE, so this is the first time anyone feels it. Use it for the M2 gate (Mon 12 – Tue 13 Oct), the smoke test before the friend test (Mon 19), and the friend test itself (Tue 20). Tick what works. For what doesn't, write it down with the build's commit.

## Setup (Claude)
- [ ] Package the slim Development build: phases A and B of [Package size](Roadmap.md#package-size-slim-it-before-the-playtest). The only build so far (6 Oct) predates M1 and M2 and is a 1 GB download. The command is in CLAUDE.md under Config gotchas → Packaging. Archive to `D:\Games-Unreal\ProjectI_Builds\<date>\`, with the PDB in `Symbols\`, outside the zip.
- [ ] Note the commit it was built from, the zip size and the installed size. The target is ≤ 300 MB zipped.
- [ ] Copy the zip to the second PC, the way a friend would get it. Steam runs on both, signed into two different accounts.

## Slim build
- [ ] It starts on both PCs with no missing-DLL or missing-asset errors (plugins were disabled and the cook list narrowed).
- [ ] Without Lumen, does the lighting look acceptable? Note the frame rate on the weaker PC (`stat fps` in the console).

## Lobby and session
Nobody has used the lobby menu by hand yet. Restart can only be tested in a packaged build, because PIE refuses seamless travel.
- [ ] The menu opens by itself; "Practice alone" starts a run offline.
- [ ] "Host a game": the top-left line says HOST, with the Steam subsystem.
- [ ] "Invite friends" opens the Steam overlay, and accepting the invite joins.
- [ ] "Join Game" from the Steam friends list joins.
- [ ] "Join a game" (search) finds the lobby.
- [ ] "Start the run" works only for the host, and only with 2+ squirrels.
- [ ] "Restart the run": the client stays connected and in voice.
- [ ] "Leave the game": the other side sees a status line, and you're back offline.

## Voice
- [ ] Proximity voice: full volume within ~8 m, fading to silence about 25 m further.
- [ ] Acorn in Mouth (every 75–105 s while driving, for 20 s): the muted squirrel's mic really cuts out, it still hears everyone, and its banner and ping legend show.
- [ ] Keys 1–8 send pings, and they show on the other PC for 4 s.

## Seats, camera and driving
- [ ] Seat positions look right on both PCs. E hops to the next free seat; Space jumps out on that seat's side.
- [ ] Jumping out above ~22 km/h (600 cm/s) throws you instead.
- [ ] The pedal squirrel's tail flag shows over the dash.
- [ ] The seated camera swings behind the car after 1 s without look input.
- [ ] Driving feels arcade, not sluggish. Any tire shimmer (TSR) left?

## Repairs and failures
- [ ] The HUD shows the failure list, the "!" markers over broken parts, prompts, the progress bar, the push count, and the timing ring with NICE / MISS.
- [ ] Engine (mash 14 times), brakes (3 timing-ring hits), tire (hold 3 s outside the car, clinging while it drags you), door (slam: 3 presses). Is each one readable, and fun?
- [ ] An open door throws the passenger out every 3 s above 300 cm/s.
- [ ] Failure rate (one every 14 s, ramping to one every 6 s): too calm, or too busy? Do failures feel random (Roadmap #16)?
- [ ] Total wreck: 3 parts failed for 10 s ends the run.

## Falls, flips and acorns
- [ ] Getting thrown out looks funny, not broken. You get up after 2.5 s, and respawn at the car after 8 s if you haven't got back in.
- [ ] Flip (`NBFlip` in the host's console): two squirrels pushing at once right the car.
- [ ] Spills: COBBLE LANE's bumps spill a few acorns, not all of them. A flip dumps a quarter.
- [ ] The drop-off gate triggers when the car is inside it.

## Run and end screen
- [ ] The countdown, section banners, "depot in X km" and the clock all show.
- [ ] How long did the run take against the 8:00 deadline? Did it drag (Roadmap #15)?
- [ ] The end screen shows the score breakdown, the stats, MVP and Most useless.

## Write down
- **The five things that felt worst.** This list drives the next block of work.
- The funniest moment. Clip it if you recorded.
- Run time, acorns delivered, and how the run ended.
- Any value that felt wrong, for the [GDD tuning table](GDD.md#tuning-table).
- If something broke: each PC's log, in the build folder under `ProjectI/Saved/Logs/ProjectI.log`.

## Friend test (Tue 20 Oct)
- Friends play the private itch build and record themselves (OBS/Discord), or you record them on Discord.
- Afterwards, 3 questions: *Play again with this group? / Funniest moment? / Most annoying thing?*
- **Go signal for full production:** most groups want another run, and the recordings have clips that are funny with the sound off.
- Grab a 15–20 s clip of a squirrel thrown out of the car for the itch page GIF.
- Watch the recordings muted. Any failure that doesn't read in under 2 s gets stronger VFX/SFX before the release.
