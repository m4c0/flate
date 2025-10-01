export module flate:bitstream;

namespace flate {
  export struct buffer_underrun {};

export class bitstream {
  static constexpr const auto max_bits_at_once = 8;
  static constexpr const auto bits_per_byte = 8U;

  const unsigned char * m_comp_ptr;
  unsigned m_comp_rem;

  unsigned m_rem{};
  unsigned m_buf{};

  constexpr void prepare_next(unsigned n) {
    if (m_rem >= n) return;

    if (m_comp_rem == 0) throw buffer_underrun {};
    auto next = *m_comp_ptr++;
    m_comp_rem--;

    m_buf = m_buf + (next << m_rem);
    m_rem += bits_per_byte;
  }
  [[nodiscard]] constexpr auto next_tiny(unsigned n) {
    // assert(n <= max_bits_at_once);
    prepare_next(n);

    auto res = m_buf & ((1U << n) - 1U);
    m_rem -= n;
    m_buf >>= n;
    return res;
  }

public:
  explicit constexpr bitstream(const void * ptr, unsigned sz) :
    m_comp_ptr { static_cast<const unsigned char *>(ptr) }
  , m_comp_rem { sz }
  {}

  [[nodiscard]] constexpr auto align() noexcept {
    if (m_rem > 0) auto _ = next_tiny(m_rem);
  }

  template <unsigned N> requires(N <= max_bits_at_once)
  [[nodiscard]] constexpr auto next() { return next_tiny(N); }

  template <unsigned N> constexpr void skip() {
    auto rem = N;
    while (rem >= max_bits_at_once) {
      auto _ = next<max_bits_at_once>();
      rem -= max_bits_at_once;
    }
    auto _ = next<N % max_bits_at_once>();
  }

  [[nodiscard]] constexpr unsigned next(unsigned n) {
    // assert(n <= sizeof(unsigned) * bits_per_byte);

    auto res = 0U;
    unsigned shift = 0;
    while (n > 0) {
      auto bits = n > bits_per_byte ? bits_per_byte : n;
      res |= next_tiny(bits) << shift;
      n -= bits;
      shift += bits_per_byte;
    }
    return res;
  }

  [[nodiscard]] constexpr auto eof() const noexcept {
    return m_comp_rem == 0 && m_rem == 0;
  }
};
} // namespace flate

using namespace flate;

static inline constexpr const unsigned char data[] { 0x8d, 0x52, 0x4d };

static_assert([] {
  bitstream b { data, 3 };
  if (b.next<1>() != 1)    return false;
  if (b.next<2>() != 0b10) return false; // NOLINT
  if (b.next<5>() != 17)   return false; // NOLINT
  if (b.next<5>() != 18)   return false; // NOLINT
  if (b.next<4>() != 10)   return false; // NOLINT
  if (b.next<3>() != 6)    return false; // NOLINT
  if (b.next<3>() != 4)    return false; // NOLINT
  return true;
}());
// Skip nothing from beginning
static_assert([] {
  bitstream b { data, 3 };
  b.skip<0>();
  return b.next<1>() == 1;
}());
// Skip nothing from somewhere
static_assert([] {
  bitstream b { data, 3 };
  if (b.next<1>() != 1) return false;
  b.skip<2>();
  return b.next<5>() == 17;
}());
// Skip from beginning
static_assert([] {
  bitstream b { data, 3 };
  b.skip<3>();
  return b.next<5>() == 17;
}());
static_assert([] {
  constexpr const auto bits_to_skip = 1 + 2 + 5 + 5 + 4;
  bitstream b { data, 3 };
  b.skip<bits_to_skip>();
  return b.next<3>() == 6 && b.next<3>() == 4;
}());
static_assert([] {
  bitstream b { data, 3 };
  return b.next(1) == 1 && b.next(2) == 2 && b.next(5) == 17;
}());
static_assert([] {
  constexpr const unsigned char data[] { 0xA0, 0x5A, 0x05 };
  bitstream b { data, 3 };
  b.skip<4>();
  return b.next(8) == 0xAA && b.next(8) == 0x55;
}());
static_assert([] {
  constexpr const unsigned char data[] { 0x40, 0x23, 0x01 };
  bitstream b { data, 3 };
  b.skip<4>();
  return b.next(16) == 0x1234;
}());
static_assert([] {
  bitstream b { data, 3 };
  b.skip<4>();
  b.align();
  b.align(); // Should NOT skip another byte
  return b.next<8>() == 0x52;
}());
