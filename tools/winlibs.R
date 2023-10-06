if(!file.exists("../windows/harfbuzz/include/harfbuzz/hb.h")){
  unlink("../windows", recursive = TRUE)
  url <- if(grepl("aarch", R.version$platform)){
    "https://github.com/r-windows/bundles/releases/download/harfbuzz-8.2.1/harfbuzz-8.2.1-clang-aarch64.tar.xz"
  } else if(grepl("clang", Sys.getenv('R_COMPILED_BY'))){
    "https://github.com/r-windows/bundles/releases/download/harfbuzz-8.2.1/harfbuzz-8.2.1-clang-x86_64.tar.xz"
  }  else if(getRversion() >= "4.2") {
    "https://github.com/r-windows/bundles/releases/download/harfbuzz-8.2.1/harfbuzz-8.2.1-ucrt-x86_64.tar.xz"
  } else {
    "https://github.com/rwinlib/harfbuzz/archive/v2.7.4.tar.gz"
  }
  download.file(url, basename(url), quiet = TRUE)
  dir.create("../windows", showWarnings = FALSE)
  untar(basename(url), exdir = "../windows", tar = 'internal')
  unlink(basename(url))
  setwd("../windows")
  file.rename(list.files(), 'harfbuzz')
}
