#include <algorithm>
#include <string>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

string readFile(const char* file)
{
  string result;
  int fd = open(file, O_RDONLY);
  if (fd >= 0) {
    char buf[256*1024];
    ssize_t nr = 0;
    while ( (nr = read(fd, buf, sizeof buf)) > 0) {
      result.append(buf, nr);
    }
    close(fd);
  } else {
    perror(file);
    abort();
  }
  return result;
}

enum Encoding
{
  kUnknown, kUnicode, kGBK, kUTF8,
};

const char* getEncodingName(Encoding enc)
{
  if (enc == kUTF8) {
    return "UTF-8";
  } else if (enc == kUnicode) {
    return "Unicode";
  } else {
    return "Unknown";
  }
}

Encoding detectEncoding(const string& content)
{
  if (content.size() < 2)
    return kGBK;

  if (content[0] == '\xFF' && content[1] == '\xFE') {
    return kUnicode;
  } else {
    for (size_t i = 0; i < content.size()-2;) {
      if (content[i] & 0x80) {
        if ((content[i+1] & 0x80) == 0) {
          return kGBK;
        } else if ((content[i] & 0xE0) == 0xC0) {
          if ((content[i+1] & 0xC0) == 0x80) {
            return kGBK;
          } else {
            i += 2;
          }
        } else if ((content[i] & 0xF0) == 0xE0) {
          if ((content[i+1] & 0xC0) == 0x80) {
            if ((content[i+2] & 0xC0) == 0x80) {
              //printf("UTF-8 !!!\n");
              return kUTF8;
            } else {
              return kGBK;
            }
          } else {
            return kGBK;
          }
        } else {
          return kGBK;
        }
      } else {
        ++i;
      }
    }
    return kUTF8;
  }
}

int countChineseCharsGbk(const string& content)
{
  int cnt = 0;
  for (size_t i = 0; i < content.size(); ++i) {
    if (content[i] & 0x80) {
      ++cnt;
      ++i;
    }
  }
  return cnt;
}

int countChineseCharsUcs2(const string& content)
{
  assert(content.size() % 2 == 0);
  int cnt = 0;
  const uint16_t* p = reinterpret_cast<const uint16_t*>(content.c_str());
  ++p;
  for (size_t i = 2; i < content.size()/2; ++i) {
    if (*p++ > 127)
      ++cnt;
  }
  return cnt;
}

int countChineseCharsUtf8(const string& content)
{
  int cnt = 0;
  for (size_t i = 0; i < content.size();) {
    char c = content[i];
    if ((c & 0x80) == 0) {
      ++i;
    } else if ((c & 0xe0) == 0xc0) {
      i += 2;
    } else if ((c & 0xf0) == 0xe0) {
      i += 3;
      ++cnt;
    } else {
      abort();
    }
  }
  return cnt;
}

int main(int argc, char* argv[])
{
  int totalLines = 0;
  int totalChinese = 0;
  size_t totalBytes = 0;
  for (int i = 1; i < argc; ++i) {
    const char* file = argv[i];
    string content = readFile(file);
    Encoding enc = detectEncoding(content);
    int chinese = 0;
    if (enc == kGBK) {
      chinese = countChineseCharsGbk(content);
    } else if (enc == kUnicode) {
      chinese = countChineseCharsUcs2(content);
    } else {
      chinese = countChineseCharsUtf8(content);
    }
    int lines = std::count(content.begin(), content.end(), '\n');
    printf("%6d %6d %7zd %s (%s)\n",
        lines, chinese, content.size(), file, getEncodingName(enc));
    totalLines += lines;
    totalChinese += chinese;
    totalBytes += content.size();
  }
  printf("%6d %6d %7zd total\n", totalLines, totalChinese, totalBytes);
}

