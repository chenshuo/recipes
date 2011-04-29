#include <algorithm>
#include <string>
#include <stdio.h>

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
  } else {
    return "Unknown";
  }
}

Encoding detectEncoding(const string& content)
{
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

int countChineseCharsUtf8(const string& content)
{
  int cnt = 0;
  for (size_t i = 0; i < content.size();) {
    if (content[i] & 0x80) {
      i += 3;
      ++cnt;
    } else {
      ++i;
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
    } else if (enc == kUnicode) {
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

