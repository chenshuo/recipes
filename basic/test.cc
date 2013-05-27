#include "uint.h"
#include <stdio.h>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using std::string;

BOOST_AUTO_TEST_CASE(test_initialize_word)
{
  UnsignedInt u0;
  BOOST_CHECK_EQUAL(u0.getValue().size(), 0);
  BOOST_CHECK_EQUAL(u0.toHex(), string("0"));
  BOOST_CHECK_EQUAL(u0.toDec(), string("0"));

  UnsignedInt u1(1);
  BOOST_CHECK_EQUAL(u1.getValue().size(), 1);
  BOOST_CHECK_EQUAL(u1.getValue()[0], 1);
  BOOST_CHECK_EQUAL(u1.toHex(), string("1"));
  BOOST_CHECK_EQUAL(u1.toDec(), string("1"));

  UnsignedInt u2(-1);
  BOOST_CHECK_EQUAL(u2.getValue().size(), 1);
  BOOST_CHECK_EQUAL(u2.getValue()[0], 0xFFFFFFFF);
  BOOST_CHECK_EQUAL(u2.toHex(), string("ffffffff"));
  BOOST_CHECK_EQUAL(u2.toDec(), string("4294967295"));

  UnsignedInt u3(0xdeadbeef);
  BOOST_CHECK_EQUAL(u3.getValue().size(), 1);
  BOOST_CHECK_EQUAL(u3.getValue()[0], 0xdeadbeef);
  BOOST_CHECK_EQUAL(u3.toHex(), string("deadbeef"));

  UnsignedInt u4(100);
  BOOST_CHECK_EQUAL(u4.getValue().size(), 1);
  BOOST_CHECK_EQUAL(u4.getValue()[0], 0x64);
  BOOST_CHECK_EQUAL(u4.toHex(), string("64"));
}

BOOST_AUTO_TEST_CASE(test_initialize_string)
{
  UnsignedInt u0("");
  UnsignedInt u1("0");
  UnsignedInt u2("1");
  UnsignedInt u3("123456789");
  BOOST_CHECK_EQUAL(u3.getValue().size(), 1);
  BOOST_CHECK_EQUAL(u3.getValue()[0], 123456789);

  for (int i = 1; i < 2000; ++i)
  {
    string nines(i, '9');
    UnsignedInt u9(nines);
    BOOST_CHECK_EQUAL(u9.toDec(), nines);
    // const UnsignedInt::value_type& v = u9.getValue();
    // printf("%3zd %3zd %zd\n", v.size(), v.capacity(), v.capacity()-v.size());
    // printf("%zd ", v.capacity()-v.size());
  }

  UnsignedInt h("", UnsignedInt::kHex);
  BOOST_CHECK_EQUAL(h.getValue().size(), 0);

  UnsignedInt h0("0", UnsignedInt::kHex);
  BOOST_CHECK_EQUAL(h0.getValue().size(), 0);

  UnsignedInt h1("1", UnsignedInt::kHex);
  BOOST_CHECK_EQUAL(h1.getValue().size(), 1);
  BOOST_CHECK_EQUAL(h1.getValue()[0], 1);

  UnsignedInt h2("12345678", UnsignedInt::kHex);
  BOOST_CHECK_EQUAL(h2.getValue().size(), 1);
  BOOST_CHECK_EQUAL(h2.getValue()[0], 0x12345678);

  UnsignedInt h3("abcdef98", UnsignedInt::kHex);
  BOOST_CHECK_EQUAL(h3.getValue().size(), 1);
  BOOST_CHECK_EQUAL(h3.getValue()[0], 0xabcdef98);

  UnsignedInt h4("ffffffff", UnsignedInt::kHex);
  BOOST_CHECK_EQUAL(h4.getValue().size(), 1);
  BOOST_CHECK_EQUAL(h4.getValue()[0], 0xffffffff);

  UnsignedInt h5("abcdef980", UnsignedInt::kHex);
  BOOST_CHECK_EQUAL(h5.getValue().size(), 2);
  BOOST_CHECK_EQUAL(h5.getValue()[0], 0xbcdef980);
  BOOST_CHECK_EQUAL(h5.getValue()[1], 0xa);

  UnsignedInt h6("fffffffff", UnsignedInt::kHex);
  BOOST_CHECK_EQUAL(h6.getValue().size(), 2);
  BOOST_CHECK_EQUAL(h6.getValue()[0], 0xffffffff);
  BOOST_CHECK_EQUAL(h6.getValue()[1], 0xf);

  for (int i = 1; i < 2000; ++i)
  {
    string input(i, 'f');
    UnsignedInt u9(input, UnsignedInt::kHex);
    BOOST_CHECK_EQUAL(u9.toHex(), input);
  }
}

