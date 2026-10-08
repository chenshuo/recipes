"""农历日历的公共部分：数据结构、名称、日期换算与输出格式。

两个版本（查表法 lunar_table.py、天文算法 lunar_astro.py）都只负责产生
"农历月序列"（每个月的起始公历日期、月序号、是否闰月、天数），
其余的查询与打印逻辑都在这里，保证两版输出格式完全一致、可以直接 diff。

只使用 Python 标准库。
"""

import argparse
import bisect
import calendar
import dataclasses
import datetime
import sys

FIRST_YEAR = 2000
LAST_YEAR = 2100

TIANGAN = "甲乙丙丁戊己庚辛壬癸"
DIZHI = "子丑寅卯辰巳午未申酉戌亥"
SHENGXIAO = "鼠牛虎兔龙蛇马羊猴鸡狗猪"
MONTH_NAMES = [
    "正",
    "二",
    "三",
    "四",
    "五",
    "六",
    "七",
    "八",
    "九",
    "十",
    "冬",
    "腊",
]
MONTH_NAMES_PLAIN = [
    "正",
    "二",
    "三",
    "四",
    "五",
    "六",
    "七",
    "八",
    "九",
    "十",
    "十一",
    "十二",
]
DAY_NAMES = [
    "初一",
    "初二",
    "初三",
    "初四",
    "初五",
    "初六",
    "初七",
    "初八",
    "初九",
    "初十",
    "十一",
    "十二",
    "十三",
    "十四",
    "十五",
    "十六",
    "十七",
    "十八",
    "十九",
    "二十",
    "廿一",
    "廿二",
    "廿三",
    "廿四",
    "廿五",
    "廿六",
    "廿七",
    "廿八",
    "廿九",
    "三十",
]


@dataclasses.dataclass(frozen=True)
class LunarMonth:
  """一个农历月。"""

  year: int  # 所属农历年（以正月初一为岁首）
  month: int  # 1..12
  leap: bool  # 是否闰月
  start: datetime.date  # 初一对应的公历日期
  days: int  # 29 或 30

  @property
  def name(self) -> str:
    return (
        ("闰" if self.leap else "") + MONTH_NAMES_PLAIN[self.month - 1] + "月"
    )


@dataclasses.dataclass(frozen=True)
class LunarDate:
  year: int
  month: int
  day: int
  leap: bool

  def __str__(self) -> str:
    leap = "闰" if self.leap else ""
    return (
        f"{ganzhi_year(self.year)}年"
        f" {leap}{MONTH_NAMES_PLAIN[self.month - 1]}月{DAY_NAMES[self.day - 1]}"
    )


def ganzhi_year(year: int) -> str:
  """农历年的干支，例如 2024 -> 甲辰。"""
  return TIANGAN[(year - 4) % 10] + DIZHI[(year - 4) % 12]


def shengxiao(year: int) -> str:
  return SHENGXIAO[(year - 4) % 12]


class LunarCalendar:
  """由连续的农历月序列构成的日历，可做公历 -> 农历查询。"""

  def __init__(self, months, solar_terms=None):
    """Args.

    months: 按时间顺序、首尾相接的 LunarMonth 列表。
    solar_terms: 可选，{datetime.date: 节气名}，用于在日历中标注节气。
    """
    self.months = list(months)
    for a, b in zip(self.months, self.months[1:]):
      if a.start + datetime.timedelta(days=a.days) != b.start:
        raise ValueError(f"月份不连续: {a} -> {b}")
    self._starts = [m.start for m in self.months]
    self.solar_terms = solar_terms or {}

  def month_of(self, d: datetime.date) -> LunarMonth:
    i = bisect.bisect_right(self._starts, d) - 1
    if i < 0 or d >= self.months[i].start + datetime.timedelta(
        days=self.months[i].days
    ):
      raise ValueError(f"{d} 超出日历范围")
    return self.months[i]

  def lunar_date(self, d: datetime.date) -> LunarDate:
    m = self.month_of(d)
    return LunarDate(m.year, m.month, (d - m.start).days + 1, m.leap)

  def months_of_year(self, year: int):
    return [m for m in self.months if m.year == year]

  def leap_month(self, year: int) -> int:
    for m in self.months_of_year(year):
      if m.leap:
        return m.month
    return 0


# ---------------------------------------------------------------------------
# 输出
# ---------------------------------------------------------------------------


