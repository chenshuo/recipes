#!/usr/bin/env python3
"""
Mariner 4 (1964-65): Earth -> Mars transfer trajectory and animation.

Method
  1. Earth and Mars positions from JPL approximate Keplerian elements (Standish, 1800-2050).
  2. Lambert's problem (universal variables) between Earth at launch and Mars at flyby.
  3. Two-body propagation around the Sun with RK4.
  4. Top-down matplotlib animation of the ecliptic plane.

Usage
  python mariner4.py                  # show the animation
  python mariner4.py --save m4.mp4    # save as video (needs ffmpeg); .gif uses Pillow
  python mariner4.py --no-anim        # print results only

Requires: numpy, matplotlib
"""
import argparse
import math
from datetime import datetime, timedelta, timezone

import numpy as np

# ---------------------------------------------------------------- constants
MU = 2.9591220828559115e-4          # Sun GM [AU^3/day^2]
AU_KM = 149597870.7
AUD_KMS = AU_KM / 86400.0           # AU/day -> km/s
DEG = math.pi / 180.0

# JPL approximate elements: (value at J2000, rate per Julian century)
# a [AU], e, I [deg], L [deg], long. perihelion [deg], long. asc. node [deg]
ELEMENTS = {
    "earth": dict(a=(1.00000261, 0.00000562), e=(0.01671123, -0.00004392),
                  I=(-0.00001531, -0.01294668), L=(100.46457166, 35999.37244981),
                  w=(102.93768193, 0.32327364), O=(0.0, 0.0)),
    "mars":  dict(a=(1.52371034, 0.00001847), e=(0.09339410, 0.00007882),
                  I=(1.84969142, -0.00813131), L=(-4.55343205, 19140.30268499),
                  w=(-23.94362959, 0.44441088), O=(49.55953891, -0.29257343)),
}
PERIOD = {"earth": 365.256, "mars": 686.98}


# ---------------------------------------------------------------- time
def julian_date(y, m, d, h=0.0):
    """Julian Date of a Gregorian calendar date (UTC hours h)."""
    if m <= 2:
        y -= 1
        m += 12
    A = y // 100
    B = 2 - A + A // 4
    return math.floor(365.25 * (y + 4716)) + math.floor(30.6001 * (m + 1)) + d + B - 1524.5 + h / 24.0


def jd_to_datetime(jd):
    return datetime(1970, 1, 1, tzinfo=timezone.utc) + timedelta(days=jd - 2440587.5)


# ---------------------------------------------------------------- ephemeris
def planet_position(name, jd):
    """Heliocentric ecliptic J2000 position [AU]."""
    T = (jd - 2451545.0) / 36525.0
    el = {k: v[0] + v[1] * T for k, v in ELEMENTS[name].items()}
    a, e = el["a"], el["e"]
    I, L, wbar, O = (el[k] * DEG for k in ("I", "L", "w", "O"))
    om = wbar - O                                   # argument of perihelion
    M = (L - wbar + math.pi) % (2 * math.pi) - math.pi
    E = M + e * math.sin(M)
    for _ in range(30):                             # Kepler's equation, Newton
        E -= (E - e * math.sin(E) - M) / (1 - e * math.cos(E))
    xp = a * (math.cos(E) - e)
    yp = a * math.sqrt(1 - e * e) * math.sin(E)
    cw, sw, cO, sO, cI, sI = math.cos(om), math.sin(om), math.cos(O), math.sin(O), math.cos(I), math.sin(I)
    return np.array([
        (cw * cO - sw * sO * cI) * xp + (-sw * cO - cw * sO * cI) * yp,
        (cw * sO + sw * cO * cI) * xp + (-sw * sO + cw * cO * cI) * yp,
        (sw * sI) * xp + (cw * sI) * yp,
    ])


def planet_velocity(name, jd, dt=0.01):
    return (planet_position(name, jd + dt) - planet_position(name, jd - dt)) / (2 * dt)


# ---------------------------------------------------------------- Lambert
def stumpff(z):
    if z > 1e-8:
        s = math.sqrt(z)
        return (s - math.sin(s)) / s**3, (1 - math.cos(s)) / z
    if z < -1e-8:
        s = math.sqrt(-z)
        return (math.sinh(s) - s) / s**3, (math.cosh(s) - 1) / (-z)
    return 1 / 6, 1 / 2


