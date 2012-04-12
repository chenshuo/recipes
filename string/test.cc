#include "StringEager.h"

#define BOOST_TEST_MAIN
#ifdef BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#else
#include <boost/test/included/unit_test.hpp>
#endif

typedef muduo::StringEager String;

BOOST_AUTO_TEST_CASE(testEmptyString)
{
  String s1;
  BOOST_CHECK_EQUAL(s1.empty(), true);
  BOOST_CHECK_EQUAL(s1.size(), 0);
  BOOST_CHECK_EQUAL(s1.capacity(), 0);
  BOOST_CHECK_EQUAL(s1.begin(), s1.end());
  BOOST_CHECK_EQUAL(strcmp(s1.c_str(), ""), 0);

  const String s2;
  BOOST_CHECK_EQUAL(s2.empty(), true);
  BOOST_CHECK_EQUAL(s2.size(), 0);
  BOOST_CHECK_EQUAL(s2.capacity(), 0);
  BOOST_CHECK_EQUAL(s2.begin(), s2.end());
  BOOST_CHECK_EQUAL(strcmp(s2.c_str(), ""), 0);
}

BOOST_AUTO_TEST_CASE(testCopyAndAssignment)
{
  String empty;
  String s1(empty);
  BOOST_CHECK_EQUAL(s1.empty(), true);
  BOOST_CHECK_EQUAL(s1.size(), 0);
  BOOST_CHECK_EQUAL(s1.capacity(), 0);
  BOOST_CHECK_EQUAL(s1.begin(), s1.end());
  BOOST_CHECK_EQUAL(strcmp(s1.c_str(), ""), 0);

  String s2("chenshuo");
  BOOST_CHECK_EQUAL(s2.empty(), false);
  BOOST_CHECK_EQUAL(s2.size(), 8);
  BOOST_CHECK_EQUAL(s2.capacity(), 15);
  BOOST_CHECK_EQUAL(s2.end() - s2.begin(), 8);
  BOOST_CHECK_EQUAL(strcmp(s2.c_str(), "chenshuo"), 0);

  String s3(s2);
  BOOST_CHECK(s2.data() != s3.data());
  BOOST_CHECK_EQUAL(s3.empty(), false);
  BOOST_CHECK_EQUAL(s3.size(), 8);
  BOOST_CHECK_EQUAL(s3.capacity(), 15);
  BOOST_CHECK_EQUAL(s3.end() - s3.begin(), 8);
  BOOST_CHECK_EQUAL(strcmp(s3.c_str(), "chenshuo"), 0);

  String s4 = s2;
  BOOST_CHECK(s2.data() != s4.data());
  BOOST_CHECK_EQUAL(s4.empty(), false);
  BOOST_CHECK_EQUAL(s4.size(), 8);
  BOOST_CHECK_EQUAL(s4.capacity(), 15);
  BOOST_CHECK_EQUAL(s4.end() - s4.begin(), 8);
  BOOST_CHECK_EQUAL(strcmp(s4.c_str(), "chenshuo"), 0);

  const char* olds3 = s3.data();
  s3 = empty;
  BOOST_CHECK_EQUAL(s3.data(), olds3);
  BOOST_CHECK_EQUAL(s3.empty(), true);
  BOOST_CHECK_EQUAL(s3.size(), 0);
  BOOST_CHECK_EQUAL(s3.capacity(), 15);
  BOOST_CHECK_EQUAL(s3.end() - s3.begin(), 0);
  BOOST_CHECK_EQUAL(strcmp(s3.c_str(), ""), 0);

  s3 = s2;
  BOOST_CHECK(s2.data() != s3.data());
  BOOST_CHECK_EQUAL(s3.data(), olds3);
  BOOST_CHECK_EQUAL(s3.empty(), false);
  BOOST_CHECK_EQUAL(s3.size(), 8);
  BOOST_CHECK_EQUAL(s3.capacity(), 15);
  BOOST_CHECK_EQUAL(s3.end() - s3.begin(), 8);
  BOOST_CHECK_EQUAL(strcmp(s3.c_str(), "chenshuo"), 0);

  s3 = s3;
  BOOST_CHECK_EQUAL(s3.data(), olds3);
  BOOST_CHECK_EQUAL(s3.empty(), false);
  BOOST_CHECK_EQUAL(s3.size(), 8);
  BOOST_CHECK_EQUAL(s3.capacity(), 15);
  BOOST_CHECK_EQUAL(s3.end() - s3.begin(), 8);
  BOOST_CHECK_EQUAL(strcmp(s3.c_str(), "chenshuo"), 0);

  s3 = "muduo";
  BOOST_CHECK_EQUAL(s3.data(), olds3);
  BOOST_CHECK_EQUAL(s3.empty(), false);
  BOOST_CHECK_EQUAL(s3.size(), 5);
  BOOST_CHECK_EQUAL(s3.capacity(), 15);
  BOOST_CHECK_EQUAL(s3.end() - s3.begin(), 5);
  BOOST_CHECK_EQUAL(strcmp(s3.c_str(), "muduo"), 0);

  s3 = "chenshuo.com";
  BOOST_CHECK_EQUAL(s3.data(), olds3);
  BOOST_CHECK_EQUAL(s3.empty(), false);
  BOOST_CHECK_EQUAL(s3.size(), 12);
  BOOST_CHECK_EQUAL(s3.capacity(), 15);
  BOOST_CHECK_EQUAL(s3.end() - s3.begin(), 12);
  BOOST_CHECK_EQUAL(strcmp(s3.c_str(), "chenshuo.com"), 0);

  s3 = "https://github.com/chenshuo/documents/downloads";
  BOOST_CHECK(s3.data() != olds3);
  BOOST_CHECK_EQUAL(s3.empty(), false);
  BOOST_CHECK_EQUAL(s3.size(), 47);
  BOOST_CHECK_EQUAL(s3.capacity(), 47);
  BOOST_CHECK_EQUAL(s3.end() - s3.begin(), 47);
  BOOST_CHECK_EQUAL(strcmp(s3.c_str(), "https://github.com/chenshuo/documents/downloads"), 0);

  const char* olds4 = s4.data();
  s4 = "muduo network library";
  BOOST_CHECK(s4.data() != olds4);
  BOOST_CHECK_EQUAL(s4.empty(), false);
  BOOST_CHECK_EQUAL(s4.size(), 21);
  BOOST_CHECK_EQUAL(s4.capacity(), 30);
  BOOST_CHECK_EQUAL(s4.end() - s4.begin(), 21);
  BOOST_CHECK_EQUAL(strcmp(s4.c_str(), "muduo network library"), 0);

  s3 = s4;
  BOOST_CHECK_EQUAL(s3.empty(), false);
  BOOST_CHECK_EQUAL(s3.size(), 21);
  BOOST_CHECK_EQUAL(s3.capacity(), 47);
  BOOST_CHECK_EQUAL(s3.end() - s3.begin(), 21);
  BOOST_CHECK_EQUAL(strcmp(s3.c_str(), "muduo network library"), 0);

  s3 = "https://github.com/chenshuo/documents/downloads/";
  s4 = s3;
  BOOST_CHECK_EQUAL(s4.empty(), false);
  BOOST_CHECK_EQUAL(s4.size(), 48);
  BOOST_CHECK_EQUAL(s4.capacity(), 60);
  BOOST_CHECK_EQUAL(s4.end() - s4.begin(), 48);
  BOOST_CHECK_EQUAL(strcmp(s4.c_str(), "https://github.com/chenshuo/documents/downloads/"), 0);

#ifdef __GXX_EXPERIMENTAL_CXX0X__
  String s5(std::move(s3));
  BOOST_CHECK_EQUAL(s3.data(), (const char*)NULL);
  BOOST_CHECK_EQUAL(s5.empty(), false);
  BOOST_CHECK_EQUAL(s5.size(), 48);
  BOOST_CHECK_EQUAL(s5.capacity(), 94);
  BOOST_CHECK_EQUAL(s5.end() - s5.begin(), 48);

  s5 = std::move(s2);
  BOOST_CHECK_EQUAL(s5.empty(), false);
  BOOST_CHECK_EQUAL(s5.size(), 8);
  BOOST_CHECK_EQUAL(s5.capacity(), 15);
  BOOST_CHECK_EQUAL(s5.end() - s5.begin(), 8);
  BOOST_CHECK_EQUAL(strcmp(s5.c_str(), "chenshuo"), 0);
#endif
}

