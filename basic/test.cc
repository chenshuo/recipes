#include "uint.h"

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using std::string;

BOOST_AUTO_TEST_CASE(test1)
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

BOOST_AUTO_TEST_CASE(test2)
{
  UnsignedInt u0("0");
  UnsignedInt u1("1");
  UnsignedInt u2("123456789");
}

BOOST_AUTO_TEST_CASE(test3)
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

BOOST_AUTO_TEST_CASE(test4)
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

BOOST_AUTO_TEST_CASE(test5)
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
