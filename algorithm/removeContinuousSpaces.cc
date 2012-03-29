#include <assert.h>
#include <algorithm>
#include <string>

struct AreBothSpaces
{
  bool operator()(char x, char y) const
  {
    return x == ' ' && y == ' ';
  }
};

void removeContinuousSpaces(std::string& str)
{
  std::string::iterator last
    = std::unique(str.begin(), str.end(), AreBothSpaces());
  str.erase(last, str.end());
}

int main()
{
  std::string inout;

  inout = "";
  removeContinuousSpaces(inout);
  assert(inout == "");

  inout = "a";
  removeContinuousSpaces(inout);
  assert(inout == "a");

  inout = " a";
  removeContinuousSpaces(inout);
  assert(inout == " a");

  inout = "  a";
  removeContinuousSpaces(inout);
  assert(inout == " a");

  inout = "a ";
  removeContinuousSpaces(inout);
  assert(inout == "a ");

  inout = "a  ";
  removeContinuousSpaces(inout);
  assert(inout == "a ");

  inout = "aaa    bbb";
  removeContinuousSpaces(inout);
  assert(inout == "aaa bbb");

  inout = "aaa    bbb       ccc";
  removeContinuousSpaces(inout);
  assert(inout == "aaa bbb ccc");

  inout = " a b   c d ";
  removeContinuousSpaces(inout);
  assert(inout == " a b c d ");

  inout = "        ";
  removeContinuousSpaces(inout);
  assert(inout == " ");
}