BOOST_AUTO_TEST_CASE(testPushBack)
{
  String s1;
  s1.push_back('a');
  BOOST_CHECK_EQUAL(s1.empty(), false);
  BOOST_CHECK_EQUAL(s1.size(), 1);
  BOOST_CHECK_EQUAL(s1.capacity(), 15);
  BOOST_CHECK_EQUAL(s1.end() - s1.begin(), 1);
  BOOST_CHECK_EQUAL(strcmp(s1.c_str(), "a"), 0);

  s1.push_back('b');
  BOOST_CHECK_EQUAL(s1.empty(), false);
  BOOST_CHECK_EQUAL(s1.size(), 2);
  BOOST_CHECK_EQUAL(s1.capacity(), 15);
  BOOST_CHECK_EQUAL(s1.end() - s1.begin(), 2);
  BOOST_CHECK_EQUAL(strcmp(s1.c_str(), "ab"), 0);

  String s2 = "hello";
  s2.push_back('C');
  BOOST_CHECK_EQUAL(s2.empty(), false);
  BOOST_CHECK_EQUAL(s2.size(), 6);
  BOOST_CHECK_EQUAL(s2.capacity(), 15);
  BOOST_CHECK_EQUAL(strcmp(s2.c_str(), "helloC"), 0);

  s2 = "OctoberDecember";
  BOOST_CHECK_EQUAL(s2.size(), 15);
  BOOST_CHECK_EQUAL(s2.capacity(), 15);
  s2.push_back('X');
  BOOST_CHECK_EQUAL(s2.size(), 16);
  BOOST_CHECK_EQUAL(s2.capacity(), 30);
  BOOST_CHECK_EQUAL(strcmp(s2.c_str(), "OctoberDecemberX"), 0);
}

