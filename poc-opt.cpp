#pragma leco tool

import flate;
import hai;
import jojo;
import jute;
import print;
import traits;

using namespace traits::ints;

static auto compress(const hai::array<char> &data) {
  putfn("compressing %d bytes", data.size());
  return flate::compress(data.begin(), data.size());
}

static auto decompress(const hai::varray<uint8_t> &data) {
  putfn("decompressing %d bytes", data.size());

  hai::array<uint8_t> dec { 1024 * 1024 };
  auto n = flate::decompresser { data.begin(), data.size() }.read(dec.begin(), dec.size());
  putfn("got %d bytes back", n);
}

int main(int argc, char **argv) {
  auto file = argc == 2 ? argv[1] : "infgen.c";
  decompress(compress(jojo::read(jute::view::unsafe(file))));
}
