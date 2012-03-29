#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>

typedef int Record;
typedef std::vector<Record> File;

struct Input
{
  Record value;
  size_t index;
  const File* file;

  explicit Input(const File* f)
    : value(-1),
      index(0),
      file(f)
  { }

  bool next()
  {
    if (index < file->size())
    { value = (*file)[index];
      ++index;
      return true;
    } else {
      return false;
    }
  }

  bool operator<(const Input& rhs) const
  {
    // make_heap to build min-heap, for merging
    return value > rhs.value;
  }
};

File mergeN(const std::vector<File>& files)
{
  File output;
  std::vector<Input> inputs;

  for (size_t i = 0; i < files.size(); ++i) {
    Input input(&files[i]);
    if (input.next()) {
      inputs.push_back(input);
    }
  }

  std::make_heap(inputs.begin(), inputs.end());
  while (!inputs.empty()) {
    std::pop_heap(inputs.begin(), inputs.end());
    output.push_back(inputs.back().value);

    if (inputs.back().next()) {
      std::push_heap(inputs.begin(), inputs.end());
    } else {
      inputs.pop_back();
    }
  }

  return output;
}

int main()
{
  const int kFiles = 32;
  std::vector<File> files(kFiles);
  for (int i = 0; i < kFiles; ++i) {
    File file(rand() % 1000);
    std::generate(file.begin(), file.end(), &rand);
    std::sort(file.begin(), file.end());
    files[i].swap(file);
  }

  File output = mergeN(files);

  std::copy(output.begin(), output.end(),
            std::ostream_iterator<Record>(std::cout, "\n"));
}
