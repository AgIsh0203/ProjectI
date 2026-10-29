# Radio script

Every line the jeep's CB radio says, for the story in [Story.md](../Story.md) (decided 29 Oct 2026). Nothing is recorded or built yet: the radio system is Roadmap item S3, after friend test 3.

The voice actor gets the lines between the `voice-script` markers below (the opening and Chapter 1) as a PDF. `Tools/Docs/MakePdf.ps1` builds it from this file, so **edit lines here, never in the PDF**, then rerun the script. The endings and Chapters 2–6 stay out of her script until they're closer to final.

## How to read this file
- **IDs:** a letter for the voice, then a number.
  - Letters: D Dispatch, B Buck, G Gravy Train, N Night Owl, U Unit Seven.
  - Numbers: 1–99 are the opening and Chapter 1, 201+ Chapter 2, 301+ Chapter 3 and so on, and 901+ the endings.
  - A letter after the number (D-34a, D-34b) marks variants of one moment: the game picks one.
  - An ID never changes once it's recorded; a new line gets a new number.
- **Recording:** the actor reads the ID before each take ("D-12, take one") into long files, and we cut them into `D-12.wav` and so on.
- **Rows:** `| ID | Who | When | Line | Direction |`, one line per row.
  - Never put a `|` inside a cell: the PDF script splits on it.
  - "When" says what's happening in plain words, for the actor. The [triggers](#triggers) table is the game's version.
  - In "Line", `...` is a beat, CAPITALS are louder, `—` is a cut-off, and `*kssht*` is radio static added later (not spoken).

## Rules for every line
1. **Short.** Players hear it mid-panic: most lines under 4 s, story lines up to ~8 s.
2. **Makes sense alone.** Players miss lines all the time, and event lines come in any order.
3. **The squirrels are never "he" or "she":** any player can be any squirrel. Use "that one", "one of 'em", the job ("the one on the wheel") or the fur colour (orange, blue, pink, lime).
4. **Only Dispatch is "she".** Buck, Gravy Train, Night Owl and Unit Seven are never he or she, in a line or a note: whoever voices them decides.
5. **The cargo is "golden acorns" or "the cargo", never "nuts".** "Nuts" only as subtle wordplay, never about the cargo, never near mouths.
6. **No swearing.** A caller about to swear gets cut off: "What the—" *kssht*.
7. **No units.** "The next bend", "a long way down", never km or miles.
8. **Who knows what:**
   - Dispatch hears the jeep through the jammed radio and sees its GPS, but can't see the jeep itself until Chapter 4.
   - Gravy Train sees it from the road.
   - Buck sees Buck & Sons' own sites.
   - Night Owl hears the channel and guesses, correctly.
9. **CB flavour, lightly:** "Over", "Breaker breaker", "ten-four", "Dispatch out". Mostly it's Dispatch who says "Over".

## How the radio picks a line
For S3, the radio system:
- **One line at a time,** with a short burst of static before and after.
- **Subtitles always:** the speaker's name in their colour, then the line, bottom centre.
- **Everyone hears it,** wherever they are:
  - it's a 2D sound through a radio filter (record clean; the filter is added in the game);
  - it ducks the music and the engine while it talks;
  - it doesn't fade with distance from the jeep, so a squirrel left behind still hears the story.
- **Three priorities:**
  - **Story** lines (the opening, section lines, the fork, the honk test, the rewind) queue and always play, in order.
  - **Event** lines play only if the radio is free and the trigger's cooldown has run out; otherwise they're dropped. Leave 4 s of quiet after any line before the next event line.
  - **Chatter** plays only after 40 s with nothing said, and each chatter line plays once per run.
- **Variants** come out of a shuffle bag: each plays once before any repeats. When an event lists two or three voices, they take turns, so it isn't always Gravy Train.
- **Before the recordings exist,** the subtitle shows on its own, with a static blip.

### The honk test (end of Chapter 1)
Once the jeep and every squirrel are in THE CAMPGROUND, D-20 plays.
- Honks count only after D-20 ends. The count closes 3 s after the last honk, or after 6 s with none.
- What plays next:
  - 0 honks → D-24. Dispatch tries again in Chapter 2's morning call (D-202).
  - exactly 2 → D-21, then a second count:
    - exactly 3 → D-22 ("Who IS this?");
    - anything else, 0 included → D-23.
  - 5 or more, in either count → D-25.
  - any other number (1, 3, 4) → D-23.
- Then the teaser card, with N-06 over it if anybody honked.
- The result is saved: Chapter 2's morning call is D-201 if anybody honked, D-202 if not.

### The morning call (Chapters 2–6)
Each chapter opens at dawn with Dispatch's morning call (D-201, D-301 and so on), which counts down to the first snow. A crew behind par at the last camp gets a variant that adds "...the snow's moving in early" (to be written with Chapter 2).

## Triggers
Chapter 1. "Once" means once per run unless it says otherwise.

