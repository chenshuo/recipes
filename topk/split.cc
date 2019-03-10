#include <fstream>
#include <memory>

std::string getOutputName(int n)
{
  char buf[256];
  snprintf(buf, sizeof buf, "input-%03d", n);
  printf("%s\n", buf);
  return buf;
}

int main(int argc, char* argv[])
{
  std::ifstream in(argv[1]);
  std::string line;
  int count = 0;
  int size = 0;
  int64_t total = 0;
  std::unique_ptr<std::ofstream> out(new std::ofstream(getOutputName(count)));
  while (getline(in, line))
  {
    line.append("\n");
    size += line.size();
    total += size;
    *out << line;
    if (size >= 1000'000'000)
    {
      ++count;
      out.reset(new std::ofstream(getOutputName(count)));
      size = 0;
    }
  }
  // out.reset();
}


