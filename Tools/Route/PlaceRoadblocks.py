"""Place G6's roadblocks, cliff-edge pinch points and THE BRIDGE's wrecking ball in L_Route.

Run inside the editor (Python) with L_Route open:
    exec(open(r"D:/Games-Unreal/ProjectI/Tools/Route/PlaceRoadblocks.py").read())

Everything goes in the Outliner folder "Roadblocks", which a rerun deletes and rebuilds, so nothing else
(the baked route, hand edits elsewhere) is touched. Move things by hand afterwards if you like, but a rerun
puts them back: change the tables below instead. Every prop gets a "Ground=<cm>" tag, so
Tools/Route/SnapToGround.py keeps it on the landscape after sculpting.

At each spot the script measures the trail (where its paint fades out) and the ground 12 m out on both
sides: the lower side is the drop. A block covers the trail from the uphill (wall) edge and leaves GAP cm
open along the drop: slow and careful gets through, rushing clips the block (crash damage) or goes over the
edge (Hold R, -3 acorns).
"""
import math
import random
import unreal

eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
FOLDER = "Roadblocks"

# A roadblock leaves a car width (~2.4 m) + 1 m; a pinch point a bit more.
ROADBLOCK_GAP = 350.0
PINCH_GAP = 450.0

# (section, kind): each goes where its section has the biggest drop beside the trail.
ROADBLOCKS = [
    ("PINE PASS", "car"),
    ("THE HAIRPIN", "tree"),
    ("ROCKY ROAD", "rockslide"),
]
# The drop is measured this far out from the centreline: the ground blends from the trail to the mountain
# over ~15 m, so a cliff only shows beyond that (L_Route, 20 Oct: 54 m at THE HAIRPIN, 25 m in PINE PASS).
DROP_REACH = 2000.0
PINCH_COUNT = 3
PINCH_MIN_DROP = 1500.0     # cm
PINCH_SPACING = 40000.0     # cm between pinch points and from roadblocks
# No pinch points where the trail has a job of its own (the tutorial, fords, the bridge, jumps, the depot).
PINCH_SKIP = ("THE PARK", "SQUIRREL CREEK", "THE BRIDGE", "THE BIG JUMP", "LITTLE JUMPS", "THE WASH", "THE DEPOT", "SQUIRREL SLALOM")

BALL_SECTION = "THE BRIDGE"
BALL_METRES = 140.0          # into the section: the middle of the deck (40 m approach + 200 m bridge)

NATURE = "/Game/ThirdParty/Kenney/Nature/%s/StaticMeshes/SM_%s.SM_%s"
def mesh(name):
    return unreal.load_asset(NATURE % (name, name, name))
ROCKS = [mesh("rock_largeA"), mesh("rock_largeB"), mesh("rock_tallA")]
LOG = mesh("log")
STUMP = mesh("stump_round")
PINE = mesh("tree_pineTallA")
SPORTS_CAR = unreal.load_asset("/Game/Vehicles/SportsCar/SM_SportsCar.SM_SportsCar")
SPORTS_WHEEL = unreal.load_asset("/Game/Vehicles/SportsCar/SM_SportsCar_Wheel.SM_SportsCar_Wheel")

random.seed(6)
log_lines = []
def log(msg):
    log_lines.append(msg)
    unreal.log("ROADBLOCKS " + msg)

routes = [a for a in eas.get_all_level_actors() if isinstance(a, unreal.NBRoute)]
if not routes:
    raise RuntimeError("No NBRoute in this level: open L_Route first")
route = routes[0]
world = unreal.EditorLevelLibrary.get_editor_world()

for actor in eas.get_all_level_actors():
    if str(actor.get_folder_path()).startswith(FOLDER):
        eas.destroy_actor(actor)

sections = {route.get_section_name(i): (route.get_section_start(i), route.get_section_start(i + 1) if i + 1 < route.get_num_sections() else route.get_length())
            for i in range(route.get_num_sections())}


def ground(x, y):
    # UE returns (found, z) or, when the bool return gets folded away, z alone (None if not found).
    result = route.get_landscape_height(x, y)
    found, z = result if isinstance(result, tuple) else (result is not None, result)
    return z if found else route.get_ground_height(x, y)


def frame(d):
    """Centre, forward and right (unit, flat) of the trail at distance d (cm)."""
    t = route.get_transform_at(d, 0.0)
    rot = t.rotation.rotator()
    yaw = math.radians(rot.yaw)
    fwd = unreal.Vector(math.cos(yaw), math.sin(yaw), 0.0)
    right = unreal.Vector(-math.sin(yaw), math.cos(yaw), 0.0)
    return t.translation, fwd, right, rot.yaw