| Trigger | Lines | Priority | Repeat |
|---|---|---|---|
| The lobby opens (the first time this session) | D-01, D-02, G-01, D-03, N-01, in that order | Story | once per session |
| Still in the lobby 90 s later | D-04 | Chatter | once per session |
| Building the jeep: the battery / the sponge / the door / the tyre fixed | D-05 / D-06 / N-02 / D-07 | Story | once each |
| The jeep is whole (all four fixed) | D-08 | Story | once |
| The run starts (go) | D-09, then D-10 | Story | once |
| TINY gives up at the junkyard gate | N-03 | Story | once |
| A section starts | the lines under its heading below | Story | once |
| BUCK'S CUT: a squirrel gets into the excavator / starts it / swings the arm near the station / its hose bursts / the station is down / the crew leaves it | B-07 / B-08 / B-09 / B-10 / B-11 / B-12 | Story | once each |
| The jeep passes the ranger station's rubble | D-15 | Story | once |
| The picnic hamper is raided | D-16a or D-16b | Event | once |
| At THE CAMPGROUND, still waiting for a squirrel after 20 s | G-10 | Event | once |
| Everyone is in THE CAMPGROUND | D-20, then [the honk test](#the-honk-test-end-of-chapter-1) | Story | once |
| A part breaks: engine / brakes / tyre / door | D-26 or G-11 / D-27 or G-12 / D-28 or G-13 / D-29 or G-14 | Event | 20 s per part |
| A part is repaired (outside the tutorial) | D-30, G-15 | Chatter | 30 s |
| A squirrel falls out of the jeep | G-16 to G-19 by its fur colour; G-20 if the door threw it; otherwise G-21 or D-31 | Event | 15 s |
| A squirrel is swept away in a river | G-22 | Event | 30 s |
| The jeep flips / is pushed back onto its wheels | D-32 or G-23 / G-24 | Event | each flip |
| The jeep is reset to the trail (hold R) | D-33a, D-33b, N-07 | Event | each reset |
| A checkpoint saves and nothing else is due | D-34a to D-34d | Chatter | 60 s |
| A total wreck rewinds to the last checkpoint | D-35a, D-35b | Story | each rewind |
| A clean jump with real air time | G-25a, G-25b, N-08 | Event | 20 s |
| The jeep falls a long way (off an edge) | D-36, G-26 | Event | 30 s |
| Off the trail for 3 s | G-27 | Event | 60 s |
| 3 or more acorns spill at once | D-37, G-28, N-09 | Event | 30 s |
| Acorn in Mouth starts | D-38, G-29, N-10 | Event | each time |
| TINY bites a tyre / is scared off by a honk / flings a squirrel | G-30a or G-30b / G-31 or N-11 / G-32 | Event | 10 s |
| The first honk of the run (before THE CAMPGROUND) | D-39 | Chatter | once |
| The snow creep starts (behind par) / well ahead of par | D-40a, D-40b, N-12 / D-41 | Event | once per section |
| 40 s with nothing said | D-42, D-43, G-33, G-34, N-13 to N-16 (N-16 only after THE BRIDGE) | Chatter | each once |

<!-- voice-script: start -->
## Chapter 1: The Crash Site
Day 1, the demo. Lines in story order, then the lines that can come at any time on the road.

### THE JUNKYARD: the opening
The crash site is the lobby. Players gather here while these play, once.

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| D-01 | Dispatch | The game opens at the crash site: the morning weather report, read to every truck. | Good morning, all units. Weather: clear in the mountains. First snow expected in six days. Dispatch out. | Routine. She's read the weather every morning for years. |
| D-02 | Dispatch | Then she calls the driver of the crashed truck. | Unit Seven, this is Dispatch. Your GPS says you've stopped. ...In a junkyard. Respond. Over. | Bored, then a beat as she reads "junkyard". |
| G-01 | Gravy Train | Dispatch is calling the crashed truck's driver. A trucker answers instead. | Dispatch, Gravy Train here. Passed your driver on the mountain road, walkin' to town for a tow. Looked fine. ...Looked sad. | Warm and slow. A gossip enjoying the news. |
| D-03 | Dispatch | She answers the trucker, who has just said the driver walked to town. | Copy that, Gravy Train. ...Unit Seven, when you're back at your truck: respond. Over. | Flat. Already tired of today. |
| N-01 | Night Owl | Meanwhile, in the junkyard, the squirrels are getting to work. | Night Owl here. Anybody else hear hammering out at Rusty Bucket? ...Small hammering. | Whispered, close to the mic. "Small" is the suspicious part. |
| D-04 | Dispatch | The crew is taking its time in the junkyard. | Unit Seven, Acornucopia would like to know why their truck is in a junkyard. ...So would I. Over. | Passing on a complaint. Dry. |

### THE JUNKYARD: building the jeep
The jeep starts in pieces, and each line plays as the crew fixes that piece. The truck's radio is stuck on "talk", so Dispatch hears everything near the jeep without seeing it.

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| D-05 | Dispatch | A car battery goes onto the jeep's hood, and the engine buzzes into life. | Unit Seven, I'm hearing an engine. ...That's not your engine. Your engine doesn't go "bzzt". | Puzzled, leaning in to listen. |
| D-06 | Dispatch | A kitchen sponge is jammed in as the brake. | Was that a squeak? ...Unit Seven, what are you building over there? | Suspicious. |
| N-02 | Night Owl | The cardboard door gets taped on. | Somebody's taping something. I can HEAR the tape. Nobody needs that much tape. Nobody. | Paranoid, building to a hiss. |
| D-07 | Dispatch | A squirrel holds a wobbly wheel on with all its strength. | Unit Seven, why do I hear grunting? ...Small grunting. | Deadpan. It echoes Night Owl's "small hammering". |
| D-08 | Dispatch | The jeep is finished. The race home is about to start. | Unit Seven, whatever you've just built: don't. Over. | A dry warning, with no hope it'll be obeyed. |

### The start

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| D-09 | Dispatch | The jeep drives off with the golden acorns. Every crate has a GPS tag, so she sees her cargo move. | Unit Seven, this is Dispatch. Be advised: your cargo has left the truck. ...Your cargo is driving. Over. | Calm and official, then slower as she rereads her screen. |
| D-10 | Dispatch | Straight after: the junkyard's huge guard dog, TINY, snaps its chain and chases the jeep, barking. | ...And why is there a dog. | Flat. Not even a question any more. |
| N-03 | Night Owl | TINY gives up at the junkyard gate, out of breath. | That bark sounded personal. | An ominous whisper. This dog will be back. |

### SQUIRREL CREEK
The first river.

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| G-02 | Gravy Train | A trucker sees the jeep for the first time. | Breaker breaker, this is Gravy Train. Just got passed by a jeep with squirrels in it. And one of 'em was steerin' with its whole body. | Delighted disbelief. |
| D-11 | Dispatch | Dispatch puts out an alert. | All units, be on the lookout for a small jeep. Possibly a toy. Driven by... I'm being told squirrels. Over. | Reading it off a form, hating every word. |

### COBBLE LANE
A very lumpy road, where acorns bounce out of the back.

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| G-03 | Gravy Train | The jeep reaches the cobbles. | Cobble Lane comin' up. Bumpiest road on the mountain. Hold onto your acorns. | A friendly warning. |

### ACORN HILL

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| N-04 | Night Owl | The jeep climbs Acorn Hill. | ACORN Hill. You think that's a coincidence? ...Think about it. | Conspiratorial. |

### PINE PASS

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| G-04 | Gravy Train | A broken-down sports car is stuck across the road, next to a cliff. | Fancy car stuck sideways on Pine Pass. Squeeze by on the drop side. Real slow. | Helpful, slowing down on "real slow". |

### THE HAIRPIN

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| G-05 | Gravy Train | A fallen pine blocks most of a tight bend. | Tree down on the Hairpin. There's a gap by the edge. ...Don't look down. | Helpful, then a wince. |

### THE BRIDGE
Foreman Buck's crew is taking the old railings off the bridge with a wrecking ball.

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| B-01 | Buck | The jeep reaches the bridge. | Buck and Sons, bridge crew. Bridge is closed, folks, we got a ball swingin'— is that a jeep? | Fast and harried. Starts official, ends high. |
| B-02 | Buck | The jeep drives onto the bridge anyway. | Why is there a jeep on my bridge? STOP THE BALL— it doesn't stop, it's a BALL! | Panic, tripping over the words. |
| B-03 | Buck | The jeep makes it across. | ...They made it. Nobody tell my insurance. | A big exhale, then a whisper. |
| B-04 | Buck | The wrecking ball knocks a squirrel off the bridge. | Oh, the ball got one. The ball got a squirrel. ...Is that covered? Is that covered? | Horrified, then worried about the paperwork. |

### THE HUMPS

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| D-12 | Dispatch | The jeep bounces over two huge humps. | Unit Seven, your GPS is going up... and down... and up. Please stop doing that. Over. | Deadpan, a little queasy. |

### ROCKY ROAD

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| B-05 | Buck | A rockslide on the steep climb. | Rockslide on Rocky Road. Yes, I know. That's why it's called that. | Tired. Buck has heard this joke before. |

### SQUIRREL SLALOM

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| N-05 | Night Owl | The jeep weaves between posts on the summit. | They named a ROAD after them. Squirrel Slalom. It's already happening. | A whisper that cracks into alarm. |

### THE BIG JUMP

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| G-06 | Gravy Train | A big ramp in the road, with room to drive round it. | Big ramp up ahead. You can go round it. ...You're not goin' round it, are you. | Amused resignation. |

### LITTLE JUMPS

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| D-13 | Dispatch | More ramps on the way down. | Unit Seven, your GPS says you were briefly... airborne. Please confirm. ...Please don't confirm. | Deadpan, then a quick retraction. |

### The fork
At the top of WIGGLY ROAD the road splits: WIGGLY ROAD, long and twisty, or BUCK'S CUT, straight down but closed.

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| D-14 | Dispatch | The crew reaches the fork. | Two roads ahead, Unit Seven. The long way round, or a road that's closed. ...You'll take the closed one. Of course you will. | Looking at a map. She already knows. |

### BUCK'S CUT (the short way)
An old ranger station blocks the closed road, and Buck's digger is parked next to it with the keys in. The crew parks the jeep, knocks the station down with the digger, and drives through.

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| B-06 | Buck | The jeep turns onto the closed road. | Road's closed, the old ranger station comes down Thursday— hey. HEY. Off my digger! | Starts bored and official, ends shouting. |
| B-07 | Buck | A squirrel climbs into the digger. | No-no-no-no, don't touch the— that's my digger, that's a WORK digger— ...oh, they're touching it. | Babbling, then deflating. |
| B-08 | Buck | The digger starts up. | Who leaves the keys in a digger? ...I leave the keys in. I always leave the keys in. | Indignant, then realising, then ashamed. |
| B-09 | Buck | The digger's arm swings towards the ranger station. | Okay, okay, if you're gonna smash the ranger station, at least use the BUCKET, not the cab! The CAB is where I eat! | Bargaining. Fast. |
| B-10 | Buck | The digger breaks down: a hose bursts and sprays oil. | That's the hydraulic hose! You gotta hold it! HOLD THE HOSE! | Shouted instructions. |
| B-11 | Buck | The ranger station comes down. | ...Huh. That's the job. That was Thursday's job. ...They did Thursday's job. | Confused, almost impressed. |
| D-15 | Dispatch | The jeep drives through the rubble. | Unit Seven, your GPS shows you inside a ranger station. ...And now outside it. ...The ranger station is gone, isn't it. | It sinks in a bit more with every pause. |
| B-12 | Buck | The crew leaves the digger in the road and drives off. | Just leave it in the road. Sure. That's fine. That's where diggers go. | Sarcastic, a bit hurt. |

### WIGGLY ROAD (the long way)
Six slow bends, and at a lookout, an Acornucopia picnic hamper full of golden acorns.

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| G-07 | Gravy Train | The crew takes the long way down. | Takin' Wiggly Road? Smart. Six bends, nice and slow. ...They're not goin' slow. | Approving, then a chuckle. |
| D-16a | Dispatch | The crew raids the picnic hamper to replace the acorns they've lost. | ...And now they've robbed a picnic. | Deadpan, writing it down. |
| D-16b | Dispatch | Same moment, another way. | That was an Acornucopia Deluxe Picnic Hamper. ...Of course it was. | Resigned. |

### COBBLE CURVE
The two roads meet again, and TINY is waiting.

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| D-17 | Dispatch | The jeep comes back onto the main road. | Both roads meet at Cobble Curve. ...So whatever you did up there, you're back on my map. | Relieved, and hiding it. |
| G-08 | Gravy Train | TINY is back and chasing the jeep. | There's a dog the size of a pony chasin' that jeep. Collar says TINY. ...Somebody honk at it! | Big, urgent, laughing. This line teaches players the horn, so make "honk" land. |

### THE DESCENT
A rocky road down with a narrow gap, TINY still chasing.

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| G-09 | Gravy Train | The road narrows between rocks. | Tight squeeze on the way down. Rocks on the mountain side. Slow and easy. ...Dog or no dog. | Calm advice, with a grin. |

### THE WASH
The last river of the day. TINY won't cross it. Yet.

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| D-18 | Dispatch | The jeep splashes through the river. | Unit Seven, why do I hear splashing? ...Never mind. I don't want to know. | Starts to ask, then gives up. |

### THE CAMPGROUND
Dusk. Nobody sleeps until the whole crew is in. Then Dispatch tries something: the players can answer with the jeep's horn, and her lines depend on how many times they honk.

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| D-19 | Dispatch | The jeep pulls into the campground. | Position logged: a campground. Stopped for the night, I assume. ...Do squirrels camp? | Half to herself. |
| G-10 | Gravy Train | The jeep is in, but one squirrel is still on its way. | Looks like they're waitin' on one more. ...C'mon, little straggler. | Fond. |
| D-20 | Dispatch | Everyone is in. Dispatch tries one last time. | Unit Seven, you've been quiet all day. ...Unit Seven, if that's you breathing, honk twice. | Slow. Half joking, half really asking. |
| D-21 | Dispatch | Somebody honks exactly twice. | ...That was a coincidence. ...Honk three times. | Very still. Suspicious. |
| D-22 | Dispatch | They honk three times. This is the end of the demo. | ...Okay. That's not Unit Seven. ...Who IS this? | The first time her professional voice slips. A real question. |
| D-23 | Dispatch | They honk some other number of times, or not at all after "honk three times". | Coincidence. Definitely a coincidence. | Too quick. Convincing herself. |
| D-24 | Dispatch | Nobody honks at all. | ...Great. I'm talking to a wrecked truck. | Deflated. |
| D-25 | Dispatch | Somebody honks five times or more. | ...That was a lot more than two. Are you okay in there? | Thrown, a bit concerned. |
| N-06 | Night Owl | The end of the demo. Dispatch has just asked the players to honk the jeep's horn, and they did. This plays over the final card. | ...She's talking to them now. Nobody else noticed? ...Night Owl, out. | An ominous whisper. |

### On the road: breakdowns
These can happen any time while driving. The jeep is held together with junk: a car battery for an engine, a sponge for a brake, a cardboard door, patched tyres.

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| D-26 | Dispatch | The battery engine catches fire. | Unit Seven, your engine's making the "bzzt" noise again. ...The bad "bzzt". | Deadpan concern. |
| G-11 | Gravy Train | The engine fire, as Gravy Train sees it. | Dispatch, that little jeep's on fire. ...One of 'em's fannin' it with its tail. | Alarmed, then charmed. |
| D-27 | Dispatch | The sponge brake gives out. | That squeal is the brakes. I know that squeal. Everybody knows that squeal. | Wincing. |
| G-12 | Gravy Train | The brakes go, as Gravy Train sees it. | Brakes are gone on that jeep. ...Is that a sponge? Their brake's a sponge. | Squinting, then laughing. |
| D-28 | Dispatch | A tyre goes flat. | Unit Seven, one of your tyres just went "pfffff". Over. | Deadpan, "pfffff" included. |
| G-13 | Gravy Train | The flat tyre, as Gravy Train sees it. | They got a flat. ...One of 'em's just holdin' the wheel on. With its arms. | Admiring. |
| D-29 | Dispatch | The cardboard door flaps open. | Unit Seven, something is flapping. Please secure your... flapping. | Searching for the word. |
| G-14 | Gravy Train | The flapping door, as Gravy Train sees it. | Door's swingin' open on that jeep. ...That door's cardboard, ain't it. | Slow realisation. |

### On the road: repairs

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| D-30 | Dispatch | The crew fixes something. | ...Huh. They fixed it. | Grudging. |
| G-15 | Gravy Train | The crew fixes something, as Gravy Train sees it. | Look at that. Fixed it with tape and a prayer. | Proud. |

### On the road: squirrels falling out

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| G-16 | Gravy Train | The orange squirrel falls out of the jeep. | The orange one's out of the jeep again. | Long-suffering. "Again" is the joke. |
| G-17 | Gravy Train | The blue squirrel falls out. | Blue one's out! ...Blue one's runnin' after it. Good for them. | Cheering it on. |
| G-18 | Gravy Train | The pink squirrel falls out. | Pink one just flew. Like, real far. | Impressed. |
| G-19 | Gravy Train | The lime-green squirrel falls out. | Lime one's off the jeep. ...It's fine. It bounced. | Reassuring. |
| G-20 | Gravy Train | The cardboard door flings a squirrel out. | That door just threw one out. ...Rude door. | Offended on the squirrel's behalf. |
| G-21 | Gravy Train | Any squirrel falls out. | One of 'em fell out again. They're fine. They bounce. | Casual. It happens all the time. |
| D-31 | Dispatch | A squirrel falls out, as Dispatch hears it. | Unit Seven, was that a scream? ...A small scream. | Deadpan. |
| G-22 | Gravy Train | A squirrel on foot gets swept away by a river. | One of 'em went for a swim! ...Current's got it. ...There it goes. | Commentating. |

### On the road: flips and resets

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| D-32 | Dispatch | The jeep flips onto its roof. | Unit Seven, your GPS says you're upside down. ...How. | Flat. |
| G-23 | Gravy Train | The jeep flips, as Gravy Train sees it. | They're on their roof! ...Now they're all pushin'. Tiny arms, big push. | Excited, then rooting for them. |
| G-24 | Gravy Train | The crew pushes the jeep back onto its wheels. | And they're back on their wheels! Atta crew! | Cheering. |
| D-33a | Dispatch | The jeep is put back on the road after getting stuck or falling off it. | They're back on the road. ...Don't ask me how. | Unbothered. |
| D-33b | Dispatch | Same moment, another way. | And they're back on the road. I've stopped asking. | Resigned. |
| N-07 | Night Owl | The jeep is suddenly back on the road after getting stuck, as Night Owl hears it. | They teleported. You all heard it. They TELEPORTED. | Vindicated, frantic. |

### On the road: the record

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| D-34a | Dispatch | The jeep reaches a new stretch of road, and Dispatch logs its position. | Logging their position. For the insurance claim. | Routine. |
| D-34b | Dispatch | Same moment, another way. | Position logged. ...Still going. | Mild surprise. |
| D-34c | Dispatch | Same moment, another way. | Noted. Filed. ...Somehow still moving. | Bemused. |
| D-34d | Dispatch | Same moment, another way. | Logging position. Again. | Bored. |
| D-35a | Dispatch | The jeep is wrecked beyond repair, and the game rewinds to the last place she logged. She pretends the crash never happened. | Let me read that back. ...No. That didn't happen. Last known position... | Calm and firm, rewriting history. |
| D-35b | Dispatch | Same moment, another way. | Correction to the log: that crash never happened. Resuming from last position. | Official. |

### On the road: air and edges

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| G-25a | Gravy Train | The jeep flies off a jump and lands it. | Did that jeep just... fly? | Awed. |
| G-25b | Gravy Train | Same moment, another way. | Woo! Get some air, little jeep! | Whooping. |
| N-08 | Night Owl | The jeep flies off a jump and lands it, as Night Owl hears it. | That jeep was in the air for two whole seconds. They're testing for flight. | Urgent whisper. |
| D-36 | Dispatch | The jeep goes off a cliff. | ...Noted. | The flattest line in the game. |
| G-26 | Gravy Train | The jeep goes off a cliff, as Gravy Train sees it. | Whoa, whoa, they're goin' over the— *kssht* ...They're okay! They're okay. | Real alarm, cut off on "the—" (static added later), then relief. |
| G-27 | Gravy Train | The jeep leaves the road and drives across the hillside. | That's not a road! That's a hill! Get back on the road! | Shouting advice. |

### On the road: the acorns

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| D-37 | Dispatch | A pile of golden acorns bounces out of the jeep. | The cargo is... leaking. Noted. | Deadpan. |
| G-28 | Gravy Train | Acorns spill, as Gravy Train sees it. | They're droppin' golden acorns all over the road! ...I'm not stoppin'. I'm not stoppin'. | Tempted, resisting. |
| N-09 | Night Owl | Acorns spill, as Night Owl sees it. | They're leaving a trail. So they can find their way back. Clever. Too clever. | Paranoid admiration. |
| D-38 | Dispatch | A squirrel sneaks a taste of the golden acorns and can't stop chewing. | Unit Seven, I'm hearing... chewing. Are you eating the cargo? Over. | Suspicious. |
| G-29 | Gravy Train | A squirrel sneaks a taste of the golden acorns and can't stop chewing, as Gravy Train sees it. | One of 'em's got its cheeks stuffed full. ...Not that they talk anyway. | Amused. |
| N-10 | Night Owl | A squirrel is caught chewing the golden acorns, as Night Owl hears it. | They're eating the evidence. | Grim certainty. |

### On the road: TINY
Whenever the dog is chasing the jeep.

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| G-30a | Gravy Train | TINY grabs a tyre in its teeth. | That dog's got your tyre! Honk at it! HONK! | Urgent. |
| G-30b | Gravy Train | Same moment, another way. | It's chewin' your wheel! Lean on that horn! | Urgent. |
| G-31 | Gravy Train | A honk scares TINY off. | Ha! Look at it go. Big dog, scared of a little honk. | Delighted. |
| N-11 | Night Owl | A honk scares TINY off, as Night Owl hears it. | They've weaponised the horn. | Horrified. |
| G-32 | Gravy Train | TINY grabs a squirrel that's on foot and flings it. | The dog's got one! ...Aaand it's thrown it. Good arm on that dog. | Sports commentator. |

### On the road: the horn

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| D-39 | Dispatch | Somebody honks the jeep's little horn for the first time. | Unit Seven, your horn sounds different. ...Smaller. | Puzzled. |

### On the road: the weather

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| D-40a | Dispatch | The crew is falling behind, and snow starts to fall early. | Weather update: flurries in the high country. Early. ...Not that anyone asked. | Pointed. |
| D-40b | Dispatch | Same moment, another way. | All units: the temperature's dropping. Snow's ahead of schedule. Over. | Official. |
| N-12 | Night Owl | The crew is falling behind, and snow starts to fall early, as Night Owl feels it. | It's getting colder. You feel that? That's not weather. That's a message. | Shivering conviction. |
| D-41 | Dispatch | The crew is well ahead of time. | ...They're making good time. I hate that I noticed. | Grudging. |

### On the road: quiet moments
When nothing has happened for a while.

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| D-42 | Dispatch | A quiet stretch. Acornucopia wants to know when its cargo will arrive. | Unit Seven, the client is asking for an ETA on the golden acorns. I've told them "rodent-dependent". Over. | Deadpan. |
| D-43 | Dispatch | A quiet stretch. | Unit Seven, Acornucopia reminds all drivers that golden acorns are "fine glazed acorns since 1962". ...They made me say that. Over. | Reading a slogan with zero enthusiasm. |
| G-33 | Gravy Train | A quiet stretch. | Breaker, Gravy Train here. Twenty years on this radio, and this is the best day I ever had. | Content. |
| G-34 | Gravy Train | A quiet stretch. | Anybody know a good diner on the mountain road? ...Anybody? ...Squirrels? | Hopeful, then joking. |
| N-13 | Night Owl | A quiet stretch. | Don't say "acorn" on an open channel. They're LISTENING. | Urgent whisper. |
| N-14 | Night Owl | A quiet stretch. | They're organised now. First the bird feeders, now the vehicles. Next? The infrastructure. | Building the case. |
| N-15 | Night Owl | A quiet stretch. | Night Owl here. Yes, it's daytime. No, I haven't slept. Neither have THEY. | Wired. |
| N-16 | Night Owl | A quiet stretch, after the crew has met Buck at the bridge. | A beaver runs the building firm, fine, that's business. But SQUIRRELS? Driving? | Outraged at everyone else. |
<!-- voice-script: end -->

## The endings
Not in the voice actor's script yet. Every ending plays these lines in this order:
1. G-901, as the jeep rolls into Maple Park.
2. D-901.
3. The result's line: D-902, D-903 or D-904.
4. B-901, once the plow stops.
5. U-901, then D-905.
6. N-901, after the credits.

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| G-901 | Gravy Train | The jeep rolls into Maple Park at dusk. | Gravy Train to that little jeep, wherever you are: ...ten-four, buddies. Ten-four. | Choked up. |
| D-901 | Dispatch | Every ending: Acornucopia demands its cargo. | The client wants to know where their golden acorns are. ...Cargo... lost in transit. Write it off. | A pause, then a choice. She's covering for them. |
| D-902 | Dispatch | On time: the first snowflake lands as the last acorn goes into the oak. | Dispatch to all units. First snow's falling. ...Somewhere out there, a crate of golden acorns made it home. Stand down. Over. | Quiet, almost warm. |
| D-903 | Dispatch | Late: the oak is snowed in, and the crew digs the door out. | Dispatch to all units. Snow's been falling since noon. ...They made it anyway. Stand down. Over. | Warm, a little proud. |
| D-904 | Dispatch | No acorns left: one golden acorn rolls out of a squirrel's cheek and is shared two hundred ways. | Dispatch to all units. The cargo is gone. All of it. ...One acorn, I'm told. Six days for one acorn. ...Good for them. Over. | Disbelief turning into respect. |
| B-901 | Buck | Buck parks the snowplow at the park gate. | Buck and Sons, closing up for winter. ...Best week of work we ever had. And we didn't do any of it. | Content. |
| U-901 | Unit Seven | At last, the truck's driver calls in. | Dispatch, this is Unit Seven. Got a new radio. ...Did I miss anything? | Cheerful, clueless. |
| D-905 | Dispatch | Her answer. | ...No. Quiet week. Over. | The driest line in the game, with a smile in it. |
| N-901 | Night Owl | After the credits. | They got home. They have a car now. ...This isn't over. | The last whisper. |

## Chapters 2–6 (drafts)
Sample lines for each chapter's key moments, to be rewritten once Chapter 1 has been built and played. Not in the voice actor's script.

### Chapter 2: The Washout

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| D-201 | Dispatch | Morning call, if anybody honked last night. | Good morning, all units. The storm's passing. First snow expected in five days. ...And yesterday was a coincidence. I've decided. | Firm denial. |
| D-202 | Dispatch | Morning call, if nobody honked last night. | Good morning, all units. First snow expected in five days. ...Unit Seven. If that's you, honk twice. ...Please. | Trying again, a little hopeful. |
| G-201 | Gravy Train | The jeep reaches the flooded farmland. | Gravy Train to every rig on this channel: the squirrel jeep is real. I'm not kiddin'. Spread the word. | Evangelical. |
| B-201 | Buck | The jeep nears Buck's sinkhole site. | Buck and Sons, sinkhole crew. Nobody touch the sand truck. ...Why did I say that out loud? | Instant regret. |
| B-202 | Buck | The crew fills the sinkhole with Buck's sand. | ...They filled my sinkhole. That was on my list. Why are they doing my list? | Baffled. |
| N-201 | Night Owl | The jeep crosses the flood. | They crossed the flood. In a jeep. They have a navy now. I'm saying it. A NAVY. | Escalating. |
| G-202 | Gravy Train | TINY swims across the flood after the jeep. | That dog just swam the whole flood after 'em. ...That's commitment. | Impressed. |
| D-203 | Dispatch | The crew raids an Acornucopia delivery box on a farmhouse porch. | An Acornucopia doorstep delivery, taken off a porch. ...That's an actual crime, Unit Seven. | Scolding, without much heat. |
| D-204 | Dispatch | The jeep reaches the barn on the hill for the night. | Position logged: a barn, on a hill. Dry, at least. ...I'm glad they're dry. Why am I glad? | Catching herself caring. |

### Chapter 3: The Old Quarry

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| D-301 | Dispatch | Morning call. | Good morning, all units. First snow in four days. ...Unit Seven, hypothetically: if someone had to cross the old quarry, there's a school bus parked by the pit. Hypothetically. | Her first tip, very casual. |
| D-302 | Dispatch | Dispatch tests them. | If anyone in that jeep can hear me: honk once for yes. | Careful, quiet. |
| D-303 | Dispatch | They honk once. | ...Okay. Okay. ...Okay. | Processing. |
| B-301 | Buck | The crew takes the sand truck again, then the school bus. | That's my sand truck again. ...And that's my bus. That's my bus too. Of course it is. | Resigned. |
| B-302 | Buck | The crew builds a sand ramp. | A ramp! They built a ramp! ...That's a good ramp, actually. | Grudging professional respect. |
| B-303 | Buck | The bus goes across the pit as a bridge. | The bus is not a bridge! The bus is for CHILDREN! | Outrage. |
| G-301 | Gravy Train | A quiet stretch. | Gravy Train here, startin' a fan club for the little jeep. Members so far: me. And my cousin. | Proud. |
| N-301 | Night Owl | The jeep crosses the bus bridge. | A ramp. And now a bus. They're building infrastructure. I SAID infrastructure. | Vindicated. |
| G-302 | Gravy Train | TINY slides down a gravel chute on its belly. | That dog came down the gravel chute on its belly. ...Gotta give it a nine. | Judging. |
| D-304 | Dispatch | The crew raids the quarry workers' lunch cooler. | The quarry crew's lunch cooler. Acornucopia snack packs. ...The quarry crew has good taste. | Approving, despite herself. |

### Chapter 4: Town Outskirts

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| D-401 | Dispatch | Morning call. | Good morning, all units. First snow in three days. ...Unit Seven, you're coming through town today. Mind the trains. | Almost friendly. |
| D-402 | Dispatch | The jeep drives past Acornucopia's headquarters, under her office window. | ...They're outside my window. ...Are they waving? | Stunned. |
| D-403 | Dispatch | Straight after. | Okay. I'm a fan. Nobody tell the client. | Giving in. |
| B-401 | Buck | The crew takes Buck's crane truck. | Crane truck's out, nobody— oh, they've got it. ...I'm not even mad. Bring it back with a full tank. | Past caring. |
| G-401 | Gravy Train | The jeep reaches the ring road, full of trucks. | All rigs on the ring road, give that little jeep a honk! | Rallying. |
| N-401 | Night Owl | The crane lifts the jeep over the rail lines. | They're in the rail network. The RAIL NETWORK. | Despair. |
| G-402 | Gravy Train | TINY rides a freight train and jumps off in front of the jeep. | That dog's ridin' a freight train. ...And it jumped off. Sure. Why not. | Giving up on logic. |
| D-404 | Dispatch | The crew raids the Acornucopia outlet store. | They robbed the Acornucopia outlet. Our own store. ...I'm logging it as a promotion. | Covering for them. |
| D-405 | Dispatch | The jeep stops at a gas station for the night. | Position logged: a gas station. ...Their jeep runs on a battery. Why are they at a gas station? | Puzzled. |

### Chapter 5: The Dam

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| D-501 | Dispatch | Morning call. | Good morning, all units. First frost last night. First snow in two days. ...You're going to make it. Probably. | Encouraging, in her way. |
| D-502 | Dispatch | The jeep reaches the spillway. | Spillway opens in ninety seconds. ...I didn't tell you that. | Quick, low, conspiratorial. |
| B-501 | Buck | The crane holds the bus across the spillway while the jeep drives over. | Okay, crane, swing left. LEFT. My left! ...Hold it there. Hold the bus! Go, go, go! | Directing at full volume. |
| B-502 | Buck | The jeep is across. | That was beautiful. Buck and Sons has never done anything that beautiful. | Moved. |
| G-501 | Gravy Train | A convoy of trucks watches from the dam road. | The whole convoy's watchin' from the dam road. ...They're cheerin'. Big rigs, cheerin'. | Emotional. |
| N-501 | Night Owl | The jeep crosses the dam. | They control the WATER now. The dam. The water. Think about it. | Terrified. |
| G-502 | Gravy Train | TINY's barks echo through the service tunnel. | Hear that echo? That's the dog. In the tunnel. ...Yep. | Spooked. |
| D-503 | Dispatch | The crew raids the visitor centre's vending machine. | The visitor centre's vending machine. ...They didn't pay. Of course they didn't pay. They're squirrels. | Exasperated, fond. |
| D-504 | Dispatch | The jeep reaches the lookout for the night. The city is visible far below. | Position logged: the lookout. You can see the city from there. ...Last night on the road, Unit Seven. | Gentle. |

### Chapter 6: First Snow

| ID | Who | When | Line | Direction |
|---|---|---|---|---|
| D-601 | Dispatch | Morning call: the snow is already falling on the pass. | All units: the snow's on the pass. It'll reach the city tonight. ...Go. Go now. | No procedure left. Urgent. |
| B-601 | Buck | Buck drives the snowplow ahead of the jeep. | This is Buck, Buck and Sons. I'm in the plow. Stay in my tracks and don't touch anything. ...Please. | Gruff, then pleading. |
| B-602 | Buck | The plow bursts through a drift. | Buck and Sons, snow clearing! Comin' through! | Triumphant. |
| G-601 | Gravy Train | The jeep follows the plow down the pass. | Proudest day of my life. Proudest. Day. | Overcome. |
| G-602 | Gravy Train | TINY's last chase through the snow. | That dog is STILL chasin' 'em. Six days! ...Respect. | Admiring. |
| D-602 | Dispatch | The jeep reaches the city. | Left at the old mill, then straight down to the park. ...Hypothetically. | Guiding them home. |
| D-603 | Dispatch | The snow is catching up with the jeep (behind time). | The snow's right behind you. ...Don't look. Just drive. | Tense. |
| N-601 | Night Owl | The jeep drives into the city. | They're going HOME. Under OUR parks. In OUR trees. | The final realisation. |
| D-604 | Dispatch | The jeep rolls through the gates of Maple Park. | ...They're home. | Barely a whisper. |
