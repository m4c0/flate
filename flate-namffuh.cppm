export module flate:namffuh;
import :bitwriter;
import :symbols;
import :tables;
import hai;
import silog;

namespace flate {
static constexpr auto static_huffman_code(unsigned lit) {
  if (lit <= 143)
    return 0b00110000 + lit;
  if (lit <= 255)
    return 0b110010000 + (lit - 144);
  if (lit <= 279)
    return lit - 256;
  return 0b11000000 + (lit - 280);
}
static constexpr auto static_huffman_bits(unsigned lit) {
  if (lit <= 143)
    return 8;
  if (lit <= 255)
    return 9;
  if (lit <= 279)
    return 7;
  return 8;
}
static constexpr void write_static_huffman(unsigned lit, bitwriter &w) {
  w.write(static_huffman_code(lit), static_huffman_bits(lit));
}

static constexpr void write_huffman_len(unsigned len, bitwriter &w) {
  for (auto code = tables::min_lens_code; code <= tables::max_lens_code;
       code++) {
    auto [bits, min] = tables::bitlens.data[code];
    if (len >= min + (1 << bits))
      continue;

    write_static_huffman(code, w);
    if (bits > 0)
      w.write(len - min, bits);
    return;
  }
  silog::fail("invalid huffman length");
}

static constexpr void write_huffman_dist(unsigned dist, bitwriter &w) {
  for (auto code = 0; code <= tables::max_dists_code; code++) {
    auto [bits, min] = tables::bitdists.data[code];
    if (dist >= min + (1 << bits))
      continue;

    w.write(code, 5);
    if (bits > 0)
      w.write(dist - min, bits);
    return;
  }
  silog::fail("invalid huffman dist");
}

constexpr void static_huffman_encode(const hai::array<symbol> &syms,
                                     bitwriter &w) {
  for (auto sym : syms) {
    switch (sym.type) {
    case type::raw:
      write_static_huffman(sym.c, w);
      break;
    case type::repeat:
      write_huffman_len(sym.len, w);
      write_huffman_dist(sym.dist, w);
      break;
    case type::end:
      write_static_huffman(256, w);
      return;
    case type::nil:
      silog::fail("unexpected symbol found on compression");
      break;
    }
  }
}
} // namespace flate

static_assert([] {
  using namespace flate;

  const auto assert = [](uint8_t res, uint8_t exp) {
    if (res != exp)
      throw 0;
  };

  hai::array<symbol> syms{4};
  syms[0] = {type::raw, 0, 0, 0x5A};
  syms[1] = {type::raw, 0, 0, 0xA5};
  syms[2] = {type::repeat, 6, 2}; // 260, 2
  syms[3] = {type::end};

  bitwriter w{16};
  static_huffman_encode(syms, w);
  w.flush();

  assert(w.buffer()[0], 0b10001010);
  assert(w.buffer()[1], 0b10100101);
  assert(w.buffer()[2], 0b00001001);
  assert(w.buffer()[3], 0b00000001);
  assert(w.buffer()[4], 0b0000000);

  // TODO: test len with extra bits
  // TODO: test dist with extra bits
  return true;
}());