def print_year_summary(cal: LunarCalendar, year: int, out=sys.stdout):
  """打印一个农历年：正月初一、闰月、各月大小及初一日期。"""
  months = cal.months_of_year(year)
  leap = cal.leap_month(year)
  leap_str = f"闰{MONTH_NAMES_PLAIN[leap - 1]}月" if leap else "无闰月"
  total = sum(m.days for m in months)
  print(
      f"农历 {year} {ganzhi_year(year)}年（{shengxiao(year)}）  "
      f"正月初一 = {months[0].start.isoformat()}  {leap_str}  "
      f"全年 {len(months)} 个月 {total} 天",
      file=out,
  )
  for m in months:
    size = "大" if m.days == 30 else "小"
    print(
        f"    {_cjk_ljust(m.name, 10)}{size}  初一 = {m.start.isoformat()}",
        file=out,
    )


def daily_label(cal: LunarCalendar, d: datetime.date) -> str:
  """用于月历格子里的简短标签：节气 > 月名(初一) > 日名。"""
  if d in cal.solar_terms:
    return cal.solar_terms[d]
  m = cal.month_of(d)
  day = (d - m.start).days + 1
  if day == 1:
    return (
        "闰" + MONTH_NAMES[m.month - 1]
        if m.leap
        else MONTH_NAMES[m.month - 1] + "月"
    )
  return DAY_NAMES[day - 1]


def _cjk_ljust(s: str, width: int) -> str:
  w = sum(2 if ord(c) > 0x2E80 else 1 for c in s)
  return s + " " * max(0, width - w)


def print_gregorian_year_grid(cal: LunarCalendar, gyear: int, out=sys.stdout):
  """打印某公历年的月历网格，每个日期下方标注农历。"""
  cell = 6
  for gmonth in range(1, 13):
    first = datetime.date(gyear, gmonth, 1)
    lunar = cal.lunar_date(first)
    print(
        f"\n{gyear}年{gmonth}月    （农历 {ganzhi_year(lunar.year)}年 "
        f"{shengxiao(lunar.year)}）",
        file=out,
    )
    print(
        "".join(
            _cjk_ljust(w, cell)
            for w in ["一", "二", "三", "四", "五", "六", "日"]
        ),
        file=out,
    )
    for week in calendar.Calendar(firstweekday=0).monthdayscalendar(
        gyear, gmonth
    ):
      line1 = "".join(_cjk_ljust(str(d) if d else "", cell) for d in week)
      line2 = "".join(
          _cjk_ljust(
              daily_label(cal, datetime.date(gyear, gmonth, d)) if d else "",
              cell,
          )
          for d in week
      )
      print(line1.rstrip(), file=out)
      print(line2.rstrip(), file=out)


def print_daily(
    cal: LunarCalendar, start: datetime.date, end: datetime.date, out=sys.stdout
):
  d = start
  one = datetime.timedelta(days=1)
  while d <= end:
    term = cal.solar_terms.get(d, "")
    print(f"{d.isoformat()}  {cal.lunar_date(d)}  {term}".rstrip(), file=out)
    d += one


def main(cal: LunarCalendar, description: str, argv=None):
  parser = argparse.ArgumentParser(description=description)
  group = parser.add_mutually_exclusive_group()
  group.add_argument(
      "--summary",
      action="store_true",
      help="（默认）按农历年列出 2000~2100 各年正月初一、闰月和各月大小",
  )
  group.add_argument(
      "--grid",
      type=int,
      metavar="YEAR",
      help="打印某个公历年的月历网格（附农历）",
  )
  group.add_argument(
      "--daily",
      action="store_true",
      help="逐日列出 2000-01-01 ~ 2100-12-31 的农历日期",
  )
  group.add_argument("--date", metavar="YYYY-MM-DD", help="查询单个日期")
  args = parser.parse_args(argv)

  if args.grid is not None:
    print_gregorian_year_grid(cal, args.grid)
  elif args.daily:
    print_daily(
        cal, datetime.date(FIRST_YEAR, 1, 1), datetime.date(LAST_YEAR, 12, 31)
    )
  elif args.date:
    d = datetime.date.fromisoformat(args.date)
    term = cal.solar_terms.get(d, "")
    print(f"{d.isoformat()}  {cal.lunar_date(d)}  {term}".rstrip())
  else:
    for year in range(FIRST_YEAR, LAST_YEAR + 1):
      print_year_summary(cal, year)