def at(centre, right, lateral, fwd=None, along=0.0):
    p = centre + right * lateral
    if fwd is not None:
        p = p + fwd * along
    return p


def edge(centre, right, side):
    """How far the trail paint reaches to one side (cm), in 25 cm steps."""
    lateral = 0.0
    while lateral < 1500.0:
        p = at(centre, right, side * (lateral + 25.0))
        if route.get_trail_weight(p.x, p.y) < 0.5:
            break
        lateral += 25.0
    return lateral


def measure(d):
    """(drop side +1/-1, its drop in cm, left edge, right edge) at distance d."""
    centre, fwd, right, yaw = frame(d)
    drops = {}
    for s in (1, -1):
        p = at(centre, right, s * DROP_REACH)
        drops[s] = centre.z - ground(p.x, p.y)
    side = 1 if drops[1] >= drops[-1] else -1
    return side, drops[side], edge(centre, right, -1), edge(centre, right, 1)


def best_spot(start, end):
    best = None
    d = start
    while d <= end:
        side, drop, _, _ = measure(d)
        if best is None or drop > best[1]:
            best = (d, drop, side)
        d += 500.0
    return best


def add_prop(static_mesh, location, rotation, scale, block, label, at_pivot=False):
    z = ground(location.x, location.y)
    prop = eas.spawn_actor_from_class(unreal.NBProp, unreal.Vector(location.x, location.y, z), rotation)
    prop.set_actor_scale3d(unreal.Vector(scale, scale, scale) if isinstance(scale, (int, float)) else scale)
    prop.set_prop(static_mesh, block, at_pivot)
    prop.set_actor_label(label)
    prop.set_folder_path(FOLDER)
    prop.set_editor_property("tags", [unreal.Name("Ground=0.0")])
    return prop


def size_of(static_mesh):
    box = static_mesh.get_bounding_box()
    return box.max - box.min


def fill_rocks(d, side, lo, hi, label, mixed):
    """Rocks (and, mixed, logs and stumps) across lateral [lo, hi] (cm, + towards the drop) at distance d."""
    centre, fwd, right, yaw = frame(d)
    lateral = lo
    n = 0
    while lateral < hi:
        size = random.uniform(160.0, 260.0)
        lateral_mid = min(lateral + size * 0.5, hi - size * 0.4)
        choice = random.random()
        if mixed and choice < 0.25:
            m, block = LOG, 0.85
        elif mixed and choice < 0.4:
            m, block = STUMP, 0.7
        else:
            m, block = random.choice(ROCKS), 0.75
        extent = size_of(m)
        scale = size / max(extent.x, extent.y, 1.0)
        p = at(centre, right, side * lateral_mid, fwd, random.uniform(-120.0, 120.0))
        add_prop(m, p, unreal.Rotator(roll=0.0, pitch=0.0, yaw=random.uniform(0.0, 360.0)), scale, block, "%s_%d" % (label, n))
        lateral += size * 0.85
        n += 1
    return n


placed = []  # distances (cm) of everything placed, for spacing the pinch points

