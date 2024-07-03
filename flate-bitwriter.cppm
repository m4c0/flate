export module flate:bitwriter;
import hai;
import traits;

using namespace traits::ints;

namespace flate {
class bitwriter {
  hai::varray<uint8_t> m_buffer;
  uint32_t m_acc{};
  uint32_t m_bits{};

public:
  constexpr bitwriter(unsigned buf_size) : m_buffer{buf_size} {}

  [[nodiscard]] constexpr const auto &buffer() const { return m_buffer; }

  constexpr void write(unsigned data, unsigned bit_count) {}
};
} // namespace flate

static_assert([] {
  constexpr const auto assert = [](auto res, auto exp) {
    if (res != exp)
      throw 0;
  };

  flate::bitwriter b{2};
  b.write(1, 1);
  b.write(2, 2);
  b.write(0x5A, 8);
  b.write(3, 2);
  // 0001.1010 1101.0101
  assert(b.buffer()[0], 0xD5u);
  assert(b.buffer()[1], 0x1Au);
  return true;
}());
