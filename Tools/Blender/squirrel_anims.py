# The squirrel's animations, written as functions of the phase t (0..1), so loops close exactly.
# Rotations are degrees about the *armature's* axes as they are in the rest pose
# (x forward, y left, z up). Each bone's rotation is relative to its parent, so a pose reads like FK:
#   y on a bone pointing down (upper arm, calf): + swings it back, - swings it forward;
#   y on a bone pointing forward (forearm, paw, head, thigh): + tips it down, - tips it up;
#   y on the tail: + brings it forward over the back;
#   x on a left limb: + swings it outward (away from the body).
# Keys named *_l are mirrored onto *_r unless the *_r key is given: (x, y, z) -> (-x, y, -z),
# and side_phase shifts the right side's phase (0.5 = alternating).
# The paws rest just under the muzzle (the model holds a nut), so any pose that raises the arms
# also splays them outward or bends the forearms down; check_clipping.py measures it.
# Used by rig_squirrel.py; the bone names are the shared squirrel skeleton.
import math

import bpy
from mathutils import Euler, Quaternion, Vector

FPS = 24
TAU = 2 * math.pi
TAIL = ["tail_01", "tail_02", "tail_03", "tail_04"]


def s(t, phase=0.0, harmonic=1):
    return math.sin(TAU * (harmonic * t + phase))


def keyed(t, keys):
    """Eases between poses: keys = [(t, {bone: (x, y, z)}), ...], sorted, t from 0 to 1."""
    for (t0, a), (t1, b) in zip(keys, keys[1:]):
        if t0 <= t <= t1:
            u = (t - t0) / (t1 - t0) if t1 > t0 else 1.0
            u = u * u * (3 - 2 * u)
            out = {}
            for bone in set(a) | set(b):
                va, vb = a.get(bone, (0, 0, 0)), b.get(bone, (0, 0, 0))
                out[bone] = tuple(x + (y - x) * u for x, y in zip(va, vb))
            return out
    return dict(keys[-1][1])


def idle(t, side):
    p = 0.0 if side == "l" else 0.1
    r = {
        "spine_02": (0, 2.5 * s(t), 0),
        "neck": (0, -1.5 * s(t), 0),
        "head": (3 * s(t, 0.3), -3 + 2 * s(t, 0.1, 2), 8 * s(t, 0.2)),
        "upperarm_l": (0, 5 + 2 * s(t, p), 0),
        "lowerarm_l": (0, 3 + 3 * s(t, p + 0.1), 0),
    }
    for i, b in enumerate(TAIL):
        r[b] = (0, 3 * s(t, -0.1 * i), 7 * s(t, -0.12 * i))
    return r, {}


def run(t, side):
    # A bound: front paws together, back paws together, half a cycle apart.
    r = {
        "pelvis": (0, 50 + 8 * s(t), 0),
        "spine_02": (0, 5 + 8 * s(t, 0.25), 0),
        "neck": (0, -25, 0),
        "head": (0, -20 - 6 * s(t, 0.25), 0),
        "upperarm_l": (15, 5 + 40 * s(t, 0.5), 0),
        "lowerarm_l": (0, 30 + 20 * s(t, 0.75), 0),
        "hand_l": (0, 25 * s(t, 0.6), 0),
        "thigh_l": (5, -50 + 30 * s(t), 0),
        "calf_l": (0, 12 * s(t, 0.25), 0),
        "foot_l": (0, -15 + 30 * s(t, 0.1), 0),
    }
    for i, b in enumerate(TAIL):
        r[b] = (0, (-25 if i == 0 else 0) + 8 * s(t, -0.15 * i - 0.2), 4 * s(t, -0.15 * i))
    return r, {"pelvis": (0, 0, 0.03 + 0.04 * max(0.0, s(t, 0.1)))}


def flail(t, side):
    # Thrown out of the car: arms windmill out to the sides, legs kick, the tail whips.
    p = 0.0 if side == "l" else 0.37
    r = {
        "spine_02": (6 * s(t, 0, 2), -18 + 10 * s(t, 0.2, 2), 0),
        "neck": (0, -10, 0),
        "head": (12 * s(t, 0.1, 2), -8, 25 * s(t, 0.3, 2)),
        "upperarm_l": (80 + 25 * s(t, p, 2), 45 * s(t, p), 0),
        "lowerarm_l": (0, 25 + 35 * s(t, p + 0.2, 2), 0),
        "hand_l": (0, 30 * s(t, p + 0.4, 2), 0),
        "thigh_l": (35 + 20 * s(t, p + 0.5, 2), -30 + 45 * s(t, p + 0.25), 0),
        "calf_l": (0, 35 * s(t, p, 2), 0),
        "foot_l": (0, 30 * s(t, p + 0.3, 2), 0),
    }
    for i, b in enumerate(TAIL):
        r[b] = (12 * s(t, -0.12 * i, 2), (-12 if i == 0 else 0) + 10 * s(t, -0.1 * i), 28 * s(t, -0.15 * i))
    return r, {}