def lambert(R1, R2, dt, mu=MU):
    """Prograde single-revolution Lambert solution (Curtis, Algorithm 5.2).
    Returns departure velocity, arrival velocity, transfer angle [rad]."""
    r1, r2 = np.linalg.norm(R1), np.linalg.norm(R2)
    theta = math.acos(np.clip(np.dot(R1, R2) / (r1 * r2), -1, 1))
    if np.cross(R1, R2)[2] < 0:
        theta = 2 * math.pi - theta
    A = math.sin(theta) * math.sqrt(r1 * r2 / (1 - math.cos(theta)))

    def y(z):
        S, C = stumpff(z)
        return r1 + r2 + A * (z * S - 1) / math.sqrt(C)

    def F(z):
        S, C = stumpff(z)
        yy = y(z)
        return (yy / C) ** 1.5 * S + A * math.sqrt(yy) - math.sqrt(mu) * dt

    lo, hi = -100 * math.pi**2, 4 * math.pi**2 * 0.9999
    while y(lo) < 0:
        lo += 0.01
    for _ in range(300):                            # bisection: F increases with z
        mid = 0.5 * (lo + hi)
        if y(mid) < 0 or F(mid) < 0:
            lo = mid
        else:
            hi = mid
    z = 0.5 * (lo + hi)
    yy = y(z)
    f = 1 - yy / r1
    g = A * math.sqrt(yy / mu)
    gdot = 1 - yy / r2
    return (R2 - f * R1) / g, (gdot * R2 - R1) / g, theta


# ---------------------------------------------------------------- propagation
def accel(r):
    return -MU * r / np.linalg.norm(r) ** 3


def rk4_step(r, v, h):
    k1r, k1v = v, accel(r)
    k2r, k2v = v + k1v * h / 2, accel(r + k1r * h / 2)
    k3r, k3v = v + k2v * h / 2, accel(r + k2r * h / 2)
    k4r, k4v = v + k3v * h, accel(r + k3r * h)
    return (r + h / 6 * (k1r + 2 * k2r + 2 * k3r + k4r),
            v + h / 6 * (k1v + 2 * k2v + 2 * k3v + k4v))


def propagate(r0, v0, t0, t1, h=0.25):
    n = int(math.ceil((t1 - t0) / h))
    h = (t1 - t0) / n
    ts = t0 + h * np.arange(n + 1)
    rs, vs = np.empty((n + 1, 3)), np.empty((n + 1, 3))
    r, v = r0.copy(), v0.copy()
    for i in range(n + 1):
        rs[i], vs[i] = r, v
        if i < n:
            r, v = rk4_step(r, v, h)
    return ts, rs, vs


# ---------------------------------------------------------------- mission
JD_LAUNCH = julian_date(1964, 11, 28, 14 + 22 / 60)   # 28 Nov 1964 14:22 UTC
JD_FLYBY = julian_date(1965, 7, 15, 1.0)              # 15 Jul 1965 ~01:00 UTC
JD_START, JD_END = JD_LAUNCH - 25, JD_FLYBY + 100


def compute():
    R1 = planet_position("earth", JD_LAUNCH)
    R2 = planet_position("mars", JD_FLYBY)
    V1, V2, theta = lambert(R1, R2, JD_FLYBY - JD_LAUNCH)
    ts1, rs1, vs1 = propagate(R1, V1, JD_LAUNCH, JD_FLYBY)      # cruise, ends exactly at flyby
    miss_km = np.linalg.norm(rs1[-1] - R2) * AU_KM
    ts2, rs2, vs2 = propagate(rs1[-1], vs1[-1], JD_FLYBY, JD_END)  # after flyby (no Mars gravity)
    ts, rs, vs = (np.concatenate([a[:-1], b]) for a, b in ((ts1, ts2), (rs1, rs2), (vs1, vs2)))

    print("Mariner 4 transfer (Sun-only two-body model)")
    print(f"  Launch      {jd_to_datetime(JD_LAUNCH):%d %b %Y %H:%M} UTC")
    print(f"  Mars flyby  {jd_to_datetime(JD_FLYBY):%d %b %Y %H:%M} UTC")
    print(f"  Flight time        {JD_FLYBY - JD_LAUNCH:8.1f} days")
    print(f"  Transfer angle     {theta / DEG:8.1f} deg")
    print(f"  Departure v_inf    {np.linalg.norm(V1 - planet_velocity('earth', JD_LAUNCH)) * AUD_KMS:8.2f} km/s")
    print(f"  Arrival v_inf      {np.linalg.norm(V2 - planet_velocity('mars', JD_FLYBY)) * AUD_KMS:8.2f} km/s")
    print(f"  Arrival miss check {miss_km:8.0f} km")
    return ts, rs, vs


