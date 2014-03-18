#include "StringTrivial.h"
#include <vector>

using namespace trivial2;

const char String::kEmpty[] = "";

void foo(String x)
{
}

void bar(const String& x)
{
}

String baz()
{
  String ret("world");
  return ret;
}

int main()
{
  String s0;
  String s1("hello");
  String s2(s0);
  String s3(s1);
  s2 = s1;
  s3 = s3;
  s1 = "aewsome";

  foo(s1);
  bar(s1);
  foo("temporary");
  bar("temporary");
  String s4 = baz();
  s4 = baz();

  std::vector<String> svec;
  svec.push_back(s0);
  svec.push_back(s1);
  svec.push_back("good job");
}

