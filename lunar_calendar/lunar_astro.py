#!/usr/bin/env python3
"""版本 2：用天文算法自行推算 2000~2100 年的农历。

不查任何历表，按照现行农历编排规则（GB/T 33661-2017《农历的编算和颁行》）推算：

  1. 以东经 120° 标准时（北京时间，UTC+8）为准；
  2. 含有"朔"（日月合朔，新月）时刻的那一天为农历月的初一；
  3. 含有"冬至"的农历月为十一月；
  4. 从一个十一月（含）到下一个十一月（不含）若有 13 个农历月，
     则其中第一个不含"中气"的月为闰月，月序与前一个月相同。

用到的天文计算（公式均出自 Jean Meeus《Astronomical Algorithms》第 2 版）：

  * 太阳视黄经：VSOP87 理论的截断级数（Meeus 附录 III，精度约 1"），
    加上 FK5 修正、章动（第 22 章简化公式）和光行差。
    冬至 = 太阳视黄经 270°，各"中气" = 视黄经为 30° 的整数倍的时刻。
  * 朔（新月）时刻：Meeus 第 49 章的周期项公式（误差通常只有几秒）。
  * 力学时 TD 与世界时 UT 的差 ΔT：Espenak & Meeus 的多项式。

只使用 Python 标准库。用法见 `python3 lunar_astro.py --help`。
"""

import bisect
import datetime
import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import lunar_common  # pylint: disable=g-import-not-at-top

# JD(儒略日) 与 Python date.toordinal() 的换算：某日 0 时(UT) 的 JD = ordinal + ORD_TO_JD
ORD_TO_JD = 1721424.5
J2000 = 2451545.0
BEIJING_OFFSET = 8.0 / 24.0  # UTC+8

SOLAR_TERM_NAMES = [  # 视黄经 0°, 15°, 30°, ... 对应的节气
    "春分",
    "清明",
    "谷雨",
    "立夏",
    "小满",
    "芒种",
    "夏至",
    "小暑",
    "大暑",
    "立秋",
    "处暑",
    "白露",
    "秋分",
    "寒露",
    "霜降",
    "立冬",
    "小雪",
    "大雪",
    "冬至",
    "小寒",
    "大寒",
    "立春",
    "雨水",
    "惊蛰",
]

# ---------------------------------------------------------------------------
# ΔT = TD - UT（秒）
# ---------------------------------------------------------------------------


def delta_t_seconds(year: float) -> float:
  """Espenak & Meeus (NASA, 2006) 的 ΔT 多项式，适用于 1986~2150 年。"""
  if year < 2005:
    t = year - 2000
    return (
        63.86
        + 0.3345 * t
        - 0.060374 * t**2
        + 0.0017275 * t**3
        + 0.000651814 * t**4
        + 0.00002373599 * t**5
    )
  if year < 2050:
    t = year - 2000
    return 62.92 + 0.32217 * t + 0.005589 * t**2
  u = (year - 1820) / 100
  return -20 + 32 * u * u - 0.5628 * (2150 - year)


def td_to_beijing_jd(jde: float) -> float:
  """力学时 JDE -> 北京时间的"儒略日"（即 UT 的 JD 加 8 小时）。"""
  year = 2000 + (jde - J2000) / 365.25
  return jde - delta_t_seconds(year) / 86400 + BEIJING_OFFSET


def beijing_jd_to_date(jd: float) -> datetime.date:
  return datetime.date.fromordinal(int(math.floor(jd - ORD_TO_JD)))


def beijing_jd_to_datetime(jd: float) -> datetime.datetime:
  ordinal = math.floor(jd - ORD_TO_JD)
  frac = jd - ORD_TO_JD - ordinal
  return datetime.datetime.fromordinal(ordinal) + datetime.timedelta(
      seconds=round(frac * 86400)
  )


# ---------------------------------------------------------------------------
# 太阳视黄经（VSOP87 截断级数，Meeus 附录 III）
# 每项为 (A, B, C)，值 = A * cos(B + C*τ)，τ 为自 J2000 起的儒略千年数，A 单位 1e-8 rad
# ---------------------------------------------------------------------------

