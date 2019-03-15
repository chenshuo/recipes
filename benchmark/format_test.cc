#include "format.h"
#include "gtest/gtest.h"

namespace {

using ::testing::Eq;  // Optional ::testing aliases. Remove if unused.
using ::testing::Test;

TEST(FormatTest, Human)
{
  setlocale(LC_NUMERIC, "");
  size_t numbers[] = {
    0, 999, 1000,
    9990, 9994,
    9995, 10049,
    10'050, 10'149,
    10'150, 10'249,
    99'949, 99'950,
    100'000, 100'499,
    999'499, 999'500,
    9'994'999, 9'995'000,
    9'995'001, 9'995'002,
    999'000'000, 999'500'000-1,
    999'500'500,
    1'000'000'000,
    10'000'000'000,
    100'000'000'000,
    1000'000'000'000,
    10'000'000'000'000,
    100'000'000'000'000,
    1'000'000'000'000'000,
    10'000'000'000'000'000,
    100'000'000'000'000'000,
    1000'000'000'000'000'000,
    10'000'000'000'000'000'000U,
    INT64_MAX,
    UINT64_MAX,
  };
  for (size_t n : numbers) {
    printf("%'32zu  '%5s'\n", n, formatSI(n).c_str());
  }
}

TEST(FormatTest, SI) {
  EXPECT_EQ("0", formatSI(0));
  EXPECT_EQ("12.3k", formatSI(12345));
  EXPECT_EQ("23.5k", formatSI(23456));
  EXPECT_EQ("100k", formatSI(99999));
  EXPECT_EQ("123k", formatSI(123456));
  EXPECT_EQ("235k", formatSI(234567));
  EXPECT_EQ("1.23M", formatSI(1234567));
  EXPECT_EQ("9.99M", formatSI(9990000));
  EXPECT_EQ("9.99M", formatSI(9995000-1));
  EXPECT_EQ("10.0M", formatSI(9995000));
  EXPECT_EQ("10.0M", formatSI(10000000-1));
  EXPECT_EQ("10.0M", formatSI(10000000));
  EXPECT_EQ("10.0M", formatSI(10049999));
  EXPECT_EQ("10.1M", formatSI(10050000));
  EXPECT_EQ("10.1M", formatSI(10150000-1));
  EXPECT_EQ("10.2M", formatSI(10150000));
  EXPECT_EQ("12.3M", formatSI(12345678));
  EXPECT_EQ("99.9M", formatSI(99900000));
  EXPECT_EQ("99.9M", formatSI(99950000-1));
  EXPECT_EQ("100M", formatSI(99950000));
  EXPECT_EQ("100M", formatSI(100*1000*1000));
  EXPECT_EQ("9.99G", formatSI( 9'990'000'000));
  EXPECT_EQ("9.99G", formatSI( 9'995'000'000-1));
  EXPECT_EQ("10.0G", formatSI( 9'995'000'000));
  EXPECT_EQ("99.9G", formatSI(99'950'000'000-1));
  EXPECT_EQ("100G", formatSI(99'950'000'000));
  EXPECT_EQ("100G", formatSI(100'000'000'000));
}

TEST(FormatTest, HumanIEC)
{
  setlocale(LC_NUMERIC, "");
  size_t numbers[] = {
    0, 1023,
    1024, 9999,
    10229, 10230,
    10232, 10233, 10234, 10235,
    10240-1, 10240,
    102347,
    102348,  // 99.9492 Ki
    102349,  // 99.9502 Ki

    1024*999,
    1023487,
    1023488,
    1048063,
    1048064,
    1024*1000,
    1024*1024-1,
    1024*1024,

    10480517,
    10480518,   // 9.995 Mi
    10*1024*1024-1,
    10*1024*1024,

    104805171,
    104805172,  // 99.95 Mi

    1048051711,
    1048051712,
    1048051713,

    1073217535,
    1073217536,

    1073741824-1,
    1073741824,

    10732049530,
    10732049531,
    107320495308,
    107320495309,

    1099511627776 - 1,
    1099511627776,

    10989618719621,
    10989618719622,

    109896187196211,
    109896187196212,
    INT64_MAX,
    UINT64_MAX,
  };
  for (size_t n : numbers) {
    printf("%'32zu  '%6s'\n", n, formatIEC(n).c_str());
  }
}

}  // namespace
