#!/bin/sh

diff_a_file()
{
  diff $1 $2 -q |grep differ |grep -v Makefile|awk '{\
    split($2, old, "/"); \
    split($4, new, "/"); \
    print "diff -U200", $2, $4, "| awk \" NR>3 {print}\" >", new[1]"-"old[1]"-"new[2]".diff"}'
  echo
}

diff_a_file s00 s01
diff_a_file s01 s02
diff_a_file s02 s03
diff_a_file s03 s04
diff_a_file s04 s05
diff_a_file s05 s06
diff_a_file s06 s07
diff_a_file s07 s08
diff_a_file s08 s09
diff_a_file s09 s10
diff_a_file s10 s11
diff_a_file s11 s12
diff_a_file s12 s13