BOOST_AUTO_TEST_CASE(test_lessthan)
{
  UnsignedInt u0;
  BOOST_CHECK_EQUAL(u0.lessThan(u0), false);

  UnsignedInt u1(1);
  BOOST_CHECK_EQUAL(u1.lessThan(u1), false);
  BOOST_CHECK_EQUAL(u0.lessThan(u1), true);
  BOOST_CHECK_EQUAL(u1.lessThan(u0), false);

  UnsignedInt u2(2);
  BOOST_CHECK_EQUAL(u1.lessThan(u2), true);
  BOOST_CHECK_EQUAL(u2.lessThan(u1), false);

  UnsignedInt u3("100000000", UnsignedInt::kHex);
  BOOST_CHECK_EQUAL(u3.getValue().size(), 2);
  BOOST_CHECK_EQUAL(u1.lessThan(u3), true);
  BOOST_CHECK_EQUAL(u3.lessThan(u1), false);

  UnsignedInt u4("100000001", UnsignedInt::kHex);
  BOOST_CHECK_EQUAL(u3.lessThan(u4), true);
  BOOST_CHECK_EQUAL(u4.lessThan(u3), false);

  UnsignedInt u5("100000002", UnsignedInt::kHex);
  UnsignedInt u6("200000001", UnsignedInt::kHex);
  BOOST_CHECK_EQUAL(u5.lessThan(u6), true);
  BOOST_CHECK_EQUAL(u6.lessThan(u5), false);
}

BOOST_AUTO_TEST_CASE(test_add_word)
{
  UnsignedInt u0;
  BOOST_CHECK_EQUAL(u0.getValue().size(), 0);
  u0.add(1);
  BOOST_CHECK_EQUAL(u0.getValue().size(), 1);
  BOOST_CHECK_EQUAL(u0.getValue()[0], 1);

  UnsignedInt u1(1);
  BOOST_CHECK_EQUAL(u1.getValue().size(), 1);
  BOOST_CHECK_EQUAL(u1.getValue()[0], 1);
  u1.add(1);
  BOOST_CHECK_EQUAL(u1.getValue().size(), 1);
  BOOST_CHECK_EQUAL(u1.getValue()[0], 2);

  UnsignedInt u2(2);
  u2.add(4294967295);
  BOOST_CHECK_EQUAL(u2.getValue().size(), 2);
  BOOST_CHECK_EQUAL(u2.getValue()[0], 1);
  BOOST_CHECK_EQUAL(u2.getValue()[1], 1);

  UnsignedInt u3(4294967295);
  u3.add(1);
  BOOST_CHECK_EQUAL(u3.getValue().size(), 2);
  BOOST_CHECK_EQUAL(u3.getValue()[0], 0);
  BOOST_CHECK_EQUAL(u3.getValue()[1], 1);

  UnsignedInt u4;
  u4.setValue(2, -1);
  u4.add(1);
  BOOST_CHECK_EQUAL(u4.getValue().size(), 3);
  BOOST_CHECK_EQUAL(u4.getValue()[0], 0);
  BOOST_CHECK_EQUAL(u4.getValue()[1], 0);
  BOOST_CHECK_EQUAL(u4.getValue()[2], 1);

  UnsignedInt u5;
  u4.setValue(3, -1);
  u4.add(1);
  BOOST_CHECK_EQUAL(u4.getValue().size(), 4);
  BOOST_CHECK_EQUAL(u4.getValue()[0], 0);
  BOOST_CHECK_EQUAL(u4.getValue()[1], 0);
  BOOST_CHECK_EQUAL(u4.getValue()[2], 0);
  BOOST_CHECK_EQUAL(u4.getValue()[3], 1);

  UnsignedInt u6("fffffffff", UnsignedInt::kHex);
  BOOST_CHECK_EQUAL(u6.getValue().size(), 2);
  u6.add(1);
  BOOST_CHECK_EQUAL(u6.getValue().size(), 2);
  BOOST_CHECK_EQUAL(u6.getValue()[0], 0);
  BOOST_CHECK_EQUAL(u6.getValue()[1], 16);

  UnsignedInt u7("10fffffffffffffff", UnsignedInt::kHex);
  BOOST_CHECK_EQUAL(u7.getValue().size(), 3);
  u7.add(1);
  BOOST_CHECK_EQUAL(u7.getValue().size(), 3);
  BOOST_CHECK_EQUAL(u7.getValue()[0], 0);
  BOOST_CHECK_EQUAL(u7.getValue()[1], 0x10000000);
  BOOST_CHECK_EQUAL(u7.getValue()[2], 1);
}

