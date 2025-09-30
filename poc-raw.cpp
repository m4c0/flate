#pragma leco tool
#include "../leco/targets.hpp"
#include <stdlib.h>

#ifdef _WIN32
#define SEP "\\"
#else
#define SEP "/"
#endif

import flate;
import print;
import yoyo;

constexpr const unsigned char lorem_ipsum[] = "HEYHEYHEY YOYO YO";

static auto create_file() {
  auto w = yoyo::file_writer::open("out/test.def")
    .take([](auto msg) { die("failed to open file: ", msg); });

  flate::compress(w, lorem_ipsum, sizeof(lorem_ipsum));
}
int main() {
  create_file();
  return system("out" SEP HOST_TARGET SEP "infgen.exe -d -d out" SEP "test.def");
}