_L0 = (
    (175347046, 0, 0),
    (3341656, 4.6692568, 6283.07585),
    (34894, 4.6261, 12566.1517),
    (3497, 2.7441, 5753.3849),
    (3418, 2.8289, 3.5231),
    (3136, 3.6277, 77713.7715),
    (2676, 4.4181, 7860.4194),
    (2343, 6.1352, 3930.2097),
    (1324, 0.7425, 11506.7698),
    (1273, 2.0371, 529.691),
    (1199, 1.1096, 1577.3435),
    (990, 5.233, 5884.927),
    (902, 2.045, 26.298),
    (857, 3.508, 398.149),
    (780, 1.179, 5223.694),
    (753, 2.533, 5507.553),
    (505, 4.583, 18849.228),
    (492, 4.205, 775.523),
    (357, 2.92, 0.067),
    (317, 5.849, 11790.629),
    (284, 1.899, 796.298),
    (271, 0.315, 10977.079),
    (243, 0.345, 5486.778),
    (206, 4.806, 2544.314),
    (205, 1.869, 5573.143),
    (202, 2.458, 6069.777),
    (156, 0.833, 213.299),
    (132, 3.411, 2942.463),
    (126, 1.083, 20.775),
    (115, 0.645, 0.98),
    (103, 0.636, 4694.003),
    (102, 0.976, 15720.839),
    (102, 4.267, 7.114),
    (99, 6.21, 2146.17),
    (98, 0.68, 155.42),
    (86, 5.98, 161000.69),
    (85, 1.3, 6275.96),
    (85, 3.67, 71430.7),
    (80, 1.81, 17260.15),
    (79, 3.04, 12036.46),
    (75, 1.76, 5088.63),
    (74, 3.5, 3154.69),
    (74, 4.68, 801.82),
    (70, 0.83, 9437.76),
    (62, 3.98, 8827.39),
    (61, 1.82, 7084.9),
    (57, 2.78, 6286.6),
    (56, 4.39, 14143.5),
    (56, 3.47, 6279.55),
    (52, 0.19, 12139.55),
    (52, 1.33, 1748.02),
    (51, 0.28, 5856.48),
    (49, 0.49, 1194.45),
    (41, 5.37, 8429.24),
    (41, 2.4, 19651.05),
    (39, 6.17, 10447.39),
    (37, 6.04, 10213.29),
    (37, 2.57, 1059.38),
    (36, 1.71, 2352.87),
    (36, 1.78, 6812.77),
    (33, 0.59, 17789.85),
    (30, 0.44, 83996.85),
    (30, 2.74, 1349.87),
    (25, 3.16, 4690.48),
)
_L1 = (
    (628331966747, 0, 0),
    (206059, 2.678235, 6283.07585),
    (4303, 2.6351, 12566.1517),
    (425, 1.59, 3.523),
    (119, 5.796, 26.298),
    (109, 2.966, 1577.344),
    (93, 2.59, 18849.23),
    (72, 1.14, 529.69),
    (68, 1.87, 398.15),
    (67, 4.41, 5507.55),
    (59, 2.89, 5223.69),
    (56, 2.17, 155.42),
    (45, 0.4, 796.3),
    (36, 0.47, 775.52),
    (29, 2.65, 7.11),
    (21, 5.34, 0.98),
    (19, 1.85, 5486.78),
    (19, 4.97, 213.3),
    (17, 2.99, 6275.96),
    (16, 0.03, 2544.31),
    (16, 1.43, 2146.17),
    (15, 1.21, 10977.08),
    (12, 2.83, 1748.02),
    (12, 3.26, 5088.63),
    (12, 5.27, 1194.45),
    (12, 2.08, 4694.0),
    (11, 0.77, 553.57),
    (10, 1.3, 6286.6),
    (10, 4.24, 1349.87),
    (9, 2.7, 242.73),
    (9, 5.64, 951.72),
    (8, 5.3, 2352.87),
    (6, 2.65, 9437.76),
    (6, 4.67, 4690.48),
)
_L2 = (
    (52919, 0, 0),
    (8720, 1.0721, 6283.0758),
    (309, 0.867, 12566.152),
    (27, 0.05, 3.52),
    (16, 5.19, 26.3),
    (16, 3.68, 155.42),
    (10, 0.76, 18849.23),
    (9, 2.06, 77713.77),
    (7, 0.83, 775.52),
    (5, 4.66, 1577.34),
    (4, 1.03, 7.11),
    (4, 3.44, 5573.14),
    (3, 5.14, 796.3),
    (3, 6.05, 5507.55),
    (3, 1.19, 242.73),
    (3, 6.12, 529.69),
    (3, 0.31, 398.15),
    (3, 2.28, 553.57),
    (2, 4.38, 5223.69),
    (2, 3.75, 0.98),
)
_L3 = (
    (289, 5.844, 6283.076),
    (35, 0, 0),
    (17, 5.49, 12566.15),
    (3, 5.2, 155.42),
    (1, 4.72, 3.52),
    (1, 5.3, 18849.23),
    (1, 5.97, 242.73),
)
_L4 = ((114, 3.142, 0), (8, 4.13, 6283.08), (1, 3.84, 12566.15))
_L5 = ((1, 3.14, 0),)

