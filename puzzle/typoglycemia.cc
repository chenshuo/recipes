// answer to http://blog.zhaojie.me/2012/11/how-to-generate-typoglycemia-text.html
#include <algorithm>
#include <iostream>
#include <string>
#include <assert.h>
#include <ctype.h>

using std::string;

void randomizeWord(char* str, int len)
{
  const char first = *str;
  const char last = str[len-1];
  assert(isalpha(first));
  assert(isalpha(last));

  std::random_shuffle(str+1, str+len-1);

  assert(first == *str);
  assert(last == str[len-1]);
}

void randomize(string& text)
{
  string::iterator start = text.begin();
  while (start < text.end())
  {
    start = std::find_if(start, text.end(), isalpha);
    if (start != text.end())
    {
      string::iterator end = std::find_if(start,
                                          text.end(),
                                          std::not1(std::ptr_fun(isalpha)));
      if (end - start > 3)
      {
        randomizeWord(&*start, end - start);
      }
      start = end;
    }
  }
}

int main()
{
  string text;
  while (getline(std::cin, text))
  {
    randomize(text);
    std::cout << text << std::endl;
  }
}
