export module flate:bitwriter;
import hai;
import print;
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
  [[nodiscard]] constexpr auto take() { return traits::move(m_buffer); }

  constexpr void write(unsigned data, unsigned bit_count) {
    [[unlikely]] if (bit_count > 16) die("attempt of writing more than 16 bits at once");

    data &= (1 << bit_count) - 1;
    m_acc |= data << m_bits;
    m_bits += bit_count;
    [[unlikely]] while (m_bits >= 8) {
      m_buffer.push_back(m_acc);
      m_bits -= 8;
      m_acc >>= 8;
    }
  }
  constexpr void write_be(unsigned data, unsigned bit_count) {
    unsigned flip{};
    for (auto i = 0; i < bit_count; i++) {
      flip <<= 1;
      flip |= data & 1;
      data >>= 1;
    }
    write(flip, bit_count);
  }

  constexpr void flush() {
    if (m_bits == 0)
      return;

    m_buffer.push_back(m_acc);
    m_bits = 0;
    m_acc = 0;
  }
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
  b.flush();
  // 0001.1010 1101.0101
  assert(b.buffer()[0], 0xD5u);
  assert(b.buffer()[1], 0x1Au);
  return true;
}());
static_assert([] {
  constexpr const auto assert = [](auto res, auto exp) {
    if (res != exp)
      throw 0;
  };

  flate::bitwriter b{2};
  b.write_be(1, 1);
  b.write_be(2, 2);
  b.write_be(0x55, 8);
  b.write_be(3, 2);
  b.flush();
  assert(b.buffer()[0], 0b01010011u);
  assert(b.buffer()[1], 0b00011101u);
  return true;
}());
