# Story: the golden acorn heist

Decided 29 Oct 2026 in a design session with the user. **Nothing here is built yet.** Friend test 3 (Sat 28 Nov) tests the game as it is today. If that test passes, Chapter 1 below becomes the public demo, and the full game is designed around this story. The build order is in [Roadmap.md](Roadmap.md#story-demo-after-friend-test-3). Gameplay detail lives in [GDD.md](GDD.md). The script sample for the voice actor is in [VA_Sample.md](VA_Sample.md).

## Premise
The squirrels are sick of plain old acorns.
1. A gourmet delivery truck goes by with crates of **golden acorns** (premium, glazed). The crew sneaks aboard to steal some, but the truck pulls away and carries them hundreds of kilometres from home.
2. The truck crashes at the edge of a **junkyard** (the driver swerves for a deer) and the crates spill out.
3. The crew can't move a wrecked truck, so they **build the junk jeep** from a toy jeep and junkyard trash. That's why the engine is a car battery, the brake is a sponge and the door is cardboard: the jeep built in A2 already looks like this.
4. They load the golden acorns, their winter stash, and race to get **home before the first snow**.

Naming: the cargo is always **acorns** (golden acorns), never "nuts". "Acorn in Mouth" keeps its name. The acorn count may go up from 40; tune it in playtests.

## The clock and the endings
- **No deadline. A stopwatch** runs over the whole journey and carries across chapters.
- Each chapter has **par times**, giving a medal: **Golden Acorn** / **Acorn** / **Late**.
- **Score** = acorns delivered × 10 + a bonus for beating par. Respawns still cost points.
- **Winter creeps in when you're slow.** When the crew is behind par, the world gets snowier: flurries, frosted props, a nervous radio. It's only cosmetic.
- **Ending cut** at the home oak in the home park. Its tone depends on the total time: "the first snowflake lands as the last acorn goes in" when on time, or digging the tree out of a snowdrift when late. With 0 acorns left the ending is a sadder (and funnier) one.
- **The only way to lose a run is a total wreck**, and even that only rewinds to the last checkpoint (below).

## Structure: chapters that feel like one trip
- 6 chapters that should feel like one continuous trip, the way RV There Yet does. A chapter takes ~6–9 min.
- **One map per chapter**, joined at a **camp** (a rest stop with a gate or tunnel) by **seamless travel**: clients stay connected, the stopwatch keeps running, and there's no menu in between. This reuses the seamless-travel restart that already exists.
- A camp is a short breather: the stats so far, a radio line, then drive on.

## Saving and checkpoints (RV There Yet style)
RV There Yet has no manual save: it autosaves at checkpoints (gas stations, new regions), and dying loses everything since the last one. Ours works the same way, more gently:
- **Automatic checkpoints** at each camp and at the start of each named section. A "CHECKPOINT" toast and a radio line show when one saves.
- **A total wreck rewinds to the last checkpoint:** the car, the acorns and the parts come back as they were there, and the stopwatch gets **+30 s**.
- **Fallen squirrels** respawn at the car automatically, as today. There's no revive item: with only 2 players, a squirrel who needs a revive could be stuck.
- **Quit and continue:** the host can quit, then **Continue** from the last checkpoint with any crew.
- **A camp only saves when the jeep AND every squirrel are inside it.** Until then the HUD shows "WAITING FOR <name>". Squirrels left behind are **not** teleported to the camp; they have to get there themselves (they still respawn at the jeep if they fall).

## Forks that merge back
Each chapter (except the last) has **one fork in the road** the crew chooses, and the two roads meet again further on:
- The **fast branch** needs a helper vehicle.
- The **long branch** is longer and rougher, with an **acorn cache** to refill from.

Acorns carry over the whole journey and spills are permanent, so each fork is a choice between speed and acorns. Splitting up is allowed: one squirrel fetches the helper vehicle while the others raid the cache.

## Helper vehicles
The junk jeep is the hero. Helper vehicles solve one obstacle each.

| Vehicle | Job | Seats (controls split, as in the jeep) |
|---|---|---|
| **Excavator** | smash through a blocker (a building) | Tracks (drive and turn) / Boom (arm up/down and swing) / Bucket (curl and smash) |
| **Sand pickup** | fill a sinkhole or build a ramp | Wheel / Pedals / Tipper |
| **School bus** | park it across a gap so the jeep drives over it | Wheel / Pedals / Doors? |
| **Crane / tow truck** | lift or haul the jeep over or out of something | Drive / Boom / Winch |
| **Snowplow** | the finale: clear the road through the first snow | Wheel / Pedals / Blade |

Rules (seats other than the excavator's are placeholders):
- **The swap:** park the jeep at a marked bay (a parked jeep doesn't wear or spill), walk or hop to the helper, do the job, walk back. The stopwatch keeps running throughout.
- **Splitting the crew is allowed.** A vehicle works with whoever is in it. A lone squirrel can work one by hopping between its seats, slowly and badly.
- **With 2 players** on the excavator, one works Tracks and the other works Boom and Bucket. Riders fix parts.
- **Helpers break too:** 1–2 parts each on the existing part system (the excavator's hydraulic hose bursts, for example), so the repair juggling continues.
- **How many:** Ch1–2 have one helper each. From Ch3 there are two, used one after the other. The finale has one moment where two are used **at the same time** (the crane holds the bus while the jeep drives over it); a placed vehicle stays put, so 2 players can still do it.

## Chapters
| # | Chapter | Fork: fast branch / long branch |
|---|---|---|
| 1 | **The Crash Site** *(the demo)* | Today's L_Route mountain trail. THE PARK is re-themed as the crash site and junkyard, and its control boards stay as the tutorial. Fast: the **excavator** smashes through a ruined ranger station. Long: the road round it, rougher, with an acorn cache. |
| 2 | **The Washout** | A storm has hit. Fast: the **sand pickup** fills a sinkhole. Long: ford the flooded river (rivers are already built). |
| 3 | **The Old Quarry** | Fast: build a sand ramp, then park the **bus** across the pit as a bridge. Long: the quarry switchbacks down and up again. |
| 4 | **Town Outskirts** | Traffic, and the dog's home turf. Fast: the **tow truck** hauls the jeep over the rail line. Long: wait at the train crossing, or gamble on beating the train. |
| 5 | **The Dam** | Fast: the **crane** holds the **bus** across the spillway (the moment where both are used at once). Long: the service tunnel. |
| 6 | **First Snow** | No fork. The **snowplow** leads and the jeep follows in its wake, racing to the home oak in the home park. Then the ending cut. |

Only Chapter 1 is being built for the demo; Ch2–6 can still change.

Today's hazards stay in Chapter 1: the dog on WIGGLY ROAD, the roadblocks and the wrecking ball on THE BRIDGE.

## The demo: Chapter 1 only
It ends at the Ch1 camp:
1. A radio cliffhanger line ("...Dispatch, the river ate the road up ahead. Over.").
2. A teaser card with silhouettes of the 4 vehicles still to come, and "Follow on itch.io".
3. The end screen as today (stats, MVP / Most useless, awards), plus the par medal.

## Voice: the CB radio
The squirrels never talk. A **CB radio** they salvaged from the crashed truck crackles on the jeep's dashboard. One voice actor plays everyone on it: the user's friend, a woman with a slightly baritone voice, in her own voice (no old-lady voice), with plenty of room for quirky characters.

| Character | Who | Voice |
|---|---|---|
| **Dispatcher** | the gourmet acorn company's dispatcher, tracking the stolen cargo; carries the story | dry, deadpan, unimpressed, very professional about very stupid things |
| **Beaver foreman** | owns the excavator the squirrels take | jittery, fast, panicky, protective of his machine |
| **Trucker** | phones in sightings | big, warm, slow drawl, loves radio gossip |
| **Conspiracy guy** | sure the squirrels are organising | whispery, paranoid, absolutely certain |

The dog's barks stay as sounds, not voice.

**Rules for the lines:**
- Short and punchy: players hear them mid-panic.
- Subtitles always on.
- Several variants per trigger, so lines don't repeat.
- A cooldown, so the radio never talks over itself.
- About **60 lines for the demo**.

**What triggers a line (the VA brief):**

| Trigger | Who | Variants |
|---|---|---|
| Opening (the crash and the jeep built) | Dispatcher | 2 |
| Run start / countdown | Dispatcher | 3 |
| New section reached | Dispatcher, Trucker | 1 per section (17) |
| Checkpoint saved | Dispatcher | 4 |
| Fork reached / branch chosen | Dispatcher, Conspiracy guy | 2 + 2 |
| Vehicle swap (excavator taken) / building smashed | Beaver foreman | 3 + 2 |
| A part breaks (engine, brakes, tyre, door) | Dispatcher, Trucker | 1–2 each |
| A squirrel flung out | Trucker | 3 |
| The car flipped / reset to the trail | Dispatcher | 2 + 2 |
| Wreck rewind | Dispatcher | 2 |
| Big jump | Trucker | 2 |
| The dog | Trucker, Conspiracy guy | 2 |
| Behind par (the snow creeps in) | Dispatcher | 3 |
| Idle chatter on the long roads | Conspiracy guy, Trucker | 4 |
| Camp reached / demo cliffhanger | Dispatcher | 1 + 1 |
| Ending (on time / late / no acorns) | Dispatcher | 1 + 1 + 1 (full game) |