def grip(t, side):
    # Seated, hunched over the wheel (or the seat edge): paws reach forward and hook down over
    # the rim, the body shudders with the car, and the paws see-saw like steering.
    steer = s(t, 0.0)  # mirrored phase below makes the paws alternate
    r = {
        "pelvis": (0, 14, 0),
        "spine_02": (0, 10 + 1.5 * s(t, 0, 6), 0),
        "neck": (0, -14, 0),
        "head": (2 * s(t, 0.3), -12 + 2 * s(t, 0.1, 6), 6 * s(t, 0.2)),
        "upperarm_l": (28, -35 + 10 * steer, 0),
        "lowerarm_l": (0, 40 - 6 * steer, 0),
        "hand_l": (0, 55, 0),
        "thigh_l": (0, -10, 0),
    }
    for i, b in enumerate(TAIL):
        r[b] = (0, 2 * s(t, -0.1 * i), 5 * s(t, -0.12 * i))
    return r, {}


def mash(t, side):
    # Slamming the door / hammering a part: paws go up beside the head in turn and pound down
    # in front of the chest; the body bobs with each blow.
    up = s(t)  # -1 = raised, +1 = down
    r = {
        "spine_02": (0, 12 + 8 * s(t, 0, 2), 0),
        "neck": (0, -10, 0),
        "head": (0, -10 - 4 * s(t, 0.1, 2), 0),
        "upperarm_l": (40 - 10 * up, -55 + 45 * up, 0),
        "lowerarm_l": (0, 30 + 20 * up, 0),
        "hand_l": (0, 20 * up, 0),
    }
    for i, b in enumerate(TAIL):
        r[b] = (0, 5 * s(t, -0.1 * i, 2), 6 * s(t, -0.12 * i))
    return r, {}


def tug(t, side):
    # Clinging to a wheel or heaving the flipped car: leaning back, arms straight out front, pulling in heaves.
    r = {
        "pelvis": (0, -12 + 6 * s(t), 0),
        "spine_02": (0, -8 + 6 * s(t, 0.1), 0),
        "neck": (0, 8, 0),
        "head": (0, -8 + 4 * s(t, 0.2), 4 * s(t, 0.3)),
        "upperarm_l": (35, -60 + 8 * s(t, 0.5), 0),
        "lowerarm_l": (0, 60 - 8 * s(t, 0.5), 0),
        "hand_l": (0, 30, 0),
        "thigh_l": (0, -15 + 5 * s(t), 0),
    }
    for i, b in enumerate(TAIL):
        r[b] = (0, -6 + 4 * s(t, -0.1 * i), 5 * s(t, -0.12 * i))
    return r, {}


def tail_fan(t, side):
    # Putting out the engine fire: the squirrel turns its back to the flames, looks over its
    # shoulder and beats its tail down at them like a fan: a fast down-stroke, a slower lift,
    # whipping from the base to the tip. The body bounces with each beat; paws on hips.
    u = t % 1.0
    beat = math.sin(math.pi * u / 0.4) if u < 0.4 else -math.sin(math.pi * (u - 0.4) / 0.6)  # + = down-stroke
    r = {
        "pelvis": (0, -4 * beat, 180),
        "spine_02": (0, 6 + 4 * beat, 0),
        "neck": (0, -6, 40),
        "head": (0, -8, 35 + 4 * s(t, 0.2)),
        "upperarm_l": (40, 20, 0),
        "lowerarm_l": (0, 50, 0),
        "hand_l": (0, 20, 0),
    }
    for i, b in enumerate(TAIL):
        lag = 0.07 * i
        ub = (u - lag) % 1.0
        bb = math.sin(math.pi * ub / 0.4) if ub < 0.4 else -math.sin(math.pi * (ub - 0.4) / 0.6)
        bb = bb if bb > 0 else 0.35 * bb  # a short lift, so the tail never folds over the head
        r[b] = (0, -(28 if i == 0 else 12) * bb - (10 if i == 0 else 0), 0)
    return r, {"pelvis": (0, 0, 0.015 * max(0.0, beat))}


