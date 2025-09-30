#pragma leco tool
#include "../leco/targets.hpp"
#include <stdlib.h>

#ifdef _WIN32
#define SEP "\\"
#else
#define SEP "/"
#endif

import flate;
import jojo;
import print;

constexpr const unsigned char lorem_ipsum[] = "HEYHEYHEY YOYO YO";

static auto create_file() {
  jojo::write("out/test.def", flate::compress(lorem_ipsum, sizeof(lorem_ipsum)));
}
int main() {
  create_file();
  return system("out" SEP HOST_TARGET SEP "infgen.exe -d -d out" SEP "test.def");
}