# --- Roadblocks ---
for section, kind in ROADBLOCKS:
    if section not in sections:
        log("no section %s, skipped" % section)
        continue
    s0, s1 = sections[section]
    # Not the first or last 15 m of the section, so a roadblock never sits on a section sign.
    d, drop, side = best_spot(s0 + 1500.0, s1 - 1500.0)
    _, _, left, right_edge = measure(d)
    drop_edge, wall_edge = (right_edge, left) if side == 1 else (left, right_edge)
    # Lateral coordinates below are + towards the drop: the block runs from the wall edge to GAP short of the drop edge.
    lo, hi = -wall_edge, drop_edge - ROADBLOCK_GAP
    centre, fwd, right, yaw = frame(d)
    name = section.title().replace(" ", "")
    if kind == "car":
        # A broken-down sports car parked across the trail, one wheel off and lying beside it.
        extent = size_of(SPORTS_CAR)
        car_len = max(extent.x, extent.y)
        mid = hi - car_len * 0.5
        add_prop(SPORTS_CAR, at(centre, right, side * mid), unreal.Rotator(roll=0.0, pitch=0.0, yaw=yaw + 90.0 * side + 8.0), 1.0, 0.9, name + "_BrokenCar")
        add_prop(SPORTS_WHEEL, at(centre, right, side * (mid - 80.0), fwd, -260.0), unreal.Rotator(roll=80.0, pitch=0.0, yaw=yaw + 30.0), 1.0, 0.0, name + "_LostWheel")
        n = fill_rocks(d, side, lo, mid - car_len * 0.5, name + "_Rock", False)
        log("%s: broken-down car at %.0f m, drop %.0f cm on the %s, gap %.0f cm (+%d rocks)" % (section, d / 100, drop, "right" if side == 1 else "left", ROADBLOCK_GAP, n))
    elif kind == "tree":
        # A pine fallen across the trail from the wall side, its stump left standing at the edge.
        extent = size_of(PINE)
        length = hi - lo + 150.0
        scale = length / max(extent.z, 1.0)
        base = at(centre, right, side * (lo - 100.0))
        # Pitch 90 lays the tree's height (Z) along the actor's X; yaw points X across the trail towards the drop.
        add_prop(PINE, base, unreal.Rotator(roll=0.0, pitch=-90.0, yaw=yaw + 90.0 * side), scale, 0.25, name + "_FallenTree", True)
        stump = size_of(STUMP)
        add_prop(STUMP, base, unreal.Rotator(roll=0.0, pitch=0.0, yaw=yaw), 120.0 / max(stump.x, 1.0), 0.7, name + "_Stump")
        log("%s: fallen tree at %.0f m, %.0f cm long, drop %.0f cm, gap %.0f cm" % (section, d / 100, length, drop, ROADBLOCK_GAP))
    else:
        n = fill_rocks(d, side, lo, hi, name + "_Rockslide", False)
        n += fill_rocks(d + 250.0, side, lo, hi - 150.0, name + "_Rockslide2", False)
        log("%s: rockslide of %d rocks at %.0f m, drop %.0f cm, gap %.0f cm" % (section, n, d / 100, drop, ROADBLOCK_GAP))
    placed.append(d)

# --- Pinch points: the biggest drops elsewhere, rocks and logs on the wall side ---
candidates = []
d = 30000.0
while d < route.get_length() - 30000.0:
    name = route.get_section_name(route.get_section_at(d))
    if name not in PINCH_SKIP:
        side, drop, left, right_edge = measure(d)
        if drop >= PINCH_MIN_DROP:
            candidates.append((drop, d, side, left, right_edge))
    d += 2000.0
candidates.sort(reverse=True)
pinches = 0
for drop, d, side, left, right_edge in candidates:
    if pinches >= PINCH_COUNT:
        break
    if any(abs(d - p) < PINCH_SPACING for p in placed):
        continue
    drop_edge, wall_edge = (right_edge, left) if side == 1 else (left, right_edge)
    n = fill_rocks(d, side, -wall_edge, drop_edge - PINCH_GAP, "Pinch%d" % (pinches + 1), True)
    log("pinch %d at %.0f m (%s): drop %.0f cm, %d rocks/logs, gap %.0f cm" % (pinches + 1, d / 100, route.get_section_name(route.get_section_at(d)), drop, n, PINCH_GAP))
    placed.append(d)
    pinches += 1

# --- THE BRIDGE's wrecking ball, on the deck (traced: the deck stands above the profile) ---
if BALL_SECTION in sections:
    d = sections[BALL_SECTION][0] + BALL_METRES * 100.0
    centre, fwd, right, yaw = frame(d)
    hit = unreal.SystemLibrary.line_trace_single(world, centre + unreal.Vector(0, 0, 3000), centre - unreal.Vector(0, 0, 3000),
                                                 unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, False, [], unreal.DrawDebugTrace.NONE, True)
    hit = hit[-1] if isinstance(hit, tuple) else hit
    deck = hit.to_tuple()[5].z if hit and hit.to_tuple()[0] else centre.z
    ball = eas.spawn_actor_from_class(unreal.NBWreckingBall, unreal.Vector(centre.x, centre.y, deck), unreal.Rotator(roll=0.0, pitch=0.0, yaw=yaw))
    ball.set_actor_label("Bridge_WreckingBall")
    ball.set_folder_path(FOLDER)
    log("wrecking ball at %.0f m on %s, deck z %.0f (profile %.0f)" % (d / 100, BALL_SECTION, deck, centre.z))

les.save_current_level()
log("done: %d roadblocks, %d pinch points" % (len(ROADBLOCKS), pinches))
