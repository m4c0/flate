export module flate:namffuh;
import :bitwriter;
import :symbols;
import hai;

namespace flate {
constexpr void static_huffman_encode(const hai::array<symbol> &syms,
                                     bitwriter &w) {
  //
}
} // namespace flate

static_assert([] {
  using namespace flate;

  const auto assert = [](uint8_t res, uint8_t exp) {
    if (res != exp)
      throw 0;
  };

  hai::array<symbol>
      syms{6};
  syms[0] = {type::raw, 0, 0, 'y'}; // 121 0x7
  syms[1] = {type::raw, 0, 0, 'o'}; // 111 0x6F
  syms[2] = {type::repeat, 6, 2};
  syms[3] = {type::end};

  bitwriter w{16};
  static_huffman_encode(syms, w);

  assert(w.buffer()[0], 0);
  return true;
}());