def sc_position(jd, ts, rs):
    if jd < JD_LAUNCH:
        return planet_position("earth", jd)
    return np.array([np.interp(jd, ts, rs[:, k]) for k in range(3)])


# ---------------------------------------------------------------- animation
def animate(ts, rs, vs, days_per_frame=1.0, fps=30, save=None):
    import matplotlib.pyplot as plt
    from matplotlib.animation import FuncAnimation

    plt.style.use("dark_background")
    fig, ax = plt.subplots(figsize=(8, 8))
    ax.set_aspect("equal")
    ax.set_xlim(-1.8, 1.8)
    ax.set_ylim(-1.8, 1.8)
    ax.set_xlabel("x [AU] (toward vernal equinox)")
    ax.set_ylabel("y [AU]")
    ax.grid(alpha=0.15)
    ax.set_title("Mariner 4: Earth to Mars, 1964-65")

    for p in ("earth", "mars"):
        orb = np.array([planet_position(p, JD_LAUNCH + PERIOD[p] * k / 360) for k in range(361)])
        ax.plot(orb[:, 0], orb[:, 1], color="0.4", lw=0.8)
    ax.plot(rs[:, 0], rs[:, 1], "--", color="0.6", lw=0.8, label="planned path")
    ax.plot(0, 0, "o", color="gold", ms=12)
    for jd, p, lab in ((JD_LAUNCH, "earth", "Launch"), (JD_FLYBY, "mars", "Mars flyby")):
        x, y, _ = planet_position(p, jd)
        ax.plot(x, y, "x", color="0.7")
        ax.annotate(lab, (x, y), xytext=(6, -12), textcoords="offset points", color="0.7", fontsize=9)

    earth, = ax.plot([], [], "o", color="#5fb0ff", ms=8, label="Earth")
    mars, = ax.plot([], [], "o", color="#ff7a4a", ms=7, label="Mars")
    trail, = ax.plot([], [], "-", color="#f4f1d0", lw=1.8)
    craft, = ax.plot([], [], "o", color="#f4f1d0", ms=4, label="Mariner 4")
    info = ax.text(0.02, 0.98, "", transform=ax.transAxes, va="top", family="monospace", fontsize=9)
    ax.legend(loc="lower right", fontsize=9)

    frames = np.arange(JD_START, JD_END, days_per_frame)

    def update(jd):
        E, M = planet_position("earth", jd), planet_position("mars", jd)
        S = sc_position(jd, ts, rs)
        earth.set_data([E[0]], [E[1]])
        mars.set_data([M[0]], [M[1]])
        if jd >= JD_LAUNCH:
            n = np.searchsorted(ts, jd)
            trail.set_data(np.append(rs[:n, 0], S[0]), np.append(rs[:n, 1], S[1]))
            craft.set_data([S[0]], [S[1]])
            v = np.array([np.interp(jd, ts, vs[:, k]) for k in range(3)])
            speed = np.linalg.norm(v) * AUD_KMS
        else:
            craft.set_data([], [])
            speed = np.linalg.norm(planet_velocity("earth", jd)) * AUD_KMS
        info.set_text(
            f"{jd_to_datetime(jd):%d %b %Y}\n"
            f"Sun distance   {np.linalg.norm(S):6.3f} AU\n"
            f"Speed          {speed:6.2f} km/s\n"
            f"From Earth     {np.linalg.norm(S - E) * AU_KM / 1e6:6.1f} M km\n"
            f"From Mars      {np.linalg.norm(S - M) * AU_KM / 1e6:6.1f} M km")
        return earth, mars, trail, craft, info

    anim = FuncAnimation(fig, update, frames=frames, interval=1000 / fps, blit=False, repeat=False)
    if save:
        anim.save(save, fps=fps, dpi=120)
        print(f"Saved {save}")
    else:
        plt.show()


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--save", help="output file (.mp4 or .gif)")
    ap.add_argument("--days-per-frame", type=float, default=1.0)
    ap.add_argument("--fps", type=int, default=30)
    ap.add_argument("--no-anim", action="store_true", help="print results only")
    args = ap.parse_args()
    ts, rs, vs = compute()
    if not args.no_anim:
        animate(ts, rs, vs, args.days_per_frame, args.fps, args.save)
