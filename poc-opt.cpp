#pragma leco tool

import flate;
import hai;
import yoyo;
import silog;
import traits;

using namespace traits::ints;

static auto compress(const hai::array<char> &data) {
  silog::log(silog::info, "compressing %d bytes", data.size());
  hai::varray<uint8_t> out{1024 * 1024};
  yoyo::memwriter buf{out};
  flate::compress(buf, data.begin(), data.size());
  out.expand(buf.raw_pos());
  return traits::move(out);
}

static auto decompress(const hai::varray<uint8_t> &data) {
  silog::log(silog::info, "decompressing %d bytes", data.size());
  yoyo::memreader rd{data.begin(), data.size()};
  flate::bitstream bs{&rd};
  return flate::huffman_reader::create(&bs)
      .fmap([&](auto &hr) {
        hai::array<uint8_t> dec{1024 * 1024};
        return hr.read_up_to(dec.begin(), dec.size());
      })
      .map([](auto n) { silog::log(silog::info, "got %d bytes back", n); });
}

int main(int argc, char **argv) {
  auto file = argc == 2 ? argv[1] : "infgen.c";
  yoyo::file_reader::open(file)
      .fmap(yoyo::slurp)
      .map(compress)
      .fmap(decompress)
      .log_error();
}