_B0 = (
    (280, 3.199, 84334.662),
    (102, 5.422, 5507.553),
    (80, 3.88, 5223.69),
    (44, 3.7, 2352.87),
    (32, 4.0, 1577.34),
)
_B1 = ((9, 3.9, 5507.55), (6, 1.73, 5223.69))

_R0 = (
    (100013989, 0, 0),
    (1670700, 3.0984635, 6283.07585),
    (13956, 3.05525, 12566.1517),
    (3084, 5.1985, 77713.7715),
    (1628, 1.1739, 5753.3849),
    (1576, 2.8469, 7860.4194),
    (925, 5.453, 11506.77),
    (542, 4.564, 3930.21),
    (472, 3.661, 5884.927),
    (346, 0.964, 5507.553),
    (329, 5.9, 5223.694),
    (307, 0.299, 5573.143),
    (243, 4.273, 11790.629),
    (212, 5.847, 1577.344),
    (186, 5.022, 10977.079),
    (175, 3.012, 18849.228),
    (110, 5.055, 5486.778),
    (98, 0.89, 6069.78),
    (86, 5.69, 15720.84),
    (86, 1.27, 161000.69),
    (65, 0.27, 17260.15),
    (63, 0.92, 529.69),
    (57, 2.01, 83996.85),
    (56, 5.24, 71430.7),
    (49, 3.25, 2544.31),
    (47, 2.58, 775.52),
    (45, 5.54, 9437.76),
    (43, 6.01, 6275.96),
    (39, 5.36, 4694.0),
    (38, 2.39, 8827.39),
    (37, 0.83, 19651.05),
    (37, 4.9, 12139.55),
    (36, 1.67, 12036.46),
    (35, 1.84, 2942.46),
    (33, 0.24, 7084.9),
    (32, 0.18, 5088.63),
    (32, 1.78, 398.15),
    (28, 1.21, 6286.6),
    (28, 1.9, 6279.55),
    (26, 4.59, 10447.39),
)
_R1 = (
    (103019, 1.10749, 6283.07585),
    (1721, 1.0644, 12566.1517),
    (702, 3.142, 0),
    (32, 1.02, 18849.23),
    (31, 2.84, 5507.55),
    (25, 1.32, 5223.69),
    (18, 1.42, 1577.34),
    (10, 5.91, 10977.08),
    (9, 1.42, 6275.96),
    (9, 0.27, 5486.78),
)
_R2 = (
    (4359, 5.7846, 6283.0758),
    (124, 5.579, 12566.152),
    (12, 3.14, 0),
    (9, 3.63, 77713.77),
    (6, 1.87, 5573.14),
    (3, 5.47, 18849.23),
)
_R3 = ((145, 4.273, 6283.076), (7, 3.92, 12566.15))
_R4 = ((4, 2.56, 6283.08),)


