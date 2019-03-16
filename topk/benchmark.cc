#include "file.h"
#include "input.h"
#include "timer.h"

#include "absl/container/flat_hash_set.h"

int main(int argc, char* argv[])
{
  setlocale(LC_NUMERIC, "");

  bool combine = false;
  bool sequential = false;
  int buffer_size = kBufferSize;
  int opt;
  while ((opt = getopt(argc, argv, "b:cs")) != -1)
  {
    switch (opt)
    {
      case 'b':
        buffer_size = atoi(optarg);
        break;
      case 'c':
        combine = true;
        break;
      case 's':
        sequential = true;
        break;
    }
  }

  LOG_INFO << "Reading " << argc - optind << (combine ? " segment " : "") << " files "
      << (sequential ? "sequentially" : "randomly")
      << ", buffer size " << buffer_size;
  Timer timer;
  int64_t total = 0;
  int64_t lines = 0;
  int64_t count = 0;

  if (combine)
  {
  std::vector<std::unique_ptr<SegmentInput>> inputs;
  inputs.reserve(argc - optind);
  for (int i = optind; i < argc; ++i)
  {
    inputs.emplace_back(new SegmentInput(argv[i], buffer_size));
  }

  if (sequential)
  {
    for (const auto& input : inputs)
    {
      Timer t;
      //std::string line;
      while (input->next())
      {
        count += input->current_count();
        ++lines;
      }
      int64_t len = input->tell();
      LOG_INFO << "Done " << input->filename() << " " << t.report(len);
      total += len;
    }
  }
  else
  {
  }
  }
  else
  {
  std::vector<std::unique_ptr<InputFile>> files;
  files.reserve(argc - optind);
  for (int i = optind; i < argc; ++i)
  {
    files.emplace_back(new InputFile(argv[i], buffer_size));
  }

  if (sequential)
  {
    for (const auto& file : files)
    {
      Timer t;
      std::string line;
      while (file->getline(&line))
      {
        ++lines;
      }
      int64_t len = file->tell();
      LOG_DEBUG << "Done " << file->filename() << " " << t.report(len);
      total += len;
    }
  }
  else
  {
    std::string line;
    absl::flat_hash_set<InputFile*> toRemove;
    while (!files.empty())
    {
      toRemove.clear();
      // read one line from each file
      for (const auto& file : files)
      {
        if (file->getline(&line))
        {
          ++lines;
        }
        else
        {
          toRemove.insert(file.get());
        }
      }
      if (!toRemove.empty())
      {
        for (auto* f : toRemove)
        {
          total += f->tell();
          LOG_DEBUG << "Done " << f->filename();
        }
        // std::partition?
        auto it = std::remove_if(files.begin(), files.end(),
                                 [&toRemove] (const auto& f) { return toRemove.count(f.get()) > 0; });
        assert(files.end() - it == toRemove.size());
        files.erase(it, files.end());
      }
    }
  }
  }

  LOG_INFO << "All done " << timer.report(total) << " "
      << muduo::Fmt("%'ld", lines) << " lines "
      << muduo::Fmt("%'ld", count) << " count";
}
