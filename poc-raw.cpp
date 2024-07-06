#pragma leco tool
#include "../leco/targets.hpp"
#include <stdlib.h>

#ifdef _WIN32
#define SEP "\\"
#else
#define SEP "/"
#endif

import flate;
import traits;
import yoyo;

constexpr const traits::ints::uint8_t lorem_ipsum[] = "HEYHEYHEY YOYO YO";

static auto create_file() {
  return yoyo::file_writer::open("out/test.def")
      .fpeek(flate::compress(lorem_ipsum, sizeof(lorem_ipsum)))
      .map([](auto &) {});
}
int main() {
  return create_file()
      .map([] {
        return system("out" SEP HOST_TARGET SEP "infgen.exe out" SEP
                      "test.def");
      })
      .log_error([] { return 1; });
}