def _vsop(series_list, tau: float) -> float:
  total = 0.0
  for power, series in enumerate(series_list):
    s = sum(a * math.cos(b + c * tau) for a, b, c in series)
    total += s * tau**power
  return total * 1e-8


def _nutation_longitude_and_obliquity(t: float):
  """章动 Δψ 及黄赤交角（度）。Meeus 第 22 章简化公式，精度约 0.5"。"""
  omega = math.radians(125.04452 - 1934.136261 * t)
  l_sun = math.radians(280.4665 + 36000.7698 * t)
  l_moon = math.radians(218.3165 + 481267.8813 * t)
  dpsi = (
      -17.20 * math.sin(omega)
      - 1.32 * math.sin(2 * l_sun)
      - 0.23 * math.sin(2 * l_moon)
      + 0.21 * math.sin(2 * omega)
  )
  return dpsi / 3600.0


def sun_apparent_longitude(jde: float) -> float:
  """太阳视黄经（度，0~360），输入为力学时儒略日 JDE。"""
  tau = (jde - J2000) / 365250.0
  t = tau * 10
  l = math.degrees(_vsop((_L0, _L1, _L2, _L3, _L4, _L5), tau))
  b = math.degrees(_vsop((_B0, _B1), tau))
  r = _vsop((_R0, _R1, _R2, _R3, _R4), tau)
  # 日心黄经 -> 地心黄经
  theta = l + 180.0
  beta = -b
  # 转换到 FK5 系统
  lam1 = math.radians(theta - 1.397 * t - 0.00031 * t * t)
  theta += (
      -0.09033
      + 0.03916
      * (math.cos(lam1) + math.sin(lam1))
      * math.tan(math.radians(beta))
  ) / 3600.0
  # 章动与光行差
  theta += _nutation_longitude_and_obliquity(t)
  theta += -20.4898 / 3600.0 / r
  return theta % 360.0


def solar_term_jde(year: int, longitude: float) -> float:
  """求 year 年中太阳视黄经等于 longitude（度）的时刻（力学时 JDE）。

  用牛顿迭代：太阳平均每天移动约 360/365.2422 度。
  """
  # 春分（0°）约在 3 月 20 日；先按平均速度给出初值
  jde = (
      datetime.date(year, 3, 20).toordinal()
      + ORD_TO_JD
      + longitude / 360.0 * 365.2422
  )
  for _ in range(50):
    diff = (longitude - sun_apparent_longitude(jde) + 180.0) % 360.0 - 180.0
    jde += diff * 365.2422 / 360.0
    if abs(diff) < 1e-9:
      break
  return jde


# ---------------------------------------------------------------------------
# 朔（新月）时刻：Meeus 第 49 章
# ---------------------------------------------------------------------------