HOP_KEYS = [
    # Seat to seat: crouch, spring (body stretched, paws reaching), tuck in the air,
    # reach down for the landing, settle back into the sitting pose.
    (0.0, {}),
    (0.15, {"pelvis": (0, 12, 0), "spine_02": (0, 15, 0), "neck": (0, -12, 0), "head": (0, -10, 0),
            "thigh_l": (0, 15, 0), "calf_l": (0, -10, 0), "upperarm_l": (15, 20, 0), "lowerarm_l": (0, 20, 0),
            "tail_01": (0, 15, 0), "tail_02": (0, 10, 0)}),
    (0.35, {"pelvis": (0, 30, 0), "spine_02": (0, -10, 0), "neck": (0, -10, 0), "head": (0, -15, 0),
            "thigh_l": (0, 70, 0), "calf_l": (0, -40, 0), "foot_l": (0, 40, 0),
            "upperarm_l": (30, -70, 0), "lowerarm_l": (0, 70, 0), "hand_l": (0, 20, 0),
            "tail_01": (0, -35, 0), "tail_02": (0, -15, 0), "tail_03": (0, -10, 0), "tail_04": (0, -10, 0)}),
    (0.6, {"pelvis": (0, 10, 0), "spine_02": (0, 10, 0), "neck": (0, -10, 0), "head": (0, -10, 0),
           "thigh_l": (0, -25, 0), "calf_l": (0, 15, 0), "upperarm_l": (45, -20, 0), "lowerarm_l": (0, 40, 0),
           "tail_01": (0, -15, 0), "tail_02": (0, 15, 0), "tail_03": (0, 15, 0), "tail_04": (0, 10, 0)}),
    (0.82, {"pelvis": (0, 15, 0), "spine_02": (0, 12, 0), "neck": (0, -15, 0), "head": (0, -10, 0),
            "thigh_l": (5, -15, 0), "calf_l": (0, -10, 0), "foot_l": (0, -15, 0),
            "upperarm_l": (25, -30, 0), "lowerarm_l": (0, 45, 0), "hand_l": (0, 30, 0),
            "tail_01": (0, 10, 0), "tail_02": (0, 5, 0)}),
    (1.0, {}),
]


def hop(t, side):
    return keyed(t, HOP_KEYS), {"pelvis": (0, 0, 0.04 * math.sin(math.pi * min(1.0, t / 0.82)) if t > 0.15 else 0)}


# name: (function, frames, side_phase, loops)
ANIMS = {
    "Idle": (idle, 48, 0.0, True),
    "Run": (run, 12, 0.0, True),
    "Flail": (flail, 12, 0.0, True),
    "Grip": (grip, 48, 0.5, True),
    "Mash": (mash, 12, 0.5, True),
    "Tug": (tug, 24, 0.0, True),
    "TailFan": (tail_fan, 14, 0.0, True),
    "Hop": (hop, 10, 0.0, False),
}
KEY_EVERY = 1  # a key per frame (Bezier between)


def _sample(fn, t, side_phase):
    rot_l, loc = fn(t, "l")
    rot_r, _ = fn((t + side_phase) % 1.0, "r")
    out = {}
    for name, v in rot_l.items():
        if name.endswith("_r"):
            continue
        out[name] = v
        if name.endswith("_l"):
            twin = name[:-2] + "_r"
            if twin in rot_l:
                out[twin] = rot_l[twin]
            else:
                x, y, z = rot_r.get(name, v)
                out[twin] = (-x, y, -z)
    for name, v in rot_l.items():
        if name.endswith("_r") and name not in out:
            out[name] = v
    return out, loc


def build(arm):
    """Makes one action per entry of ANIMS on the armature object."""
    rest = {b.name: b.matrix_local.to_3x3().normalized() for b in arm.data.bones}
    for pb in arm.pose.bones:
        pb.rotation_mode = "QUATERNION"
    arm.animation_data_create()
    for name, (fn, frames, side_phase, loops) in ANIMS.items():
        action = bpy.data.actions.new(name)
        action.use_fake_user = True
        arm.animation_data.action = action
        for f in range(0, frames + 1, KEY_EVERY):
            t = f / frames
            rot, loc = _sample(fn, t % 1.0 if loops else t, side_phase)
            for pb in arm.pose.bones:
                r = rest[pb.name]
                deg = rot.get(pb.name, (0, 0, 0))
                q = Euler([math.radians(a) for a in deg], "XYZ").to_quaternion()
                pb.rotation_quaternion = (r.inverted() @ q.to_matrix() @ r).to_quaternion()
                pb.location = r.inverted() @ Vector(loc.get(pb.name, (0, 0, 0)))
                pb.keyframe_insert("rotation_quaternion", frame=f, group=pb.name)
                pb.keyframe_insert("location", frame=f, group=pb.name)
        action.use_frame_range = True
        action.frame_start, action.frame_end = 0, frames
        action.use_cyclic = loops
    arm.animation_data.action = bpy.data.actions["Idle"]
    for pb in arm.pose.bones:
        pb.rotation_quaternion = Quaternion()
        pb.location = Vector()
