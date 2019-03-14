#include "format.h"
#include "gtest/gtest.h"

namespace {

using ::testing::Eq;  // Optional ::testing aliases. Remove if unused.
using ::testing::Test;

TEST(FormatTest, SI) {
  EXPECT_EQ("0", formatSI(0));
  EXPECT_EQ("12345", formatSI(12345));
  EXPECT_EQ("99999", formatSI(99999));
  EXPECT_EQ("123k", formatSI(123456));
  EXPECT_EQ("235k", formatSI(234567));
  EXPECT_EQ("1235k", formatSI(1234567));
  EXPECT_EQ("10.0M", formatSI(10000000-1));
  EXPECT_EQ("10.0M", formatSI(10000000));
  EXPECT_EQ("10.0M", formatSI(10049999));
  EXPECT_EQ("10.1M", formatSI(10050000));
  EXPECT_EQ("12.3M", formatSI(12345678));
}

}  // namespace
