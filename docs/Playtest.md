# Playtest checklist

Everything here has been verified only by Python scripts in PIE, so this is the first time anyone feels it. Use it for the M2 gate (Mon 12 – Tue 13 Oct), the smoke test before the friend test (Mon 19), and the friend test itself (Tue 20). Tick what works. For what doesn't, write it down with the build's commit.

## Setup (Claude)
- [x] Package the slim Development build: phases A and B of [Package size](Roadmap.md#package-size-slim-it-before-the-playtest). The command is in CLAUDE.md under Config gotchas → Packaging. Archive to `D:\Games-Unreal\ProjectI_Builds\<date>\`, with the PDB in `Symbols\`, outside the zip.
- [x] Note the commit it was built from, the zip size and the installed size. The target is ≤ 300 MB zipped.

  | Date | Commit | Zip | Installed | Notes |
  |---|---|---|---|---|
  | 2026-10-06 | (pre-M1) | 1.04 GB | 3 GB | Included the 2.3 GB PDB. |
  | 2026-10-13 | `a950c06` | 254 MB | 495 MB | Phases A + B. `ProjectI-2026-10-13-Dev.zip`. The exe (302 MB) is now the biggest file; content is 129 MB. Launches offline with no load errors. |
  | 2026-10-17 | `dc7593b` | 258 MB | 498 MB | `ProjectI-2026-10-17-Dev.zip`. First build with Tier 1 (horn, pile, callouts, charm, onboarding), sounds + music, puff VFX, the squirrel/acorn models, route props and input prompts. Content 136 MB. Launches offline with no asset-load errors; Lilita One loads. Played in friend test 1 (results below). |
  | 2026-10-17 (fix) | `b20dce9` | 258 MB | 498 MB | `ProjectI-2026-10-17-fix-Dev.zip` in `ProjectI_Builds\2026-10-17-fix\`. Fixes the restart crash: two packaged copies on one PC (`?AutoStart`, `-nosteam`, IP) went through 2 restarts with the client connected; the 17 Oct build crashed the client on the first. Adds part wear and the director fixes. Not played by hand. |
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
- [ ] Horn: H (or pad Y) in any seat shows "HONK!" over the car on both PCs. Fun to spam, or annoying? (Not tested by script: Python can't press keys.)
- [ ] Crash camera punch: noticeable on a hard hit without being sickening. Tune `CameraPunchDistance` / `CameraPunchRoll` on BP_Squirrel.
- [ ] Sound (all CC0 placeholders, 14 Oct): engine loop pitch, crash, spill pops, part breaks, the fire loop on a broken engine, repair ding, flip thud, righted boing, bicycle horn, squirrel scream / landing thud / grab chatter, menu and run music, countdown beeps, end jingles. Which ones are wrong or annoying? The run music is a 32 s loop: does it grate over 8 minutes?
- [ ] Input prompts: the bottom hint and the repair panel show key icons, and switch to Xbox buttons the moment a pad is touched (untested: no pad in the PIE checks), then back on a key press.
- [ ] Art (14 Oct): the low-poly squirrel reads from the chase cam and in each fur colour; acorns in the pile and when spilled; the route props (trees, fences, lamps, cones). Packaging check: the imported glTF materials depend on the Interchange plugin's `/InterchangeAssets/gltf` master materials, so confirm they cook and render in the package.
- [ ] VFX (code-built puffs, 14 Oct): engine fire + smoke, crash sparks, spill puff, flip dust, righted and repair stars, tire smoke, brake sparks, landing dust. Readable from the chase cam? Too much or too little? Tune "Puffs" on BP_OffroadCar_Pawn / its parts / BP_Squirrel.
- [ ] Charm pass: does the squirrel read as a squirrel from the chase cam? Do the googly pupils and tail wobble look lively, or jittery on the client (they run off replicated motion)? Tune "Squirrel|Charm" on BP_Squirrel.
- [ ] Onboarding: the WHEEL / PEDALS labels show over the empty driving seats and vanish once each is used; the PARK boards are readable from the car. Did a new player find E without being told?

## Repairs and failures
- [ ] The HUD shows the failure list, the "!" markers over broken parts, prompts, the progress bar, the push count, and the timing ring with NICE / MISS.
- [ ] Engine (mash 14 times), brakes (3 timing-ring hits), tire (hold 3 s outside the car, clinging while it drags you), door (slam: 3 presses). Is each one readable, and fun?
- [ ] An open door throws the passenger out every 3 s above 300 cm/s.
- [ ] Each break shows its big callout ("ENGINE ON FIRE!"...). Readable at a glance with the sound off? Does it cover anything you needed to see? The callouts are in Lilita One now: check the size and outline still look right (the font face is new, untested in a build).
- [ ] Failure rate (one every 14 s, ramping to one every 6 s): too calm, or too busy? Do failures feel random (Roadmap #16)?
- [ ] Total wreck: 3 parts failed for 10 s ends the run.

## Falls, flips and acorns
- [ ] Getting thrown out looks funny, not broken. You get up after 2.5 s, and respawn at the car after 8 s if you haven't got back in.
- [ ] Flip (`NBFlip` in the host's console): two squirrels pushing at once right the car.
- [ ] Spills: COBBLE LANE's bumps spill a few acorns, not all of them. A flip dumps a quarter.
- [ ] The acorn pile on the engine deck shrinks with each spill and can be read from the chase cam. Does the deck squirrel sitting in it look OK?
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

## Known problems in the 17 Oct build (tell testers first)
For the testers of 19–25 Oct, who play `2026-10-17-fix` (the restart crash, director and wear fixes; nothing from F1 on). All of these are fixed or changed in `main`; ask them for everything else (fun, confusion not on this list, Acorn in Mouth, run length).
1. Voice is quiet and there are no settings: use Discord if you can't hear each other.
2. No "YOU" marker: you are the squirrel in your fur colour. The WHEEL seat steers (A/D), the PEDALS seat does gas and brake (W/S): two different people.
3. The WHEEL and PEDALS labels can overlap at the start.
4. The seated camera is far back; the car looks small.
5. Some failures barely matter (flat tyre, open door, failed brakes), and parts can break while standing still or for no visible reason.
6. THE BIG JUMP costs acorns even on a clean landing.
7. Trees, rocks and fences have no collision.
8. Being thrown out isn't explained: usually the broken door, or jumping out at speed. 8 s to get back in, or respawn at −25.
9. The repair prompt may show for squirrels who aren't repairing.
10. The route is flat (the mountain trail comes next).
11. Engine repair: mash left-click on the engine deck; nothing on screen says so.

## Friend test 1 results (Sat 17 Oct, build `dc7593b`)
3 players over Steam (monty500500, Anya_Musk, bongo24) plus the host, 2 runs. Sources: 3 form answers, the notes doc, the host's chat with them, 3 recordings (Drive) whose Hindi/English voice chat was machine-translated with Whisper, so quotes are approximate.
- **Runs:** run 1 delivered at 3:50 left (30/40 acorns, score 764); run 2 a total wreck at ~1:30 left. Run 2's restart **crashed every client** (the recording shows the crash toast 12 s into the end screen).
- **Scores:** fun 4, 4, 3. Play again: "yes, right now", "maybe another day", "no". Would watch a streamer: yes, yes, no. Acorn in Mouth: funny, funny, didn't notice. Run length: **too short ×3**. First minute: "took a bit" ×3.
- **Best repair:** door, brakes. **Worst: engine mash ×3** ("I have to press it multiple times, I didn't understand"; "not clear at the start, maybe a UI element"; the notes ask for another key).
- **Funniest:** multiple fires while friends got thrown out; the car falling off the ramp; nobody steering, everyone yelling "WE BALL!" and pushing through a wrecked car.
- **Readability:** nobody knew who was steering or which squirrel was theirs ("am I even controlling it?", "I thought I was blue, I'm pink"). The form asks for feedback on roles.
- **Failures:** too many, too constant ("errors pop up way too often, I'd prefer a cruising phase"). Fixing one let the next in at once, and leaving one broken held the rest off (a director bug, fixed 17 Oct). Flat tires and the door didn't matter ("what difference does a flat tyre make", "does the door really matter? No"), and brake failure meant nothing because nobody brakes.
- **THE BIG JUMP** only costs acorns on landing, so why jump.
- **Voice:** too quiet, so everyone moved to Discord. They want mic/speaker device and volume settings.
- **Camera:** "not very good". The seated camera is 11 m back, so the car is a speck at the bottom of the screen.
- **Bugs:** restart crash (above, fixed 17 Oct); "widgets are shared across players" (probably the repair prompt showing on squirrels who aren't repairing; unconfirmed); props have no collision (the car drives through trees; one player said colliders might make it annoying); the WHEEL and PEDALS labels draw on top of each other.
- **Ideas:** a damage meter; manual gears (and a Gears seat); terrain instead of a flat slab; branching sections; side tasks / vehicle swaps (R.V. There Yet?); distractions like roadside ads; a Linux build.
- **Name ideas:** Nuts and Bolts, Roll Deez Nuts, Rodent Racing.
- GPUs: RTX 3060, 3070 Ti, 4070 laptop. No frame-rate complaints.

## Friend test 2 answers (18 Oct, the package sent on 17 Oct)
2 form answers (Luigi, Lunalovegroot; Luna played Pedals with "Tutfrem"). The package predates F1–F9, so voice settings, the who-is-who HUD and prop collision weren't in it.
- **Scores:** fun **1** and 3. Play again: "no", "maybe another day". Would watch a streamer: no, maybe. Acorn in Mouth: annoying, didn't notice. Run length: about right, too short. First minute: "took a bit" ×2.
- **Best repair:** brakes, engine. **Worst:** tire, brakes.
- **Funniest:** reversing and working with the partner to turn the right way (the Wheel/Pedals split works); driving most of the course fine, then the car falling apart on the last stretch.
- **Failures feel random, not earned (both):** "random damage is super annoying, it should be a punishment for hitting stuff instead"; "tires popping for no reason … better if we had to avoid the cobbles … rather than popping while stationary". The director has no speed gate (parts break while the car stands still) and crashes add no wear.
- **Ejections unexplained:** "no idea why I get thrown out of the car, I don't see the point in needing to return the car quickly" (probably the "Get back in the car! 8" countdown; the door and the bail never say why).
- **Voice again (both):** game sound drowned the teammate, one moved to Discord; "could not find a way to change these settings". F1's Sound and voice page wasn't in this build, but they also didn't know where to look.
- **Roles:** "difficult to understand which position steers"; Luna tried to steer from the Pedals seat and her partner found steering by chance.
- **Bugs:** no collision on most of the environment (F7 since).
- **Name ideas:** Squarrel (squirrel + quarrel), Go Nuts.
- GPUs: RTX 3070 Ti, GTX 1080 Ti.

### Luigi's extra review (Discord, 18 Oct, same 17 Oct package)
Written after the form above, so it's about the flat route before F1–F9 and the terrain.
- **"Right now, the game is incredibly easy."** Wants a much narrower track, a real risk of falling from heights, much harder obstacles to dodge, and moving ones (swinging wrecking balls). "Ramp up the difficulty about 20x."
- **One job each:** suggested P1 accelerate, P2 brake, P3 steer, P4 repair. (The user's call: the pedals stay one seat; the Gears seat gets a handbrake instead.)
- **Camera:** first person, or a much closer third person; "it feels unnecessarily distant".
- **Repairs:** find a way to do every repair from inside the car.
- **"RV There Yet?"** has a near perfect framework for this genre; take more from it (not a copy).
- **Setting:** nothing ties the game to the players being squirrels. Suggests squirrels getting home over dangerous tree branches and rivers, using the environment, with reasons to get out of the vehicle, e.g. finding spare parts for a makeshift (Flintstones-style) car.
- Where each point went: [Roadmap.md](Roadmap.md#luigis-review-18-oct-where-it-went).

## Next test: check the friend-test fixes
- [ ] The restart works on Steam: the end screen, then the next run, with every client still in and in voice.
- [ ] Voice is loud enough without Discord. Esc → Options → Sound: the Voices and Mic boost sliders change it live, the Speakers / headphones list switches device, and the values are still there after a relaunch.
- [ ] Everyone can tell which squirrel is theirs (YOU arrow) and who is on the Wheel and Pedals (top left, name tags).
- [ ] The seated camera (6.5 m, tilting down while following): the car readable, the road ahead visible? Tune `SeatedArmLength` / `SeatedCameraHeight` / `SeatedFollowPitch` on BP_Squirrel.
- [ ] Flat tires pull and cap speed, the door throws people or loses acorns, failed brakes lurch, the engine shows MASH LMB! / N MORE!. Do they matter now, without being unfair?
- [ ] The damage meter (top right, CAR %): do players read it and fix things before they break?
- [ ] THE BIG JUMP: a clean landing keeps the acorns and shows AIR TIME! +N; the end screen lists the Air bonus.
- [ ] The 5.4 km route (L_Route): how long did a run take against 9:00? Too long now?
- [ ] Trees, rocks, fences and lamps are solid: fun obstacles, or annoying?
- [ ] Calm and storm: does the quiet stretch feel like a breather, and the storm like a storm?
- [ ] Failures: does fixing one buy a breather now? Does driving flat out break the engine more, and steering hard the front tires? Note the director log lines (`NBDirector: ...`) from the host's log.

## Friend test 2 fixes (18 Oct): check by hand
- [ ] Damage feels earned: hitting a tree or cliff breaks the part on that side (`NBCrash:` in the host log), rocky stretches wear the tyres, and nothing breaks while parked. Too harsh, or still random?
- [ ] Falling out says why (THE DOOR FLUNG YOU OUT! etc.) and the countdown explains the −25 respawn.
- [ ] A/D on the Pedals (or W/S on the Wheel) shows who does that job.
- [ ] "Can't hear your crew? Esc: Sound and voice" shows before the run; voices stay audible with the Game slider low. Python couldn't look at the HUD, so none of these lines has been seen yet.

## Options panel (18 Oct): check in a packaged build
- [ ] Esc → Options: the Sound, Controls and Display tabs switch, and a gamepad can reach every control.
- [ ] Controls: look sensitivity and invert apply at once (mouse and right stick); crash shake at 0 removes the camera punch.
- [ ] Keys: click Horn, press J: J honks and H doesn't; bind Horn to E and Get in takes H (swap); Reset keys brings back the defaults; the bottom hints and the WHEEL / PEDALS lines show the new keys; the bindings survive a relaunch.
- [ ] Display: window mode, resolution, quality, frame cap and VSync apply at once and survive a relaunch. The editor only shows placeholder values here.

## Cones, markers, off the trail (18 Oct): check by hand
- [ ] Traffic cones fly when the car hits them, without slowing it; a squirrel can kick them too. In a 2-PC run each player's cones may lie differently (they aren't replicated): does anyone notice?
- [ ] The red corner posts and sign boards are solid now: fair obstacles, or do they catch the car on tight corners?
- [ ] Off the trail: a short cut across the grass is fine, a long one gets slippery ("OFF THE TRAIL!  grip -N%"), wears the tyres and slows the car. Too harsh or too soft? (`OffroadMinGrip`, `OffroadRampSeconds`, `OffroadDrag` on BP_OffroadCar_Pawn.)

## Car reset (18 Oct): check by hand
- [ ] Drive off a cliff or into a gully: after ~3 s stuck, "STUCK?  Hold R" shows; holding R fills the bar and puts the car back on the trail 8 m before where it went off (not on a lower leg of the hairpin below), upright and facing the right way, with riders aboard and anyone on foot back at the car (no respawn penalty). "CAR RESET!  -3 acorns" shows.
- [ ] A second reset within 15 s says "Reset ready in N s". Is 3 acorns + 15 s the right price, or do players use it as a shortcut?
- [ ] Gamepad View works; rebinding Reset in Options works.

## Mountain trail (18 Oct): check by hand
Only an autopilot has driven it so far (single-player PIE, no breakdowns, 16 m/s: 7:52 against the 9:00 deadline, no stuck points, 17 acorns lost).
- [ ] Is 9:00 still enough with breakdowns on the mountain trail? The autopilot alone needs 7:52.
- [ ] The look (no-Lumen build): do the mountains, cliffs and river read? Does the dirt trail stand out from the grass?
- [ ] 18 % climbs: fun or a slog? With a broken engine (35 % power, then a stall), is a climb a funny crisis or a dead stop? (`MaxGrade` on the route, then a rebake.)
- [ ] Descents with failed brakes: scary-fun, or the brake grab flips the car?
- [ ] The fords: do the wheels and body sit in the water? Does a squirrel that jumps out get swept away visibly, and is it back in a seat quickly enough? Do the hidden pebbles wear the tyres enough to matter (a flat mid-river)?
- [ ] THE BRIDGE: is the drop to the river visible, and is falling off funny or frustrating?
- [ ] ROCKY ROAD: the rocks on the steep climb tip the car up to 30°. Too much?
- [ ] Are the cliffs beside the trail obstacles (the car stops dead) or do they just look good?
- [ ] Hairpin and wiggles: no odd ridges between the two legs.
- [ ] Client view in a 2-PC run: the car sits on the ground and in the water, same as on the host.
- [x] Zip size of the first build with the landscape: 265 MB (`2026-10-18-terrain`, was 258 MB).

## Friend test (moved up to the weekend of 17–18 Oct)
- Host run sheet: https://claude.ai/artifact/HhHUPpK2G5D7fsctMVJHRY (ticks and notes, then a summary to paste back to Claude).
- Friends' questionnaire: a Google Form made by `Tools/Playtest/FeedbackForm.gs` (run it once at script.google.com; answers land in a Google Sheet). Fallback: https://claude.ai/artifact/Gv6LbW984RDA3HLJGRddvF (same questions; copy-to-Discord for anyone who can't save there).
- Friends play the private itch build and record themselves (OBS/Discord), or you record them on Discord.
- Afterwards, 3 questions: *Play again with this group? / Funniest moment? / Most annoying thing?*
- **Go signal for full production:** most groups want another run, and the recordings have clips that are funny with the sound off.
- Grab a 15–20 s clip of a squirrel thrown out of the car for the itch page GIF.
- Watch the recordings muted. Any failure that doesn't read in under 2 s gets stronger VFX/SFX before the release.
