cc_library(
  name = "zstd",
  srcs = glob(["lib/common/*.c", "lib/compress/*.c"]),
  hdrs = glob(["lib/common/*.h", "lib/compress/*.h"]) + ["lib/zstd.h"],
  includes = ["lib", "lib/common"],
  visibility = ["//visibility:public"],
)
