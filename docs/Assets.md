# Assets: sources, licences and where files go

Living doc, last updated 2026-10-12. Merged from the two research notes of 2026-10-09. Nothing has been downloaded or committed yet. What gets built when: [Roadmap.md](Roadmap.md).

## The rule: the repo is public
`AgIsh0203/ProjectI` is public on GitHub, so every licence has to answer two questions: can it ship in the itch build, and can it be committed?

| Licence | Examples | Ship in the build | Commit to the repo | Credit |
|---|---|---|---|---|
| **CC0** | Kenney, Quaternius, KayKit, Poly Haven, ambientCG, FreePD | Yes | Yes | Not required (we credit anyway) |
| **CC-BY** | some Poly Pizza and Freesound files, Kevin MacLeod, game-icons.net | Yes | Yes | **Required**, in the game and on the itch page |
| **OFL / Apache 2.0** (fonts) | Google Fonts | Yes | Yes, with the font's licence file | Keep the licence file |
| **Fab Standard License** | all free Fab content, including Epic's monthly free items | Yes | **No.** The Fab EULA bans standalone redistribution; sharing is only allowed with collaborators (e.g. a private repo). | No |
| **Sonniss GDC bundle, Pixabay** | | Yes | **No**, same no-standalone-redistribution rule | No |
| **Non-commercial (NC)** | some Freesound files and itch packs | **No**: it would block the later Steam release | No | — |
| **UE EULA** (Epic engine and template content) | the Vehicle template's offroad buggy | Yes | Already committed, by your decision | The engine notice ([CREDITS.md](../CREDITS.md)) |

### Where files go
- `Content/ThirdParty/<Source>/`: CC0, CC-BY and font files. Committed (LFS).
- `Content/ThirdParty_Private/<Source>/`: Fab, Sonniss and Pixabay files. `.gitignore` excludes it, so they live only on the dev PC and in packaged builds.

