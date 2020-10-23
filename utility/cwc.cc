#include <algorithm>
#include <string>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

using namespace std;

string readFile(const char* file)
{
  string result;
  // 128 KiB is from:
  // http://git.savannah.gnu.org/cgit/coreutils.git/tree/src/ioblksize.h
  // > As of May 2014, 128KiB is determined to be the minimium
  // > blksize to best minimize system call overhead.
  char buf[128*1024];
  if (strcmp(file, "-") == 0) {
    size_t nr = 0;
    while ( (nr = fread(buf, 1, sizeof buf, stdin)) > 0) {
      // printf("%zd\n", nr);
      result.append(buf, nr);
      if (feof(stdin))
        break;
    }
    return result;
  }
  int fd = open(file, O_RDONLY);
  if (fd >= 0) {
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
  switch (enc) {
    case kUnicode:
      return "Unicode";
    case kUTF8:
      return "UTF-8";
    case kGBK:
      return "GBK";
    default:
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
          // FIXME: 4-byte UTF-8, x & 0xF8 == 0xF0
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
    } else if ((c & 0xf8) == 0xf0) {
      i += 4;
      ++cnt;
    } else {
      abort();
    }
  }
  return cnt;
}

int countChinese(const string& content, Encoding* enc)
{
  *enc = detectEncoding(content);
  int chinese = 0;
  if (*enc == kGBK) {
    chinese = countChineseCharsGbk(content);
  } else if (*enc == kUnicode) {
    chinese = countChineseCharsUcs2(content);
  } else {
    chinese = countChineseCharsUtf8(content);
  }

  return chinese;
}

int main(int argc, char* argv[])
{
  int totalLines = 0;
  int totalChinese = 0;
  size_t totalBytes = 0;
  if (argc <= 1) {
    const char* file = "-";
    string content = readFile(file);
    Encoding enc = kUnknown;
    int chinese = countChinese(content, &enc);
    int lines = std::count(content.begin(), content.end(), '\n');
    printf("%6d %6d %7zd %s (%s)\n",
        lines, chinese, content.size(), file, getEncodingName(enc));
    return 0;
  }
  for (int i = 1; i < argc; ++i) {
    const char* file = argv[i];
    string content = readFile(file);
    Encoding enc = kUnknown;
    int chinese = countChinese(content, &enc);
    int lines = std::count(content.begin(), content.end(), '\n');
    printf("%6d %6d %7zd %s (%s)\n",
        lines, chinese, content.size(), file, getEncodingName(enc));
    totalLines += lines;
    totalChinese += chinese;
    totalBytes += content.size();
  }
  printf("%6d %6d %7zd total\n", totalLines, totalChinese, totalBytes);
}

