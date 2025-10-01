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
  flate::bitstream bs { data.begin(), data.size() };
  flate::huffman_reader::create(&bs)
      .fmap([&](auto &hr) {
        hai::array<uint8_t> dec{1024 * 1024};
        return hr.read_up_to(dec.begin(), dec.size());
      })
      .map([](auto n) { putfn("got %d bytes back", n); })
      .log_error();
}

int main(int argc, char **argv) {
  auto file = argc == 2 ? argv[1] : "infgen.c";
  decompress(compress(jojo::read(jute::view::unsafe(file))));
}