What follows from that:
- **Import only the files you use.** Smaller repo, smaller LFS bill, faster cooks.
- **Commit the imported `.uasset` files, not the raw downloads.** (`.gitignore` drops `*.obj` anyway, since it's also a compiler output.)
- **Keep the download small** (the demo budget is ≤ 200 MB zipped; [Roadmap.md](Roadmap.md#package-size-slim-it-before-the-playtest)). Cap textures at 1024 px; Kenney's are far smaller. Music loops are the biggest audio files, so keep each under ~3 min. Check the zip size after each import batch.
- **Credit on import.** Add the `CREDITS.md` line the day a file is imported, CC0 included, so the itch page can thank everyone.
- **Cooking.** Assets assigned in a cooked Blueprint cook with it. BP_OffroadCar_Pawn and BP_Squirrel live in always-cooked directories (`/Game/VehicleTemplate/Blueprints/OffroadCar`, `/Game/Blueprints`), so their slots are covered. Anything else only C++ constructors reference (a HUD font, an imported mesh) must sit in a directory listed under `DirectoriesToAlwaysCook` in `Config/DefaultGame.ini`. `/Game/ThirdParty` is listed (since the font, 13 Oct); add `/Game/ThirdParty_Private` when its first file lands.
- **Public clones miss the private files.** Blueprints that point at them load with empty slots. `FNBFeedback` does nothing without assets, so the game still runs, just quieter. The second PC gets everything through the packaged build, not git.
- **Fallback:** if the private folder becomes a hassle, go CC0-only. Kenney, Freesound CC0 and jsfxr cover every slot, at lower quality.

## Shopping list
In priority order. Items 1–5 are for the friend test (download by Fri 16 Oct); 6–8 only if there's time.

| # | Get | Licence → folder | For |
|---|---|---|---|
| 1 | Kenney **Impact Sounds** and **Interface Sounds** | CC0 → `ThirdParty/Kenney` | crashes, acorn pops, repair dings, UI, countdown |
| 2 | **Sonniss GDC Game Audio Bundle**: pick files, don't import it all | royalty-free → `ThirdParty_Private/Sonniss` | the engine loop (a rattly lawnmower or moped), metal clangs, fire crackle, tire squeal |
| 3 | **Freesound**, filtered to CC0: squirrel chatter and squeaks, a scream, a cartoon boing, a silly horn | CC0 → `ThirdParty/Freesound` (CC-BY only with a credit line) | squirrel barks, horn, righted car |
| 4 | **Music**: two CC0 loops (menu, run) from FreePD, Kenney or OpenGameArt | CC0 → `ThirdParty/Music` | menu and run |
| 5 | A chunky **font** (Lilita One: imported 13 Oct) and Kenney **Input Prompts** | OFL / CC0 → `ThirdParty/Fonts`, `ThirdParty/Kenney` | failure callouts, HUD, controls hints |
| 6 | Kenney **Particle Pack** | CC0 → `ThirdParty/Kenney` | cartoon smoke, stars and sparks as Niagara sprites |
| 7 | Kenney **Nature Kit** and **City Kit (Roads)** | CC0 → `ThirdParty/Kenney` | route and depot dressing (first thing cut) |
| 8 | Quaternius **LowPoly Animated Animals** (the pug) | CC0 → `ThirdParty/Quaternius` | the dog hazard (Roadmap #13), only if it's picked |

## Slot map
Which file goes in which slot. Every slot is an `FNBFeedback` (a sound plus a Niagara effect; each play sets an `Intensity` 0–1 parameter on both) unless noted. "New" slots don't exist yet; the Roadmap item adds them.

| Slot | Where | Suggestion | Source |
|---|---|---|---|
| `EngineSound` (a plain sound) | BP_OffroadCar_Pawn, `EngineAudio` | One cheap, rattly loop. Pitch already follows RPM. A lawnmower or moped is funnier than a real car. | Sonniss |
| `CrashFeedback` | car | Metal crunch or wood thump, louder with Intensity; sparks | Sonniss, Kenney Impact |
| `SpillFeedback` | car | A few short wooden pops ("impactWood"), more for a bigger spill | Kenney Impact, jsfxr |
| `FlippedFeedback` | car | A cartoon thud and a dust puff | Kenney Impact, Freesound CC0 |
| `RightedFeedback` | car | Boing | Freesound CC0, jsfxr |
| `BreakFeedback` | each part | Engine and brakes: a metal clang. Tire: pop and hiss. Door: a creak. | Sonniss |
| `FailedLoopFeedback` | Engine | Fire crackle loop and a fire/smoke effect; its `Severity` parameter ramps 0→1 until the stall | Sonniss, Niagara |
| `FailedLoopFeedback` | Brakes, Tire, Door | Optional: a squeal, flapping rubber, a creak | Sonniss |
| `RepairFeedback` | each part | A confirmation ding and a sparkle | Kenney Interface |
| `HornFeedback` | car | A squeaky bicycle or clown horn | Freesound CC0 |
| `ScreamFeedback`, `LandFeedback`, `GrabFeedback` | BP_Squirrel (`/Game/Blueprints`, always cooked) | A scream when thrown out, a thud on landing (Intensity = impact speed), a chatter when grabbing a part | Freesound CC0 |
| New run events | game mode / HUD | Countdown beeps, a delivery fanfare, a total-wreck explosion | Kenney Interface / Digital Audio |
| Music | menu, run | Upbeat, goofy, loopable | FreePD, Kenney |

## Sources

### 3D models
| Source | Licence | Useful for |
|---|---|---|
| [Kenney](https://kenney.nl/assets) | CC0 | **Car Kit** (bolt-on hood and door panels, crates for the bed, parked cars), **Racing Kit**, **City Kit (Roads)** (road tiles, signs, cones, barriers), **Nature Kit** (trees, rocks, logs, fences, bridges), **Furniture** and **Survival** kits (crates and barrels for the depot), **Food Kit** (maybe an acorn), **Platformer Kit**. Also on [itch](https://kenney-assets.itch.io/car-kit). |
| [Quaternius](https://quaternius.com) | CC0 | **Ultimate Nature**, **Stylized Nature MegaKit**, [LowPoly Cars](https://quaternius.itch.io/lowpoly-cars), [LowPoly Animated Animals](https://quaternius.itch.io/lowpoly-animated-animals) (pug, cow, pig, horse, llama; no squirrel). |
| [KayKit](https://kaylousberg.itch.io) | CC0 | City Builder, Forest Nature. A chunky style that fits Kenney. |
| [Poly Pizza](https://poly.pizza) | CC0 or CC-BY, per model | Search hub that includes Kenney and Quaternius. About [9 static low-poly squirrels](https://poly.pizza/search/squirrel) and some acorns. |
| [OpenGameArt](https://opengameart.org) | Mixed, per file | Fallback search; a few squirrels of varying quality. |
| [Fab](https://www.fab.com) free listings and Epic's monthly free content | Fab Standard License (private folder) | Higher fidelity than the Kenney style, so mostly sound packs, materials and Niagara effects. Claim the monthly free items even if unused. |

Kenney and Quaternius share a flat-colour low-poly style, so they mix well.

### The squirrel
No free, rigged, CC0 squirrel exists. Poly Pizza's are static. The [polyducks one](https://polyducks.itch.io/3d-model-squirrel) costs $5, has no rig and can't be redistributed raw. Quaternius and Omabuarts' Quirky Series have no squirrel.

**Plan for the demo:** keep the squirrel built from primitives in C++ and make it funny rather than realistic (Roadmap #7). There's no rig to fight, the fur colours keep working, there's no licence risk, and a stiff, wobbly primitive ragdolls funnier than a realistic one. A Poly Pizza CC0 mesh could replace `BodyVisual` later.

### Textures, materials and skies
| Source | Licence | Useful for |
|---|---|---|
| [Poly Haven](https://polyhaven.com) | CC0 | HDRI skies; ground, asphalt, cobble and wood textures |
| [ambientCG](https://ambientcg.com) | CC0 | The same, with a bigger material library |

### Sound effects
| Source | Licence | Useful for |
|---|---|---|
| [Kenney audio packs](https://kenney.nl/assets?q=audio) | CC0 | Impact Sounds, Interface Sounds, UI Audio, Digital Audio, Casino, Voiceover |
| [Sonniss GDC Game Audio Bundle](https://sonniss.com/gameaudiogdc) | Royalty-free, commercial use, no credit (private folder) | Multi-GB yearly bundles of pro SFX: engine loops, impacts, clangs, fire, tire squeal |
| [Freesound](https://freesound.org) | Per file: filter to CC0; CC-BY with a credit line; skip NC | Squirrel chatter, cartoon boings, horns, rattles |
| [Pixabay sound effects](https://pixabay.com/sound-effects/) | Pixabay licence: free, commercial use, no credit (private folder) | Quick grabs: horns, boings, screams |
| [lmglolo free car SFX](https://lmglolo.itch.io/free-car-sfx), [indiesfx car sounds](https://indiesfx.itch.io/indiesfx-s017c) | Check each page | Small car packs |
| [jsfxr](https://sfxr.me) / ChipTone | You own the output | Pings, acorn pops and repair dings in seconds |

### Music
Music must be safe on Twitch and YouTube. CC0 and CC-BY tracks are, unless someone registered them with Content ID.

| Source | Licence | Note |
|---|---|---|
| [FreePD](https://freepd.com) | CC0 | Includes Kevin MacLeod's public-domain tracks |
| [Kevin MacLeod / incompetech](https://incompetech.com) | CC BY 4.0 | Lots of goofy chase and comedy tracks ("Monkeys Spinning Monkeys", "Run Amok"). Credit required. |
| Kenney music loops | CC0 | |
| [OpenGameArt CC0 loops](https://opengameart.org/content/short-loops-background-music-pack) | CC0 | |
| Pixabay music | Pixabay licence | **Skip tracks with the Content ID shield**: a claim on a streamer's clip works against "viewer fun first". |

### VFX
`FNBFeedback` takes Niagara systems only. Starter Content's fire, smoke and sparks are legacy Cascade effects, so they don't fit the slots (and Starter Content isn't in the project).

| Source | Licence | Useful for |
|---|---|---|
| Niagara's built-in emitter templates | Part of the engine | Fire, smoke, sparks, bursts and confetti made from templates. The systems we make are ours to commit. |
| Kenney **Particle Pack** | CC0 | Cartoon smoke puffs, stars and sparks as sprite textures for those systems |
| Epic's Niagara examples on Fab (check there's a 5.7 version) | Fab licence (private folder) | Ready-made fire, smoke, sparks, explosions, markers, fireworks |

### Fonts and UI
| Source | Licence | Useful for |
|---|---|---|
| [Google Fonts](https://fonts.google.com): **Lilita One**, **Fredoka**, **Bangers** (OFL); **Luckiest Guy** (Apache 2.0) | OFL / Apache 2.0 | Chunky cartoon faces that read at stream resolution. Commit the licence file with the font. |
| [Kenney Fonts](https://kenney.nl/assets/kenney-fonts), Kenney **UI Pack**, Kenney **Input Prompts** | CC0 | Fonts, buttons and panels, keyboard and gamepad glyphs for the controls hint and pings |
| [game-icons.net](https://game-icons.net) | CC BY 3.0 | 4000+ icons (wrench, fire, tire). Credit required. |
