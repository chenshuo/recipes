#!/usr/bin/env python3
"""
Voyager 2 (1977-89): Grand Tour trajectory with patched conics, and animation.

Method
  1. Planet positions from JPL approximate Keplerian elements (Standish, 1800-2050).
  2. One Lambert arc per leg: Earth -> Jupiter -> Saturn -> Uranus -> Neptune,
     using the real launch and closest-approach dates.
  3. Each arc propagated around the Sun with RK4.
  4. At each planet, v_inf in/out and the turn angle give the flyby distance
     needed for that gravity assist, compared with the real mission.

Usage
  python voyager2.py                     # show the animation
  python voyager2.py --save v2.mp4       # save as video (needs ffmpeg); .gif uses Pillow
  python voyager2.py --scale linear      # true distances instead of sqrt-compressed
  python voyager2.py --no-anim           # print results only

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
    "jupiter": dict(a=(5.20288700, -0.00011607), e=(0.04838624, -0.00013253),
                    I=(1.30439695, -0.00183714), L=(34.39644051, 3034.74612775),
                    w=(14.72847983, 0.21252668), O=(100.47390909, 0.20469106)),
    "saturn":  dict(a=(9.53667594, -0.00125060), e=(0.05386179, -0.00050991),
                    I=(2.48599187, 0.00193609), L=(49.95424423, 1222.49362201),
                    w=(92.59887831, -0.41897216), O=(113.66242448, -0.28867794)),
    "uranus":  dict(a=(19.18916464, -0.00196176), e=(0.04725744, -0.00004397),
                    I=(0.77263783, -0.00242939), L=(313.23810451, 428.48202785),
                    w=(170.95427630, 0.40805281), O=(74.01692503, 0.04240589)),
    "neptune": dict(a=(30.06992276, 0.00026291), e=(0.00859048, 0.00005105),
                    I=(1.77004347, 0.00035372), L=(-55.12002969, 218.45945325),
                    w=(44.96476227, -0.32241464), O=(131.78422574, -0.00508664)),
}
PERIOD = {"earth": 365.256, "mars": 686.98, "jupiter": 4332.59, "saturn": 10759.2,
          "uranus": 30688.5, "neptune": 60182.0}

# Planet GM [km^3/s^2], equatorial radius [km], actual Voyager 2 closest approach above cloud tops [km]
PHYS = {"jupiter": (126686534, 71492, 570000), "saturn": (37931187, 60268, 101000),
        "uranus": (5793939, 25559, 81500), "neptune": (6836529, 24764, 4950)}


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
# Encounter sequence (UTC of closest approach)
EVENTS = [
    ("earth",   julian_date(1977, 8, 20, 14 + 29 / 60), "Launch"),
    ("jupiter", julian_date(1979, 7, 9, 22 + 29 / 60),  "Jupiter flyby"),
    ("saturn",  julian_date(1981, 8, 26, 3 + 24 / 60),  "Saturn flyby"),
    ("uranus",  julian_date(1986, 1, 24, 17 + 59 / 60), "Uranus flyby"),
    ("neptune", julian_date(1989, 8, 25, 3 + 56 / 60),  "Neptune flyby"),
]
JD_START, JD_END = EVENTS[0][1] - 60, EVENTS[-1][1] + 120


def compute():
    """Patched conics: one Lambert arc per leg, then RK4 propagation of each arc."""
    legs = []
    for (p1, t1, _), (p2, t2, _) in zip(EVENTS[:-1], EVENTS[1:]):
        R1, R2 = planet_position(p1, t1), planet_position(p2, t2)
        V1, V2, _ = lambert(R1, R2, t2 - t1)
        ts, rs, vs = propagate(R1, V1, t1, t2, h=0.5)
        legs.append(dict(p1=p1, p2=p2, t1=t1, t2=t2, V1=V1, V2=V2, ts=ts, rs=rs, vs=vs,
                         miss_km=np.linalg.norm(rs[-1] - R2) * AU_KM))

    print("Voyager 2 Grand Tour (patched conics, Sun-only legs)\n")
    print(f"{'Leg':<20}{'Days':>7}{'Arrival miss':>15}")
    for L in legs:
        print(f"{L['p1'].title() + ' -> ' + L['p2'].title():<20}{L['t2'] - L['t1']:7.0f}{L['miss_km']:12.0f} km")

    print(f"\n{'Flyby':<10}{'Date':>13}{'v_inf in':>10}{'v_inf out':>11}{'Turn':>7}"
          f"{'Closest (model)':>17}{'Actual':>11}")
    for k in range(1, len(EVENTS)):
        p, t, _ = EVENTS[k]
        vp = planet_velocity(p, t)
        vin = legs[k - 1]["V2"] - vp
        a = np.linalg.norm(vin) * AUD_KMS
        date = f"{jd_to_datetime(t):%d %b %Y}"
        gm, R, actual = PHYS[p]
        if k < len(EVENTS) - 1:
            vout = legs[k]["V1"] - vp
            b = np.linalg.norm(vout) * AUD_KMS
            turn = math.acos(np.clip(np.dot(vin, vout) / (np.linalg.norm(vin) * np.linalg.norm(vout)), -1, 1))
            vinf = 0.5 * (a + b)
            # hyperbolic flyby: turn angle delta = 2*asin(1 / (1 + rp*vinf^2/mu))
            rp = gm / vinf**2 * (1 / math.sin(turn / 2) - 1)
            alt = f"{(rp - R) / 1000:12.0f}k km" if rp > R else "      inside"
            print(f"{p.title():<10}{date:>13}{a:8.1f}  {b:9.1f}  {turn / DEG:5.0f}°{alt:>17}{actual / 1000:8.1f}k km")
        else:
            print(f"{p.title():<10}{date:>13}{a:8.1f}  {'-':>9}  {'-':>5} {'-':>16}{actual / 1000:8.2f}k km")
    print("\n(v_inf in km/s; altitudes above the cloud tops)")
    return legs


def sc_state(jd, legs):
    if jd <= EVENTS[0][1]:
        return planet_position("earth", jd), planet_velocity("earth", jd), -1
    if jd >= EVENTS[-1][1]:
        return None, None, len(legs)
    for k, L in enumerate(legs):
        if jd < L["t2"]:
            r = np.array([np.interp(jd, L["ts"], L["rs"][:, i]) for i in range(3)])
            v = np.array([np.interp(jd, L["ts"], L["vs"][:, i]) for i in range(3)])
            return r, v, k


# ---------------------------------------------------------------- animation
COLORS = {"earth": "#5fb0ff", "mars": "#ff7a4a", "jupiter": "#e0a96a",
          "saturn": "#f0cf5a", "uranus": "#6fe3e0", "neptune": "#8c9cff"}
SIZES = {"earth": 6, "mars": 5, "jupiter": 10, "saturn": 9, "uranus": 7, "neptune": 7}


def animate(legs, days_per_frame=5.0, fps=30, scale="sqrt", save=None):
    import matplotlib.pyplot as plt
    from matplotlib.animation import FuncAnimation

    RMAX = 32.0

    def proj(xyz):
        """Project ecliptic positions to the plot; 'sqrt' compresses radius so inner planets stay visible."""
        xyz = np.atleast_2d(xyz)
        x, y = xyz[:, 0], xyz[:, 1]
        if scale == "linear":
            return x, y
        r = np.hypot(x, y)
        s = np.where(r > 0, np.sqrt(r * RMAX) / np.where(r > 0, r, 1), 0)
        return x * s, y * s

    plt.style.use("dark_background")
    fig, ax = plt.subplots(figsize=(9, 9))
    ax.set_aspect("equal")
    ax.set_xlim(-RMAX, RMAX)
    ax.set_ylim(-RMAX, RMAX)
    ax.set_xticks([])
    ax.set_yticks([])
    ax.set_title("Voyager 2: the Grand Tour, 1977-89"
                 + ("  (radius compressed as √r)" if scale == "sqrt" else ""))

    for d in (1, 5, 10, 20, 30):                    # distance rings
        th = np.linspace(0, 2 * np.pi, 200)
        px, py = proj(np.c_[d * np.cos(th), d * np.sin(th), 0 * th])
        ax.plot(px, py, color="0.2", lw=0.6)
        lx, ly = proj([d * 0.707, -d * 0.707, 0])
        ax.text(lx[0], ly[0], f"{d} AU", color="0.45", fontsize=8)

    planets = list(COLORS)
    for p in planets:
        orb = np.array([planet_position(p, EVENTS[0][1] + PERIOD[p] * k / 360) for k in range(361)])
        ax.plot(*proj(orb), color="0.4", lw=0.8)
    full = np.vstack([L["rs"] for L in legs])
    ax.plot(*proj(full), "--", color="0.55", lw=0.8)
    for p, t, lab in EVENTS:
        px, py = proj(planet_position(p, t))
        ax.plot(px, py, "x", color="0.7")
        ax.annotate(lab, (px[0], py[0]), xytext=(6, -12), textcoords="offset points", color="0.7", fontsize=8)
    ax.plot(0, 0, "o", color="gold", ms=10)

    pdots = {p: ax.plot([], [], "o", color=COLORS[p], ms=SIZES[p], label=p.title())[0] for p in planets}
    trail, = ax.plot([], [], "-", color="#f4f1d0", lw=1.8)
    craft, = ax.plot([], [], "o", color="#f4f1d0", ms=4, label="Voyager 2")
    info = ax.text(0.02, 0.98, "", transform=ax.transAxes, va="top", family="monospace", fontsize=9)
    ax.legend(loc="lower right", fontsize=8)

    frames = np.arange(JD_START, JD_END, days_per_frame)

    def update(jd):
        pos = {p: planet_position(p, jd) for p in planets}
        for p in planets:
            pdots[p].set_data(*proj(pos[p]))
        r, v, k = sc_state(jd, legs)
        if r is None:                                # after Neptune: path not modelled
            craft.set_data([], [])
            info.set_text(f"{jd_to_datetime(jd):%d %b %Y}\nNeptune flyby complete\n(post-flyby path not modelled)")
            return
        flown = [L["rs"][L["ts"] <= jd] for L in legs[:max(k, 0) + 1]] if k >= 0 else []
        if flown:
            pts = np.vstack(flown + [r[None, :]])
            trail.set_data(*proj(pts))
            craft.set_data(*proj(r))
        else:
            trail.set_data([], [])
            craft.set_data([], [])
        dist = np.linalg.norm(r)
        dE = np.linalg.norm(r - pos["earth"])
        target = f"Next: {legs[k]['p2'].title()} in {legs[k]['t2'] - jd:5.0f} d" if k >= 0 else \
            f"Launch in {EVENTS[0][1] - jd:4.0f} d"
        info.set_text(
            f"{jd_to_datetime(jd):%d %b %Y}\n{target}\n"
            f"Sun distance  {dist:6.2f} AU\n"
            f"Speed         {np.linalg.norm(v) * AUD_KMS:6.2f} km/s\n"
            f"Solar escape  {math.sqrt(2 * MU / dist) * AUD_KMS:6.2f} km/s\n"
            f"Light time    {dE * AU_KM / 299792.458 / 3600:6.2f} h")

    anim = FuncAnimation(fig, update, frames=frames, interval=1000 / fps, blit=False, repeat=False)
    if save:
        anim.save(save, fps=fps, dpi=110)
        print(f"Saved {save}")
    else:
        plt.show()


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--save", help="output file (.mp4 or .gif)")
    ap.add_argument("--days-per-frame", type=float, default=5.0)
    ap.add_argument("--fps", type=int, default=30)
    ap.add_argument("--scale", choices=["sqrt", "linear"], default="sqrt",
                    help="sqrt compresses distances so the inner planets stay visible")
    ap.add_argument("--no-anim", action="store_true", help="print results only")
    args = ap.parse_args()
    legs = compute()
    if not args.no_anim:
        animate(legs, args.days_per_frame, args.fps, args.scale, args.save)
