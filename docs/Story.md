# Story: the golden acorn heist

Decided 29 Oct 2026 with the user, in two design sessions: the outline, then this expansion. **Nothing here is built yet.** Friend test 3 (Sat 28 Nov) tests the game as it is today. If it passes, Chapter 1 becomes the public demo, and the full game is built around this story. The build order is in [Roadmap.md](Roadmap.md#story-demo-after-friend-test-3), and gameplay detail is in [GDD.md](GDD.md).

This file is the bible: the whole story, with the reasons behind it. Three other versions are made from it:

| Version | For | Source | PDF |
|---|---|---|---|
| Radio script | us and the game | [story/RadioScript.md](story/RadioScript.md): every radio line with its ID, trigger and direction, and how the radio picks a line | none |
| Story overview | proofreaders | [story/StoryOverview.html](story/StoryOverview.html): the story told plainly, with no game words and numbered paragraphs | `Squirrel Wheels - Story.pdf` |
| Voice script | the voice actor | [story/VoiceScript.html](story/VoiceScript.html) (a template) filled with the opening and Chapter 1 lines from RadioScript.md | `Squirrel Wheels - Voice Script.pdf` |

How to keep them in step:
- `Tools/Docs/MakePdf.ps1` writes both PDFs to Downloads.
- Radio lines are edited only in RadioScript.md, because the voice script is generated from it.
- A story change here that the overview tells needs the same change in StoryOverview.html.
- [story/VA_Sample.md](story/VA_Sample.md) is the first sample sent to the voice actor (29 Oct). The voice script replaces it.

## The story in one breath
A crew of squirrels from the Old Oak in Maple Park is bored of plain acorns, so they sneak onto an Acornucopia delivery truck for a taste of the golden ones. The truck doesn't stop for three days. Then it swerves for a deer and crashes into a mountain junkyard. The crew builds a jeep out of a toy and junk, loads the golden acorns, and drives home across six days of road before the first snow. A guard dog chases them the whole way. The truck's radio broadcasts every crash to a dispatcher, who slowly works out who she's been talking to.

The loading screen shows it as an **intro card**: text only, ~5 s, so a viewer who joins a stream gets the setup.
> The squirrels of the Old Oak were sick of plain acorns.
> So they snuck onto an Acornucopia truck for a taste of the golden ones.
> The truck didn't stop for three days.

A comic-strip intro (four still panels: the heist, the drive, the deer, the crash) is a Later idea, if an artist turns up.

## The world

### Setting and tone
- **Setting:** a made-up mountain country with a North American feel: pines, ranger stations, yellow school buses, freight trains, CB slang. Every place name is invented.
- **Units:** the HUD stays in km. The radio never uses units ("the next bend", "a long way down").
- **Tone:** family-friendly slapstick. Squirrels get flung, dunked and chased, but never hurt.
- **No swearing:** a caller who's about to swear gets cut off by static ("What the—" *kssht*).
- **The cargo is always golden acorns** (or "the cargo"), never "nuts".
  - "Nuts" only as subtle wordplay ("this whole operation is nuts", "in a nutshell"), never about the cargo and never near mouths.
  - "Acorn in Mouth" keeps its name.

### Who talks
- Humans talk, and so does **Foreman Buck, a beaver** who runs a construction firm. Everyone treats a beaver contractor as normal.
- **The squirrels never talk**, not on the radio and not in text. The horn is their only voice.
- TINY only barks. The deer only stares.
- **The joke:** nobody blinks at a beaver with a digger licence, but nobody believes squirrels can drive. Only Night Owl points it out (N-16).

### The radio
- **Whose radio it is:** the CB on the jeep's dashboard is **Unit Seven's truck radio**. Unit Seven (the truck's driver) walked to town for a tow after the crash and left it on. The squirrels bolted it into the jeep.
- **The crash jammed its talk button,** so it transmits everything around the jeep: the engine, crashes, the horn, squirrels panting. That explains three things:
  - how Dispatch hears the jeep without seeing it (D-05: "your engine doesn't go 'bzzt'");
  - why she says "if that's you breathing";
  - why the honk test works at all.
- **Unit Seven never answers.** Dispatch calls "Unit Seven" all week, but the radio is in the jeep. Unit Seven speaks once, in the ending, on a new radio.
- **The other voices** are on the open CB channel: a trucker (Gravy Train), a conspiracy caller (Night Owl), and Buck's site radio.
- **Every golden-acorn crate has a GPS tag.** Dispatch always knows where the cargo is, but never what it's doing. She hears and guesses until Chapter 4, when she sees the jeep from her window.
- **Who knows what:**
  - Dispatch hears (the jammed radio) and tracks (the GPS).
  - Gravy Train sees: that truck drives the same roads.
  - Buck sees Buck & Sons' own sites.
  - Night Owl hears the channel and is right about everything, though nobody believes a word.

### Time and the snow
- **Six days, one chapter each,** from dawn to dusk. The camps are the nights. The truck took three days to get here; a junk jeep with detours takes six.
- **The morning call:** each chapter starts with Dispatch's call, which includes the weather. "First snow expected in six days" counts down by one each morning. A crew behind par hears "...the snow's moving in early".
- **Chapter 6:** the snow reaches the high pass in the morning and follows the crew down to the city.
  - On time, they beat it to the Old Oak.
  - Late, it got there first.
- **Lighting:** chapters start in the morning, and camps are at dusk. A sun that moves during a chapter is a Later art idea.

### Acornucopia
- "Fine glazed acorns since 1962". Golden acorns are its premium line: glazed, shiny, famous.
- Its headquarters and factory are in the Chapter 4 town. Dispatch's office window looks onto the road.
- **Its products are everywhere along the road.** Each chapter's acorn cache is one of them: a picnic hamper, a doorstep delivery box, snack packs, the outlet store, a vending machine. Dispatch logs every raid.
- "The client" (Acornucopia's management) keeps asking for an ETA, and in the ending, for its cargo.

### Buck & Sons
- **Five trades, five machines:** Foreman Buck's company holds the highway contract all the way home: "demolition, earthworks, crane hire, the school run and snow clearing". So every helper vehicle is Buck's: the **excavator, sand pickup, crane truck, school bus and snowplow**.
- **Every helper job is on Buck's to-do list.** The squirrels keep doing Buck's contract by accident: the ranger station due to come down on Thursday, the sinkhole, and so on. That's why Buck goes from furious to grateful.
- **The hazards on the road today are Buck's crew's mess:**
  - the wrecking ball on THE BRIDGE: the crew is taking the old railings off, which is why the deck has none;
  - the roadblocks.
- Buck leaves the keys in everything (B-08).

## The cast

| Who | What | Voice | Ch1 lines |
|---|---|---|---|
| **The crew** | the players: 2–4 squirrels from the Old Oak, unnamed | never speak | none |
| **Dispatch** | Acornucopia's radio dispatcher; the lead | dry, deadpan, very professional about very stupid things | 50 |
| **Foreman Buck** | a beaver, boss of Buck & Sons; owns every helper vehicle | jittery, fast, panicky, protective of every machine | 12 |
| **Gravy Train** | a trucker's CB handle; the crew's first and biggest fan | big, warm, slow drawl, loves radio gossip | 36 |
| **Night Owl** | a conspiracy caller who listens all night (and now all day) | whispery, paranoid, absolutely certain | 16 |
| **Unit Seven** | the truck's driver | one line, in the ending | 0 |
| **TINY** | the junkyard's enormous guard dog | barks (sound effects) | none |
| **The deer** | the deer that caused the crash | silent | none |

### The crew
- **Who they are:** the players themselves, unnamed. Players' own names already float over their heads, and a character name would fight that.
- **Where they're from:** foragers from the squirrel colony in the Old Oak, a huge tree in Maple Park, a city park far down the valley.
- **Why they did it:** they aren't starving. They're bored foodies who promised the colony a feast ("this winter we eat like kings"). The colony waits at the oak: elders and kits.
- **What the radio calls them:** by job ("the one on the wheel") or by fur colour ("the orange one's out again": orange, blue, pink, lime). Never he or she, because any player can be any squirrel.

### Dispatch
- Acornucopia's radio dispatcher, tracking every truck by GPS. A woman, only ever called "Dispatch".
- **Voice:** the voice actor's own voice, dry and deadpan. Her pauses ("...") are where the comedy lives.
- **The part:** the lead, and the voice actor's own part (her friends can take the others).
- **Arc:**
  - by the book on day 1, calling Unit Seven all day;
  - suspicious from the honk test;
  - in denial on day 2, then testing them;
  - a fan from the moment she sees them from her window (day 4);
  - helping openly at the dam;
  - guiding them home and covering for them ("Cargo... lost in transit. Write it off.").

### Foreman Buck
- **Who:** a beaver who runs Buck & Sons. The squirrels borrow one of Buck's machines every day.
- **Arc:**
  - furious on day 1 ("Off my digger!");
  - confused on day 2 (they filled Buck's sinkhole for Buck);
  - resigned on day 3 ("...that's my bus too. Of course it is.");
  - past caring on day 4 ("Bring it back with a full tank.");
  - directing the crane for them on day 5;
  - driving the snowplow ahead of them on day 6.
- The ending line: "Best week of work we ever had. And we didn't do any of it."
- **The voice is open:** a different voice from Dispatch helps players tell callers apart over the static.

### Gravy Train
- **Who:** a trucker who drives the same roads all week, so Gravy Train sees what Dispatch can only hear: the fires, the squirrels falling out, the dog.
- **The colour callouts are Gravy Train's job.** That makes it the second-biggest part.
- **Arc:**
  - the first sighting on day 1;
  - telling every trucker on day 2;
  - a fan club on day 3;
  - truckers honking hello on day 4;
  - a cheering convoy at the dam;
  - "Proudest day of my life" on day 6.

### Night Owl
- **Who:** a conspiracy caller who listens to the CB all night, and now all day: "Yes, it's daytime. No, I haven't slept."
- **Right about everything, believed by no one.**
- **The theories escalate daily:**
  - day 1: "They're organised";
  - day 2: "They have a navy";
  - day 3: "They're building infrastructure";
  - day 4: "They're in the rail network";
  - day 5: "They control the water";
  - day 6: "They're going home. Under OUR parks."
- Night Owl gets the last word, after the credits: "This isn't over."

### Unit Seven
- The truck's driver. Swerved for the deer, walked to town for a tow, and left the radio on.
- Never answers: the radio is in the jeep.
- **One line, at the very end:** "Dispatch, this is Unit Seven. Got a new radio. ...Did I miss anything?" Dispatch: "...No. Quiet week. Over."

### TINY
- **Who:** the guard dog of Rusty Bucket Salvage. Enormous, with "TINY" on its collar.
- **Day 1:**
  - chained up and barking through the whole build;
  - snaps its chain when the jeep leaves;
  - chases them out of the junkyard, harmlessly;
  - comes back for real on the way down the mountain.
- **Afterwards:** it turns up once every chapter, and it needs no reason: chasing cars and chasing squirrels just became the same thing.
- A honk scares it off.
- **It won't cross a river on day 1,** and swims a flood on day 2. It keeps getting more determined.
- **At the end** it limps into Maple Park, too tired to chase, and curls up under the Old Oak. The colony lets it stay.

### The deer
The deer that caused the crash. It stands at the edge of the junkyard on day 1, then at every camp, calmly watching the jeep arrive: at the barn, at the quarry office, at pump four of the gas station. At the end it watches from across the park. **Nobody ever mentions it.** It's a background joke for viewers to spot.

## Arcs by chapter

| Chapter (day, forecast) | Dispatch | Buck | Gravy Train | Night Owl | TINY |
|---|---|---|---|---|---|
| 1 The Crash Site (day 1, "six days") | by the book; calls Unit Seven all day; the honk test | furious: "Off my digger!" | the first sightings | "They're organised." | chained; snaps the chain; ambushes them on the descent; won't cross the river |
| 2 The Washout (day 2, "five days") | denial: "Yesterday was a coincidence. I've decided." Or, if nobody honked, she tries again | confused: they filled Buck's sinkhole | tells every trucker the jeep is real | "They have a navy." | swims the flood |
| 3 The Old Quarry (day 3, "four days") | tests them ("honk once for yes"); her first "hypothetical" tip | resigned: "...that's my bus too. Of course it is." | starts a fan club | "They're building infrastructure." | slides down a gravel chute |
| 4 Town Outskirts (day 4, "three days") | sees them from her window and becomes a fan | "I'm not even mad. Bring it back with a full tank." | truckers honk hello | "They're in the rail network." | rides a freight train and jumps off |
| 5 The Dam (day 5, first frost, "two days") | helps openly: "Spillway opens in ninety seconds. ...I didn't tell you that." | turns foreman and directs the crane for them | the convoy cheers | "They control the water." | barks echo in the tunnel; runs along the dam wall |
| 6 First Snow (day 6, "tonight") | guides them home and covers for them | drives the snowplow ahead of them | "Proudest day of my life." | "They're going HOME. Under OUR parks." | the last chase, then asleep under the oak |

## How the story explains the game

| Mechanic | In the story |
|---|---|
| The jeep's parts | Built from junk: a car battery for an engine, a kitchen sponge for a brake, a cardboard door, patched tyres. That's why they break. |
| Repairs | Junk fixes: jiggle the battery, re-jam the sponge, slam and tape the door, hold the tyre on. |
| Seats and jobs | Squirrels are small: steering takes a whole squirrel, and so do the pedals. |
| Acorn in Mouth | Golden acorns are too good. One squirrel sneaks a taste and can't stop chewing, so it can only point (the pings). Night Owl: "They're eating the evidence." It pays off in the no-acorns ending. |
| Checkpoints | Dispatch logging the GPS position "for the insurance claim". |
| Wreck rewind | Dispatch strikes the crash from the record: "Let me read that back. ...No. That didn't happen." |
| Reset to the trail (hold R) | "They're back on the road. ...Don't ask me how." Night Owl: "They TELEPORTED." |
| Falling out, respawning | Slapstick, not explained: "They're fine. They bounce." |
| Score | Golden acorns delivered = the size of the feast. |
| Par medals, snow creep | How close winter came to catching them; Dispatch's forecast moves up. |
| The camp waits for everyone | Nobody sleeps until the whole crew is in. |
| The horn | The squirrels' only voice. It scares TINY, and it answers Dispatch in the honk test. |
| Forks | The long road passes an Acornucopia product to raid. The short road needs one of Buck's machines. |
| Helper vehicles | Buck & Sons' machines, parked along the road with the keys in. |
| Fur colours | The radio's colour callouts. |

## The clock and the endings
- **No deadline. A stopwatch** runs over the whole journey and carries across chapters.
- Each chapter has **par times**, which give a medal: **Golden Acorn**, **Acorn** or **Late**.
- **Score** = golden acorns delivered × 10 + a bonus for beating par − respawns.
- **Winter creeps in when you're slow.** Behind par, the world gets snowier: flurries, frosted props, the forecast moving up on the radio. It's only cosmetic.
- **The only way to lose is a total wreck,** and even that only rewinds to the last checkpoint.
- The **ending** depends on the total time and the acorns left ([The endings](#the-endings)).

## Structure: chapters that feel like one trip
- **One trip, not six levels:** 6 chapters that should feel like one continuous trip, the way RV There Yet does. A chapter takes ~6–9 min.
- **One map per chapter,** joined at a **camp** by **seamless travel**:
  - clients stay connected, the stopwatch keeps running, and there's no menu in between;
  - it reuses the seamless-travel restart that already exists.
- **A camp is the night,** in this order:
  1. the jeep and every squirrel arrive;
  2. night falls: the day's stats, and a Night Owl line (it's their hour);
  3. dawn, the travel to the next map, and Dispatch's morning call.

## Saving and checkpoints (RV There Yet style)
RV There Yet has no manual save: it autosaves at checkpoints (gas stations, new regions), and dying loses everything since the last one. Ours works the same way, more gently:
- **Automatic checkpoints** at each camp and at the start of each named section. A "CHECKPOINT" toast shows when one saves, and the radio may log it (D-34).
- **A total wreck rewinds to the last checkpoint.** The car, the acorns and the parts come back as they were there, and the stopwatch gets **+30 s**. Dispatch strikes the crash from the record (D-35).
- **Fallen squirrels** respawn at the car automatically, as today. There's no revive item: with only 2 players, a squirrel who needs a revive could be stuck.
- **Quit and continue:** the host can quit, then **Continue** from the last checkpoint with any crew.
- **A camp only saves when the jeep AND every squirrel are inside it.**
  - Until then the HUD shows "WAITING FOR <name>".
  - Squirrels left behind are **not** teleported to the camp. They have to get there themselves (they still respawn at the jeep if they fall).

## Forks that merge back
Each chapter except the last has **one fork in the road**, and the two roads meet again further on:
- The **short road** needs one of Buck's machines.
- The **long road** is longer and rougher, with an **acorn cache**: an Acornucopia product to raid.
- **Each fork trades speed for acorns,** because acorns carry over the whole journey and spills are permanent. A crew at 0 acorns drives on to the saddest (and funniest) ending.
- **Splitting up is allowed:** for example, one squirrel works the machine alone while the others guard the parked jeep.
- **TINY strikes after the merge,** or on both roads, so every crew meets it.

## Helper vehicles
The junk jeep is the hero. Buck's machines solve one obstacle each.

| Machine | Buck's trade | Chapters | Job | Seats (controls split, as in the jeep) |
|---|---|---|---|---|
| **Excavator** | demolition | 1 | knock down the ranger station | Tracks (drive and turn) / Boom (arm up/down and swing) / Bucket (curl and smash) |
| **Sand pickup** | earthworks | 2, 3 | fill a sinkhole; build a ramp | Wheel / Pedals / Tipper |
| **School bus** | the school run | 3, 5 | lie across a gap so the jeep drives over its roof | Wheel / Pedals / Doors? |
| **Crane truck** | crane hire | 4, 5 | lift the jeep over the rail line; hold the bus over the spillway | Drive / Boom / Winch |
| **Snowplow** | snow clearing | 6 | clear the road down the pass | none: Buck drives it, and the jeep follows |

Seats other than the excavator's are placeholders.

**The rules:**
- **The swap:** park the jeep at a marked bay (a parked jeep doesn't wear or spill), walk or hop to the machine, do the job, walk back. The stopwatch keeps running throughout.
- **Splitting the crew is allowed.** A machine works with whoever is in it. A lone squirrel can work one by hopping between its seats, slowly and badly.
- **With 2 players** on the excavator, one works Tracks and the other Boom and Bucket. Riders fix parts.
- **Machines break too:** 1–2 parts each on the existing part system (the excavator's hydraulic hose bursts, for example), so the repair juggling continues.
- **How many, and when:** a new machine does its first job alone, and the chapters with two machines use ones the crew already knows.

  | Ch | Machines |
  |---|---|
  | 1 | excavator (new) |
  | 2 | sand pickup (new) |
  | 3 | a sand ramp (known), then the school bus (new) |
  | 4 | crane truck (new) |
  | 5 | the crane holds the bus, both at once; a placed machine stays put, so 2 players can do it |
  | 6 | Buck's snowplow, which the crew doesn't drive |

## Chapter 1: The Crash Site (the demo)
- **Day 1.** The forecast: first snow in six days.
- **The map:** today's L_Route (5.4 km, 17 sections). Three sections are renamed: THE PARK → **THE JUNKYARD**, HOME STRETCH → **THE DESCENT**, THE DEPOT → **THE CAMPGROUND**. The fork is new. The other 14 names stay.
- **The lines** are in [RadioScript.md](story/RadioScript.md#chapter-1-the-crash-site): 114 with variants, in story order, then the event lines.

### The loading screen
The intro card ([above](#the-story-in-one-breath)), ~5 s, before the lobby loads.

### THE JUNKYARD: the lobby (today THE PARK)
**What's there:**
- **Rusty Bucket Salvage,** a mountain junkyard. The Acornucopia truck lies jackknifed through the fence, smoking, its trailer split open. Golden-acorn crates are spilled across the yard, among junk piles.
- **TINY,** chained at its kennel by the gate, barking at everything.
- **The deer** at the treeline, watching.
- **The jeep,** in pieces in the middle.
- **THE PARK's control boards** stay, re-dressed as junkyard signs.

**What happens:** players gather (the Waiting phase), explore the wreck and build the jeep. Dispatch's opening plays once per session:
1. the morning weather (D-01);
2. Dispatch calling Unit Seven (D-02);
3. Gravy Train saying the driver walked to town (G-01, D-03);
4. Night Owl hearing "small hammering" (N-01);
5. D-04 if they take a while.

**Why:** the story is told while players poke around. There's no cutscene and no new art beyond props.

### Building the jeep: the tutorial
- **The jeep starts in pieces:** the battery engine on the ground by the hood, the sponge out of the brake, the door hanging off, one tyre flat. Each is that part in its failed state, with a label ("PUT THE BATTERY ON", and so on).
- **Fixing each one once builds the jeep** and teaches its repair: battery = Mash, sponge = the timing ring, door = slam (Mash 3), tyre = Hold.
- **The radio comments on each,** because Dispatch hears it through the jammed radio: D-05 ("your engine doesn't go 'bzzt'"), D-06, N-02, D-07 ("small grunting"). When the jeep is whole: D-08 ("whatever you've just built: don't").
- **Nothing goes wrong during the build:** no failure director, no wear, no spills.
- **The countdown starts when the jeep is whole and 2+ squirrels are in.** Whether the host can hold it for a friend who's still joining is an [open question](#open-questions).

### The start
- **Go:** the jeep drives off with the crates. D-09 ("Your cargo is driving."), then D-10 ("...And why is there a dog.").
- **TINY snaps its chain** and chases the jeep out of the junkyard.
  - **It can't bite yet:** it's a harmless chase that sets up the grudge.
  - It gives up, panting, at the gate: N-03 ("That bark sounded personal.").

### The mountain: SQUIRREL CREEK to LITTLE JUMPS
Today's sections and hazards are unchanged. The story is the dressing and the radio.

| Section | What's there today | Story | Radio |
|---|---|---|---|
| SQUIRREL CREEK | the first ford | the first sighting | G-02, then D-11 (the alert) |
| COBBLE LANE | lumpy cobbles; a pinch point of rocks and logs | acorns bounce out | G-03 |
| ACORN HILL | a big hump on the climb | | N-04 |
| PINE PASS | sweeping bends; a broken-down sports car across the road | a tourist's car, abandoned | G-04 (squeeze by on the drop side) |
| THE HAIRPIN | a 170° bend; a fallen pine | | G-05 (there's a gap by the edge) |
| THE BRIDGE | a deck with no rails over a gorge; the wrecking ball | **Buck & Sons' bridge crew**, taking the old railings off with the ball (which is why there are none): a site hut, signs, cones | B-01 to B-04 |
| THE HUMPS | two big humps | | D-12 |
| ROCKY ROAD | the big climb; a rockslide | Buck's crew's rockslide | B-05 |
| SQUIRREL SLALOM | pillars on the summit | | N-05 |
| THE BIG JUMP | a kicker, with room to go round | | G-06 |
| LITTLE JUMPS | two smaller kickers | | D-13 |

### The fork: the top of WIGGLY ROAD
Two signs: WIGGLY ROAD, and BUCK'S CUT — ROAD CLOSED. Dispatch reads the map: D-14 ("You'll take the closed one. Of course you will.").

**BUCK'S CUT (the short road, new):**
- **The road:** a construction road straight down the mountain to COBBLE CURVE, much shorter than WIGGLY ROAD. Check it against the 18 % grade limit when it's built; it may need a switchback.
- **What's on it:** an old **ranger station** blocks the road, due for demolition on Thursday (it's on Buck's list). Beside it: **Buck's excavator**, keys in, and a **jeep parking bay**.
- **What the crew does:**
  1. parks the jeep (no wear, no spills while parked);
  2. takes the excavator (Tracks / Boom / Bucket);
  3. knocks the station down;
  4. walks or drives back;
  5. drives the jeep through the rubble.
- **The excavator's part:** a hydraulic hose that bursts (B-10).
- **Radio, in order:**
  - B-06: off my digger!
  - B-07
  - B-08: the keys
  - B-09: the bucket, not the cab
  - B-10
  - B-11: "They did Thursday's job."
  - D-15: the GPS inside the ranger station
  - B-12: left in the road
- **Afterwards:** the excavator stays wherever they leave it.

**WIGGLY ROAD (the long road, today's):**
- Six bends and the pinch point, as today. The dog no longer lives here.
- **The acorn cache:** an **Acornucopia picnic hamper** at a lookout, which refills the acorns lost so far (how many is an [open question](#open-questions)).
- Radio: G-07, D-16a/b ("...and now they've robbed a picnic.").

### COBBLE CURVE and THE DESCENT: TINY's ambush
- **COBBLE CURVE:** the roads meet (D-17: "you're back on my map"), and TINY bursts out at the start of the curve.
- **G-08 teaches the horn** before the honk test: "Collar says TINY. ...Somebody honk at it!"
- **The chase:** TINY chases from COBBLE CURVE down THE DESCENT (today HOME STRETCH; its pinch point gets G-09). It gives up at THE WASH's water: TINY doesn't like rivers. Yet (see Chapter 2).
- **What TINY does is today's dog, unchanged:**
  - it bites the nearest tyre (wear, sometimes a flat);
  - it grabs a squirrel on foot and flings it;
  - a honk scares it off for 10 s.
- Event lines: G-30 to G-32, N-11.

### THE WASH
The second ford. D-18.

### THE CAMPGROUND (today THE DEPOT)
- **What's there:** a campground at dusk, with a fire pit and picnic tables. The finish gate becomes the campground gate. The deer is there, watching.
- **The camp waits** for the jeep and every squirrel:
  - the HUD shows "WAITING FOR <name>";
  - G-10 plays after 20 s of waiting;
  - D-19 plays when the jeep arrives;
  - the camp is a checkpoint.
- **Then the honk test,** the demo's cliffhanger.

### The honk test
Everyone's in. The radio crackles: "Unit Seven, you've been quiet all day. ...Unit Seven, if that's you breathing, honk twice." (D-20)

| The crew does | Dispatch says |
|---|---|
| honks twice | "...That was a coincidence. ...Honk three times." (D-21), then: |
| ...then three times | "...Okay. That's not Unit Seven. ...Who IS this?" (D-22) |
| ...then anything else | "Coincidence. Definitely a coincidence." (D-23) |
| honks 1, 3 or 4 times | "Coincidence. Definitely a coincidence." (D-23) |
| doesn't honk | "...Great. I'm talking to a wrecked truck." (D-24). She tries again at the start of Chapter 2. |
| honks 5 times or more | "...That was a lot more than two. Are you okay in there?" (D-25) |

- **What counts:** only honks after her line ends. The exact counting rules are in [RadioScript.md](story/RadioScript.md#the-honk-test-end-of-chapter-1).
- **Why:** it's the demo's best clip ("we talked to the radio with the horn!") and the hook for the full game. Dispatch's change of heart starts here and spreads over Chapters 2–6.

### The demo ending
1. The honk test.
2. **The teaser card:** silhouettes of the four machines still to come (sand pickup, school bus, crane truck, snowplow) and "Follow on itch.io". N-06 plays over it if anybody honked: "She's talking to them now."
3. **The end screen,** as today (stats, MVP / Most useless, awards), plus the par medal.

In the full game the camp turns to night instead, and Chapter 2 starts at dawn.

### What Chapter 1 needs built
Roadmap items in brackets ([Roadmap.md](Roadmap.md#story-demo-after-friend-test-3)).
- [ ] **Stopwatch and par medals** instead of the deadline; the new score (S1).
- [ ] **Checkpoints** at section starts and the camp; the wreck rewind; host Continue (S2).
- [ ] **The radio:** triggers, priorities, variants, subtitles, every Ch1 line as a subtitle until the recordings arrive (S3).
- [ ] **THE JUNKYARD (S4):**
  - the crashed truck, the crates, junk piles, TINY's kennel and chain, the deer;
  - the control boards as junkyard signs;
  - the jeep in pieces, with building it as the tutorial;
  - the intro card on the loading screen.
- [ ] **The three renames (S4):** in `ANBRoute`'s segment list and in L_Route's baked sign texts. Edit the signs by hand, because a re-bake would lose the hand edits.
- [ ] **Buck's Cut (S5)** (the road, plain fork signs and the picnic hamper are pulled forward to friend test 3: Roadmap J2 and J13, 31 Oct):
  - the road (grade check), the ranger station (destructible), the excavator and its seats and hose, the parking bay;
  - the fork signs;
  - the picnic hamper on WIGGLY ROAD.
- [ ] **Golden acorns** (S6) and the **snow creep** (S7).
- [ ] **TINY re-staged (S9)** (the ambush stretch is pulled forward to friend test 3: Roadmap J12, 31 Oct):
  - chained in the junkyard, then the chain-snap chase with no bite;
  - the ambush stretch moved from WIGGLY ROAD to COBBLE CURVE through THE DESCENT;
  - it stops at THE WASH.
- [ ] **Buck & Sons dressing on THE BRIDGE:** a site hut and signs (part of S4).
- [ ] **The honk test, the teaser card** and "Follow on itch.io" (S8).

## Chapters 2–6
Beats for each. Lines are drafts in [RadioScript.md](story/RadioScript.md#chapters-26-drafts). Section names and road ideas are drafts too: change them freely when a chapter is designed for real.

### Chapter 2: The Washout
- **Day 2.** A storm overnight flooded the farmland below the mountains. The forecast: first snow in five days.
- **The morning call:**
  - if anybody honked last night, firm denial: "Yesterday was a coincidence. I've decided." (D-201);
  - if nobody did, she tries the honk test again (D-202).
- **The road:** muddy farm tracks, flooded lanes, fences and hay bales, a scarecrow. Rain lowers grip, and puddles hide potholes.
- **Draft sections:** THE FLOODED FARM, SCARECROW CORNER, HAYBALE HILL, then the fork (THE SINKHOLE or THE FLOOD), PORCH LANE, THE BIG BARN.
- **The fork:** the storm has washed a sinkhole into the main road, the next job on Buck's list.
  - **Short road:** take Buck's **sand pickup** (Wheel / Pedals / Tipper), back it up to the hole, tip the sand in, drive over. Buck: "They filled my sinkhole. That was on my list. Why are they doing my list?" (B-202)
  - **Long road:** ford the flooded river. It's deeper than Chapter 1's fords, and the current sweeps away any squirrel who steps off the jeep.
- **The cache:** an Acornucopia doorstep delivery box on a farmhouse porch, on the long road (D-203).
- **TINY** swims the flood after them (G-202). It wouldn't cross a river yesterday.
- **The camp:** a barn on a hill, high above the water. The deer is already there.
- **Arcs:**
  - Gravy Train tells every trucker the jeep is real (G-201).
  - Night Owl: "They have a navy." (N-201)

### Chapter 3: The Old Quarry
- **Day 3.** Gravel pits, cliffs, conveyor belts, rusting machinery. The forecast: first snow in four days.
- **The morning call:** Dispatch's first "hypothetical" tip: a school bus is parked by the pit (D-301). Later she tests them: "Honk once for yes." (D-302, D-303)
- **The road:** loose gravel that drifts, quarry trucks, and a blasting schedule (a timed rockfall).
- **Draft sections:** THE BLASTING ZONE, THE CONVEYORS, then the fork (THE PIT or THE SWITCHBACKS), LUNCH LEDGE, THE QUARRY OFFICE.
- **The fork:** the road ends at a cut in the rock, too wide to jump.
  - **Short road, two machines in a row:**
    1. the **sand pickup** builds a ramp up to the high ledge;
    2. the **school bus** drives up it and parks across the cut;
    3. the jeep drives over the bus's roof.
    - Buck: "...that's my bus too. Of course it is." Then: "The bus is not a bridge! The bus is for CHILDREN!" (B-301, B-303)
  - **Long road:** the switchbacks all the way down into the pit and back up the other side, on loose gravel, under rockfall.
- **The cache:** the quarry workers' lunch cooler, full of Acornucopia snack packs (D-304).
- **TINY** takes a shortcut down a gravel chute, on its belly (G-302).
- **The camp:** the quarry office, a portacabin.
- **Arcs:**
  - Gravy Train starts a fan club (G-301).
  - Night Owl: "They're building infrastructure." (N-301)

### Chapter 4: Town Outskirts
- **Day 4.** The road leaves the wild for town. The forecast: first snow in three days.
- **The road:**
  - suburbs, traffic to dodge, roundabouts, the ring road, rail yards;
  - the **Acornucopia factory**, with a giant golden acorn on the roof;
  - next door, the **headquarters**, where Dispatch works.
- **Draft sections:** THE RING ROAD, ROUNDABOUT ROW, FACTORY ROW, then the fork (THE RAIL YARD or THE CROSSING), OUTLET ALLEY, PUMP FOUR.
- **The window:** the jeep passes under Dispatch's office window. "...They're outside my window. ...Are they waving?" Then: "Okay. I'm a fan. Nobody tell the client." (D-402, D-403)
  - Players can honk at the window.
  - From here on, Dispatch is on their side.
- **The fork:** the rail yard blocks the way.
  - **Short road:** Buck's **crane truck** (Drive / Boom / Winch) lifts the jeep clean over the rail lines. Buck: "I'm not even mad. Bring it back with a full tank." (B-401)
  - **Long road:** the level crossing. A freight train takes forever to pass, unless the crew gambles on beating it before the barrier comes down.
- **The cache:** the Acornucopia outlet store by the crossing. Dispatch logs the raid "as a promotion" (D-404).
- **TINY** rides a passing freight train and leaps off in front of the jeep (G-402).
- **The camp:** a gas station. "Their jeep runs on a battery. Why are they at a gas station?" (D-405) The deer is at pump four, and nobody mentions it.
- **Arcs:**
  - Truckers on the ring road honk hello (G-401).
  - Night Owl: "They're in the rail network." (N-401)

### Chapter 5: The Dam
- **Day 5.** First frost. A huge dam across the valley, the reservoir, the spillway. The forecast: first snow in two days.
- **The morning call:** "You're going to make it. Probably." (D-501)
- **The road:** frost on the road (ice patches, less grip), the reservoir shore, the dam wall.
- **Draft sections:** FROST ROAD, THE RESERVOIR, THE DAM WALL, then the fork (THE SPILLWAY or THE SERVICE TUNNEL), THE VISITOR CENTRE, THE LOOKOUT.
- **The fork:** the spillway cuts the road over the dam, and it opens on a timer. Dispatch now helps openly: "Spillway opens in ninety seconds. ...I didn't tell you that." (D-502)
  - **Short road, two machines at once:** the **crane truck** holds the **school bus** across the spillway while the jeep drives over it.
    - Buck directs over the radio, finally the crew's foreman: "Swing left. LEFT. My left! ...Hold the bus! Go, go, go!" Then: "That was beautiful." (B-501, B-502)
    - A placed machine stays put, so 2 players can do it.
  - **Long road:** the dark service tunnel inside the dam, lit only by the headlights, narrow and leaking.
- **The cache:** the visitor centre's vending machine. "They didn't pay. Of course they didn't pay. They're squirrels." (D-503)
- **TINY:** its barks echo through the tunnel (G-502), then it runs along the top of the dam wall.
- **The camp:** the dam lookout. Far below, the lights of the city. Home is in sight: "Last night on the road, Unit Seven." (D-504)
- **Arcs:**
  - A convoy cheers from the dam road (G-501).
  - Night Owl: "They control the WATER now." (N-501)

### Chapter 6: First Snow
- **Day 6.** The snow reached the high pass overnight, and it will reach the city tonight.
- **The morning call:** "Go. Go now." (D-601)
- **No fork and no cache.** Buck drives the **snowplow** down the pass, and the jeep follows in its track. "Stay in my tracks and don't touch anything. ...Please." (B-601)
  - The plow drives itself, like the autopilot, so the crew only drives the jeep. The finale is a pure chase home.
  - Off the plow's track, deep snow slows the jeep.
- **The storm follows them down:** behind par, it catches up visibly. D-603: "The snow's right behind you. ...Don't look. Just drive."
- **TINY's last chase** through the snow (G-602).
- **The road:** the pass, snowy switchbacks, the old mill, city streets, the gates of Maple Park. Dispatch guides them through town: "Left at the old mill, then straight down to the park. ...Hypothetically." (D-602)
- **Draft sections:** THE HIGH PASS, PLOW ROAD, SNOWY SWITCHBACKS, THE OLD MILL, CITY STREETS, MAPLE PARK.
- **Arcs:**
  - Gravy Train: "Proudest day of my life." (G-601)
  - Night Owl: "They're going HOME. Under OUR parks." (N-601)
  - Dispatch, at the park gates: "...They're home." (D-604)
- Then the ending.

## The endings
**Every ending:**
1. The jeep rolls into Maple Park at dusk, and the colony pours out of the Old Oak. Gravy Train: "Ten-four, buddies." (G-901)
2. TINY limps in behind, far too tired to chase, and curls up under the oak. Nobody makes it leave.
3. The deer watches from across the park.
4. On the radio, Acornucopia demands its cargo. Dispatch: "Cargo... lost in transit. Write it off." (D-901)

**What changes with the result:**

| Result | The scene | Dispatch |
|---|---|---|
| **On time** (beat the total par) | the first snowflake lands on Maple Park as the last golden acorn goes into the oak, and the feast begins | "First snow's falling. ...Somewhere out there, a crate of golden acorns made it home. Stand down." (D-902) |
| **Late** | the Old Oak is already snowed in; the crew digs the front door out, and the feast happens anyway | "Snow's been falling since noon. ...They made it anyway." (D-903) |
| **No acorns left** | the crate is empty and the colony stares. Then one golden acorn rolls out of a squirrel's cheek (Acorn in Mouth pays off), gets sliced two hundred ways, and is the best feast they ever had | "One acorn, I'm told. Six days for one acorn. ...Good for them." (D-904) |

**Then:**
1. Buck parks the plow: "Best week of work we ever had. And we didn't do any of it." (B-901)
2. At last, **Unit Seven calls in**: "Dispatch, this is Unit Seven. Got a new radio. ...Did I miss anything?" (U-901)
3. Dispatch: "...No. Quiet week. Over." (D-905)
4. **After the credits,** Night Owl: "They got home. They have a car now. ...This isn't over." (N-901)

## Running gags
- **The deer** is at every stop, and nobody ever mentions it.
- **Night Owl's theories** grow every day, from "organised" to "under OUR parks", and they're all true.
- **Buck loses one more machine every day** and slowly stops minding, because the squirrels keep doing the jobs on Buck's list.
- **Dispatch never stops calling "Unit Seven",** and Unit Seven never answers, until the very end.
- **Dispatch's "small" things:** small hammering, small grunting, a small scream, a smaller horn.
- **Gravy Train's colour reports:** "The orange one's out of the jeep again."
- **Every raid** is on an Acornucopia product, and Dispatch logs each one.

## Open questions
To decide when the story is built, not now:
- **The start of the run:** does the countdown start by itself when the jeep is whole, or can the host hold it for a friend who's still joining? Recommendation: by itself, with a 5 s countdown the host can cancel from the lobby menu.
- **Acorns:** how many the jeep carries (more than 40?) and how many a cache refills. Tune in playtests.
- **Par times** for each chapter, after hand-driven runs.
- **The final title.** The story uses "Squirrel Wheels" as a working title.
- **Later art:** the comic-strip intro, and a sun that moves during a chapter.
