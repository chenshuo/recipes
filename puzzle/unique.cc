#include <algorithm>
#include <string.h>

struct AreBothSpaces
{
  bool operator()(char x, char y) const
  {
    return x == ' ' && y == ' ';
  }
};

int removeContinuousSpaces(char* const str)
{
  size_t len = strlen(str); // or use std::string
  char* end = str+len;
  assert(*end == '\0');
  char* last = std::unique(str, end, AreBothSpaces());
  *last = '\0';
  return last - str;
}

int main()
{
  char inout[256] = "";
  strcpy(inout, "");
  removeContinuousSpaces(inout);
  assert(strcmp(inout, "") == 0);

  strcpy(inout, "a");
  removeContinuousSpaces(inout);
  assert(strcmp(inout, "a") == 0);

  strcpy(inout, " a");
  removeContinuousSpaces(inout);
  assert(strcmp(inout, " a") == 0);

  strcpy(inout, "  a");
  removeContinuousSpaces(inout);
  assert(strcmp(inout, " a") == 0);

  strcpy(inout, "a ");
  removeContinuousSpaces(inout);
  assert(strcmp(inout, "a ") == 0);

  strcpy(inout, "a  ");
  removeContinuousSpaces(inout);
  assert(strcmp(inout, "a ") == 0);

  strcpy(inout, "abc    def");
  removeContinuousSpaces(inout);
  assert(strcmp(inout, "abc def") == 0);

  strcpy(inout, "abc    def       ghi");
  removeContinuousSpaces(inout);
  assert(strcmp(inout, "abc def ghi") == 0);

  strcpy(inout, " a b   d e ");
  removeContinuousSpaces(inout);
  assert(strcmp(inout, " a b d e ") == 0);

  strcpy(inout, "           ");
  removeContinuousSpaces(inout);
  assert(strcmp(inout, " ") == 0);
}