BOOST_AUTO_TEST_CASE(testAppendAndAssign)
{
  String s1;
  s1.append("hello");
  BOOST_CHECK_EQUAL(s1.size(), 5);
  BOOST_CHECK_EQUAL(s1.capacity(), 15);
  BOOST_CHECK_EQUAL(strcmp(s1.c_str(), "hello"), 0);

  s1.append("world");
  BOOST_CHECK_EQUAL(s1.size(), 10);
  BOOST_CHECK_EQUAL(s1.capacity(), 15);
  BOOST_CHECK_EQUAL(strcmp(s1.c_str(), "helloworld"), 0);

  s1.append("solstice");
  BOOST_CHECK_EQUAL(s1.size(), 18);
  BOOST_CHECK_EQUAL(s1.capacity(), 30);
  BOOST_CHECK_EQUAL(strcmp(s1.c_str(), "helloworldsolstice"), 0);

  s1.append(s1.data(), 5);
  BOOST_CHECK_EQUAL(s1.size(), 23);
  BOOST_CHECK_EQUAL(s1.capacity(), 30);
  BOOST_CHECK_EQUAL(strcmp(s1.c_str(), "helloworldsolsticehello"), 0);

  s1.append(s1.data()+10, 8);
  BOOST_CHECK_EQUAL(s1.size(), 31);
  BOOST_CHECK_EQUAL(s1.capacity(), 60);
  BOOST_CHECK_EQUAL(strcmp(s1.c_str(), "helloworldsolsticehellosolstice"), 0);

  String s2;
  s2.assign(s1.c_str(), s1.size());
  BOOST_CHECK_EQUAL(s2.size(), 31);
  BOOST_CHECK_EQUAL(s2.capacity(), 31);
  BOOST_CHECK_EQUAL(strcmp(s2.c_str(), "helloworldsolsticehellosolstice"), 0);

  s2.assign("muduo", 4);
  BOOST_CHECK_EQUAL(s2.size(), 4);
  BOOST_CHECK_EQUAL(s2.capacity(), 31);
  BOOST_CHECK_EQUAL(strcmp(s2.c_str(), "mudu"), 0);

  s2.assign(s2.data()+1, 2);
  BOOST_CHECK_EQUAL(s2.size(), 2);
  BOOST_CHECK_EQUAL(s2.capacity(), 31);
  BOOST_CHECK_EQUAL(strcmp(s2.c_str(), "ud"), 0);
}