def new_moon_jde(k: int) -> float:
  """第 k 个朔的力学时 JDE；k=0 为 2000-01-06 前后的朔。"""
  t = k / 1236.85
  jde = (
      2451550.09766
      + 29.530588861 * k
      + 0.00015437 * t**2
      - 0.000000150 * t**3
      + 0.00000000073 * t**4
  )
  e = 1 - 0.002516 * t - 0.0000074 * t**2
  rad = math.radians
  m = rad(2.5534 + 29.10535670 * k - 0.0000014 * t**2 - 0.00000011 * t**3)
  mp = rad(
      201.5643
      + 385.81693528 * k
      + 0.0107582 * t**2
      + 0.00001238 * t**3
      - 0.000000058 * t**4
  )
  f = rad(
      160.7108
      + 390.67050284 * k
      - 0.0016118 * t**2
      - 0.00000227 * t**3
      + 0.000000011 * t**4
  )
  om = rad(124.7746 - 1.56375588 * k + 0.0020672 * t**2 + 0.00000215 * t**3)
  sin = math.sin
  jde += (
      -0.40720 * sin(mp)
      + 0.17241 * e * sin(m)
      + 0.01608 * sin(2 * mp)
      + 0.01039 * sin(2 * f)
      + 0.00739 * e * sin(mp - m)
      - 0.00514 * e * sin(mp + m)
      + 0.00208 * e * e * sin(2 * m)
      - 0.00111 * sin(mp - 2 * f)
      - 0.00057 * sin(mp + 2 * f)
      + 0.00056 * e * sin(2 * mp + m)
      - 0.00042 * sin(3 * mp)
      + 0.00042 * e * sin(m + 2 * f)
      + 0.00038 * e * sin(m - 2 * f)
      - 0.00024 * e * sin(2 * mp - m)
      - 0.00017 * sin(om)
      - 0.00007 * sin(mp + 2 * m)
      + 0.00004 * sin(2 * mp - 2 * f)
      + 0.00004 * sin(3 * m)
      + 0.00003 * sin(mp + m - 2 * f)
      + 0.00003 * sin(2 * mp + 2 * f)
      - 0.00003 * sin(mp + m + 2 * f)
      + 0.00003 * sin(mp - m + 2 * f)
      - 0.00002 * sin(mp - m - 2 * f)
      - 0.00002 * sin(3 * mp + m)
      + 0.00002 * sin(4 * mp)
  )
  # 行星摄动的附加项
  planetary = (
      (0.000325, 299.77 + 0.107408 * k - 0.009173 * t**2),
      (0.000165, 251.88 + 0.016321 * k),
      (0.000164, 251.83 + 26.651886 * k),
      (0.000126, 349.42 + 36.412478 * k),
      (0.000110, 84.66 + 18.206239 * k),
      (0.000062, 141.74 + 53.303771 * k),
      (0.000060, 207.14 + 2.453732 * k),
      (0.000056, 154.84 + 7.306860 * k),
      (0.000047, 34.52 + 27.261239 * k),
      (0.000042, 207.19 + 0.121824 * k),
      (0.000040, 291.34 + 1.844379 * k),
      (0.000037, 161.72 + 24.198154 * k),
      (0.000035, 239.56 + 25.513099 * k),
      (0.000023, 331.55 + 3.592518 * k),
  )
  jde += sum(c * sin(rad(a)) for c, a in planetary)
  return jde


# ---------------------------------------------------------------------------
# 农历编排
# ---------------------------------------------------------------------------