BOOST_AUTO_TEST_CASE(test_add_other)
{
  UnsignedInt u1(1);
  UnsignedInt u0(4294967294);

  u0.add(u1);
  BOOST_CHECK_EQUAL(u0.getValue().size(), 1);
  BOOST_CHECK_EQUAL(u0.getValue()[0], 4294967295);

  u0.add(u1);
  BOOST_CHECK_EQUAL(u0.getValue().size(), 2);
  BOOST_CHECK_EQUAL(u0.getValue()[0], 0);
  BOOST_CHECK_EQUAL(u0.getValue()[1], 1);

  u0.add(4294967295);
  u0.add(u1);
  BOOST_CHECK_EQUAL(u0.getValue().size(), 2);
  BOOST_CHECK_EQUAL(u0.getValue()[0], 0);
  BOOST_CHECK_EQUAL(u0.getValue()[1], 2);

  UnsignedInt u2("ffffffffffffffff", UnsignedInt::kHex);
  BOOST_CHECK_EQUAL(u2.getValue().size(), 2);
  u1.add(u2);
  BOOST_CHECK_EQUAL(u1.getValue().size(), 3);
}

BOOST_AUTO_TEST_CASE(test_sub)
{
  UnsignedInt u1(1);
  {
  UnsignedInt u0(2);
  u0.sub(u1);
  BOOST_CHECK_EQUAL(u0.getValue().size(), 1);
  BOOST_CHECK_EQUAL(u0.getValue()[0], 1);

  u0.sub(u1);
  BOOST_CHECK_EQUAL(u0.getValue().size(), 0);
  }

  {
  UnsignedInt u2(0xffffffff);
  u2.add(2);
  BOOST_CHECK_EQUAL(u2.getValue().size(), 2);
  BOOST_CHECK_EQUAL(u2.getValue()[0], 1);
  BOOST_CHECK_EQUAL(u2.getValue()[1], 1);

  u2.sub(u1);
  BOOST_CHECK_EQUAL(u2.getValue().size(), 2);
  BOOST_CHECK_EQUAL(u2.getValue()[0], 0);
  BOOST_CHECK_EQUAL(u2.getValue()[1], 1);

  u2.sub(u1);
  BOOST_CHECK_EQUAL(u2.getValue().size(), 1);
  BOOST_CHECK_EQUAL(u2.getValue()[0], 0xffffffff);

  u2.sub(u2);
  BOOST_CHECK_EQUAL(u2.getValue().size(), 0);
  }

  {
  UnsignedInt u3(0xffffffff);
  u3.add(2);
  u3.sub(u3);
  BOOST_CHECK_EQUAL(u3.getValue().size(), 0);
  }

  {
  UnsignedInt u4("10000000000000000", UnsignedInt::kHex);
  BOOST_CHECK_EQUAL(u4.getValue().size(), 3);
  u4.sub(1);
  BOOST_CHECK_EQUAL(u4.getValue().size(), 2);
  BOOST_CHECK_EQUAL(u4.getValue()[0], 0xffffffff);
  BOOST_CHECK_EQUAL(u4.getValue()[1], 0xffffffff);
  }
}

