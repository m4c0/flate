export module flate:symbols;
import :bitstream;
import :huffman;
import :tables;
import hai;
import traits;

using namespace traits::ints;

namespace flate::symbols {
  export struct code_gt_max_lens {};
  export struct dist_code_gt_max {};

enum class type { nil, end, raw, repeat };
struct symbol {
  type type;
  unsigned len{};
  unsigned dist{};
  uint8_t c{};
};

static constexpr symbol read_repeat(const tables::huff_tables & huff, bitstream * bits, unsigned code) {
  if (code > tables::max_lens_code) throw code_gt_max_lens{};;

  const auto len_bits = tables::bitlens.data[code];
  const auto len = len_bits.second + bits->next(len_bits.bits);
  const auto dist_code = decode_huffman(huff.hdist, bits);
  if (dist_code > tables::max_dists_code) throw dist_code_gt_max {};

  const auto dist_bits = tables::bitdists.data[dist_code];
  const auto dist = dist_bits.second + bits->next(dist_bits.bits);
  return symbol { type::repeat, len, dist };
}

[[nodiscard]] inline constexpr symbol read_next_symbol(const tables::huff_tables & huff, bitstream * bits) {
  const auto code = decode_huffman(huff.hlist, bits);
  constexpr const auto end_code = 256;

  if (code < end_code)  return symbol { type::raw, 0U, 0U, static_cast<uint8_t>(code) };
  if (code == end_code) return symbol { type::end };
  return read_repeat(huff, bits, code);
}
} // namespace flate::symbols

using namespace flate::symbols;

static constexpr auto build_sparse_huff() {
  // 2-bit alignment to match a half-hex and simplify the assertion
  return flate::tables::huff_tables{
      .hlist =
          flate::huffman_codes{
              hai::array<unsigned>::make(0, 0, 4),
              hai::array<unsigned>::make('?', 256, 270, 285),
          },
      .hdist =
          flate::huffman_codes{
              hai::array<unsigned>::make(0, 0, 4),
              hai::array<unsigned>::make(2, 6, 0, 0),
          },
  };
}
static constexpr auto test_read_next_symbol(uint8_t data, symbol expected) {
  auto bits = flate::bitstream { &data, 1 };
  auto sym = read_next_symbol(build_sparse_huff(), &bits);
  if (sym.type != expected.type) throw 0;
  if (sym.len != expected.len) throw 0;
  if (sym.dist != expected.dist) throw 0;
  if (sym.c != expected.c) throw 0;
  return true;
}
static_assert(test_read_next_symbol(0b00, symbol{type::raw, 0, 0, '?'}));
static_assert(test_read_next_symbol(0b10, symbol{type::end}));
static_assert(test_read_next_symbol(0b11100101, symbol{type::repeat, 24, 12}));

static constexpr auto test_fixed_table(uint8_t first_byte, uint8_t second_byte,
                                       symbol expected) {
  const unsigned char buf[] { first_byte, second_byte, 0 };
  auto bits = flate::bitstream { buf, 3 };
  auto sym = read_next_symbol(flate::tables::create_fixed_huffman_table(), &bits);
  if (sym.type != expected.type) throw 0;
  if (sym.len != expected.len) throw 0;
  if (sym.dist != expected.dist) throw 0;
  if (sym.c != expected.c) throw 0;
  return true;
}
static_assert(test_fixed_table(0b01001100, 0, symbol{type::raw, 0, 0, 2}));
static_assert(test_fixed_table(0b10010011, 0, symbol{type::raw, 0, 0, 146}));
static_assert(test_fixed_table(0b0000000, 0, symbol{type::end})); // 256
static_assert(test_fixed_table(0b10100000, 0,
                               symbol{type::repeat, 4, 257})); // 258, dist=16,0
static_assert(test_fixed_table(0b01000011, 0,
                               symbol{type::repeat, 163, 1})); // 282
static_assert(test_fixed_table(0b01100000, 0b1000, symbol{type::repeat, 5, 2}));