class AstroLunarCalendarBuilder:
  """按 GB/T 33661-2017 规则从天文数据编排农历月。"""

  def __init__(self, first_year: int, last_year: int):
    self.first_year = first_year
    self.last_year = last_year
    # 所需的天象范围：前一年冬至 ~ 后一年冬至，再留一点余量
    self.new_moons = []  # [(北京时间 JD, 日期)]
    k = math.floor(
        (
            datetime.date(first_year - 2, 10, 1).toordinal()
            + ORD_TO_JD
            - 2451550.1
        )
        / 29.530588861
    )
    end = datetime.date(last_year + 2, 3, 1)
    while True:
      jd = td_to_beijing_jd(new_moon_jde(k))
      d = beijing_jd_to_date(jd)
      if d > end:
        break
      self.new_moons.append((jd, d))
      k += 1
    self.new_moon_dates = [d for _, d in self.new_moons]

    # 二十四节气（北京时间）。中气 = 视黄经为 30° 整数倍者（偶数下标）。
    self.solar_terms = {}  # date -> 名称
    self.solar_term_times = []  # [(北京时间 JD, 名称)]
    zhongqi = []
    for year in range(first_year - 2, last_year + 2):
      for i in range(24):
        jd = td_to_beijing_jd(solar_term_jde(year, i * 15.0))
        d = beijing_jd_to_date(jd)
        self.solar_terms[d] = SOLAR_TERM_NAMES[i]
        self.solar_term_times.append((jd, SOLAR_TERM_NAMES[i]))
        if i % 2 == 0:
          zhongqi.append(d)
    self.solar_term_times.sort()
    self.zhongqi_dates = sorted(zhongqi)

  def winter_solstice(self, year: int) -> datetime.date:
    """year 年冬至（视黄经 270°）的北京日期。"""
    return beijing_jd_to_date(td_to_beijing_jd(solar_term_jde(year, 270.0)))

  def _month_index_containing(self, d: datetime.date) -> int:
    """返回包含日期 d 的农历月在 new_moons 中的下标（朔日 <= d 的最后一个）。"""
    return bisect.bisect_right(self.new_moon_dates, d) - 1

  def _has_zhongqi(self, i: int) -> bool:
    """第 i 个农历月 [朔日_i, 朔日_{i+1}) 中是否含有中气。"""
    start, end = self.new_moon_dates[i], self.new_moon_dates[i + 1]
    j = bisect.bisect_left(self.zhongqi_dates, start)
    return j < len(self.zhongqi_dates) and self.zhongqi_dates[j] < end

  def build(self):
    """返回 (月份列表, 节气字典)。

    以"岁"（冬至所在的十一月 ~ 下一个十一月之前）为单位编排月序，
    再按正月初一切分到农历年。
    """
    raw = []  # [(朔日下标, 月序 1..12, 是否闰月)]
    for year in range(self.first_year - 2, self.last_year + 1):
      a = self._month_index_containing(self.winter_solstice(year))
      b = self._month_index_containing(self.winter_solstice(year + 1))
      n_months = b - a  # 12 或 13
      leap_index = None
      if n_months == 13:
        for i in range(a + 1, b):
          if not self._has_zhongqi(i):
            leap_index = i
            break
      elif n_months != 12:
        raise AssertionError(f"{year} 年冬至到次年冬至有 {n_months} 个月？")
      month = 11
      for i in range(a, b):
        if i == leap_index:
          raw.append((i, month, True))
        else:
          month = month % 12 + 1 if i != a else 11
          raw.append((i, month, False))

    # 给每个月标注所属农历年：正月（非闰）开启新的一年
    months = []
    lunar_year = None
    for i, month, leap in raw:
      start = self.new_moon_dates[i]
      if month == 1 and not leap:
        lunar_year = start.year
      if lunar_year is None:
        continue  # 第一个正月之前的残月不需要
      days = (self.new_moon_dates[i + 1] - start).days
      months.append(
          lunar_common.LunarMonth(lunar_year, month, leap, start, days)
      )
    return months, self.solar_terms

  def near_midnight_warnings(self, minutes: float = 10.0):
    """列出朔或中气时刻距午夜很近的情况（这类日期对计算误差最敏感）。"""
    lo = datetime.date(self.first_year - 1, 11, 1)
    hi = datetime.date(self.last_year + 1, 1, 31)
    events = [(jd, "朔") for jd, _ in self.new_moons]
    events += [
        (jd, n)
        for jd, n in self.solar_term_times
        if SOLAR_TERM_NAMES.index(n) % 2 == 0
    ]
    result = []
    for jd, name in sorted(events):
      frac = (jd - ORD_TO_JD) % 1.0
      dist = min(frac, 1 - frac) * 1440
      if dist < minutes and lo <= beijing_jd_to_date(jd) <= hi:
        result.append((beijing_jd_to_datetime(jd), name, dist))
    return result


def build_calendar() -> lunar_common.LunarCalendar:
  builder = AstroLunarCalendarBuilder(
      lunar_common.FIRST_YEAR, lunar_common.LAST_YEAR
  )
  months, terms = builder.build()
  return lunar_common.LunarCalendar(months, terms)


if __name__ == "__main__":
  if len(sys.argv) > 1 and sys.argv[1] == "--warnings":
    b = AstroLunarCalendarBuilder(
        lunar_common.FIRST_YEAR, lunar_common.LAST_YEAR
    )
    for when, name, dist in b.near_midnight_warnings():
      print(f"{when:%Y-%m-%d %H:%M:%S}  {name}  距午夜 {dist:.1f} 分钟")
  else:
    lunar_common.main(
        build_calendar(), "用天文算法推算 2000~2100 年农历（版本 2）"
    )