BOOST_AUTO_TEST_CASE(test_multiply_word)
{
  UnsignedInt u0;
  u0.multiply(0);
  BOOST_CHECK_EQUAL(u0.getValue().size(), 0);
  u0.multiply(1);
  BOOST_CHECK_EQUAL(u0.getValue().size(), 0);
  u0.multiply(2);
  BOOST_CHECK_EQUAL(u0.getValue().size(), 0);

  UnsignedInt u1(1);
  BOOST_CHECK_EQUAL(u1.getValue().size(), 1);
  BOOST_CHECK_EQUAL(u1.getValue()[0], 1);
  u1.multiply(10);
  BOOST_CHECK_EQUAL(u1.getValue().size(), 1);
  BOOST_CHECK_EQUAL(u1.getValue()[0], 10);
  u1.multiply(100000000);
  BOOST_CHECK_EQUAL(u1.getValue().size(), 1);
  BOOST_CHECK_EQUAL(u1.getValue()[0], 1000000000);
  u1.multiply(10);
  BOOST_CHECK_EQUAL(u1.getValue().size(), 2);
  BOOST_CHECK_EQUAL(u1.getValue()[0], 0x540be400);
  BOOST_CHECK_EQUAL(u1.getValue()[1], 2);
  BOOST_CHECK_EQUAL(u1.toHex(), string("2540be400"));

  UnsignedInt u2(-1);
  BOOST_CHECK_EQUAL(u2.getValue().size(), 1);
  BOOST_CHECK_EQUAL(u2.getValue()[0], 0xFFFFFFFF);
  u2.multiply(-1);
  BOOST_CHECK_EQUAL(u2.getValue().size(), 2);
  BOOST_CHECK_EQUAL(u2.getValue()[0], 1);
  BOOST_CHECK_EQUAL(u2.getValue()[1], 0xfffffffe);
  BOOST_CHECK_EQUAL(u2.toHex(), string("fffffffe00000001"));
  BOOST_CHECK_EQUAL(u2.toDec(), string("18446744065119617025"));
  u2.multiply(0xFFFFFFFF);
  BOOST_CHECK_EQUAL(u2.getValue().size(), 3);
  BOOST_CHECK_EQUAL(u2.getValue()[0], 0xFFFFFFFF);
  BOOST_CHECK_EQUAL(u2.getValue()[1], 2);
  BOOST_CHECK_EQUAL(u2.getValue()[2], 0xfffffffd);
  BOOST_CHECK_EQUAL(u2.toHex(), string("fffffffd00000002ffffffff"));
  BOOST_CHECK_EQUAL(u2.toDec(), string("79228162458924105385300197375"));
}

BOOST_AUTO_TEST_CASE(test_multiply_other)
{
  UnsignedInt u2(-1);

  u2.multiply(u2);
  BOOST_CHECK_EQUAL(u2.getValue().size(), 2);
  BOOST_CHECK_EQUAL(u2.getValue()[0], 1);
  BOOST_CHECK_EQUAL(u2.getValue()[1], 0xfffffffe);

  u2.multiply(u2);
  BOOST_CHECK_EQUAL(u2.getValue().size(), 4);
  BOOST_CHECK_EQUAL(u2.getValue()[0], 1);
  BOOST_CHECK_EQUAL(u2.getValue()[1], 0xfffffffc);
  BOOST_CHECK_EQUAL(u2.getValue()[2], 5);
  BOOST_CHECK_EQUAL(u2.getValue()[1], 0xfffffffc);
}

BOOST_AUTO_TEST_CASE(test_devide_word)
{
  UnsignedInt u0;
  BOOST_CHECK_EQUAL(u0.devide(1), 0);
  BOOST_CHECK_EQUAL(u0.getValue().size(), 0);

  UnsignedInt u1(1000000009);
  BOOST_CHECK_EQUAL(u1.devide(10), 9);
  BOOST_CHECK_EQUAL(u1.getValue()[0], 100000000);

  UnsignedInt u2(-1);
  BOOST_CHECK_EQUAL(u2.devide(10), 5);
  BOOST_CHECK_EQUAL(u2.getValue()[0], 0x19999999);

  UnsignedInt u3(-1);
  u3.multiply(u3);
  BOOST_CHECK_EQUAL(u3.devide(10), 5);
  BOOST_CHECK_EQUAL(u3.getValue()[0], 0x66666666);
  BOOST_CHECK_EQUAL(u3.getValue()[1], 0x19999999);
  BOOST_CHECK_EQUAL(u3.toDec(), string("1844674406511961702"));
  BOOST_CHECK_EQUAL(u3.devide(10), 2);
  BOOST_CHECK_EQUAL(u3.toHex(), string("28f5c28f0a3d70a"));
  BOOST_CHECK_EQUAL(u3.toDec(), string("184467440651196170"));

  UnsignedInt u4(-1);
  u4.multiply(11);
  BOOST_CHECK_EQUAL(u4.getValue().size(), 2);
  BOOST_CHECK_EQUAL(u4.devide(12), 9);
  BOOST_CHECK_EQUAL(u4.getValue().size(), 1);
  BOOST_CHECK_EQUAL(u4.getValue()[0], 0xeaaaaaa9);

  // FIXME: more
}
