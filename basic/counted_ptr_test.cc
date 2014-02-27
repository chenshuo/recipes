#include "counted_ptr.h"

#include <vector>

#include <stdio.h>

counted_ptr<int> foo()
{
  counted_ptr<int> sp(new int(123));
  return sp;
}

void bar(counted_ptr<int> x)
{
  printf("%d\n", x.use_count());
}

class Zoo;
struct Age
{
  int value;
};

int main()
{
  counted_ptr<int> sp1;
  counted_ptr<int> sp2(new int);
  sp1 = sp2;
  counted_ptr<int> sp3(sp2);
  {
  counted_ptr<int> sp4(sp2);
  counted_ptr<int> sp5(sp1);
  printf("%d\n", sp5.use_count());
  bar(std::move(sp5));
  }
  counted_ptr<int> sp6(foo());

  printf("%d\n", *sp6);
  counted_ptr<Age> sp7(new Age);
  sp7->value = 43;
  printf("%d\n", sp7->value);

  std::vector<counted_ptr<int>> vec;
  vec.push_back(sp1);
  vec.push_back(sp2);
  vec.push_back(sp3);
  vec.push_back(sp6);

  if (sp2)
    printf("%p\n", sp2.get());
  // counted_ptr<Zoo> x;
}
