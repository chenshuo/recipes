#!/usr/bin/env python3
"""把 lunar_table.py / lunar_astro.py 的结果与参考数据逐日比对。

参考数据：https://github.com/hungtcs/traditional-chinese-calendar-database
的 database/all.csv（源自香港天文台 1901~2100 年公历与农历对照表）。
本脚本不联网，需要先自行下载该 CSV：

  python3 verify_against_reference.py all.csv
"""

import csv
import datetime
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import lunar_astro  # pylint: disable=g-import-not-at-top
import lunar_common  # pylint: disable=g-import-not-at-top
import lunar_table  # pylint: disable=g-import-not-at-top

_T2S = str.maketrans("閏穀處驚蟄滿種", "闰谷处惊蛰满种")


def load_reference(path):
  ref = {}
  with open(path, encoding="utf-8-sig") as f:
    for row in csv.DictReader(f):
      try:
        d = datetime.date(
            int(row["gregorianYear"]),
            int(row["gregorianMonth"]),
            int(row["gregorianDate"]),
        )
      except (TypeError, ValueError):
        continue
      if not lunar_common.FIRST_YEAR <= d.year <= lunar_common.LAST_YEAR:
        continue
      month = row["lunarMonth"].translate(_T2S)
      ref[d] = (
          month,
          row["lunarDate"],
          (row["solarTerm"] or "").translate(_T2S),
      )
  # 参考数据按公历年分文件生成，跨年时会丢失"闰"字（例如 2034-01-01~19 实为
  # 闰十一月，参考数据写成十一月）。同一农历月内沿用前一天的月名加以修正。
  for d in sorted(ref):
    prev = ref.get(d - datetime.timedelta(days=1))
    month, day, term = ref[d]
    if prev and day != "初一" and prev[0] == "闰" + month:
      ref[d] = (prev[0], day, term)
  return ref


def describe(cal, d):
  m = cal.month_of(d)
  return (
      m.name,
      lunar_common.DAY_NAMES[(d - m.start).days],
      cal.solar_terms.get(d, ""),
  )


def compare(name, cal, ref, check_terms):
  bad = 0
  for d in sorted(ref):
    got = describe(cal, d)
    want = ref[d]
    if not check_terms:
      got, want = got[:2], want[:2]
    if got != want:
      bad += 1
      if bad <= 20:
        print(f"  [{name}] {d}: 计算={got} 参考={want}")
  print(f"{name}: 比对 {len(ref)} 天，不一致 {bad} 天")
  return bad


def main():
  if len(sys.argv) != 2:
    sys.exit(__doc__)
  ref = load_reference(sys.argv[1])
  bad = compare("版本1 查表法", lunar_table.build_calendar(), ref, False)
  bad += compare("版本2 天文算法", lunar_astro.build_calendar(), ref, True)
  sys.exit(1 if bad else 0)


if __name__ == "__main__":
  main()
