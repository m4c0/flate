export module flate:bitstream;
import yoyo;

export namespace flate {
class bitstream {
  static constexpr const auto max_bits_at_once = 8;
  static constexpr const auto bits_per_byte = 8U;

  yoyo::reader *m_reader;

  unsigned m_rem{};
  unsigned m_buf{};

  constexpr auto prepare_next(unsigned n) {
    if (m_rem >= n)
      return mno::req<void>{};
    return m_reader->read_u8().map([this](auto next) {
      m_buf = m_buf + (next << m_rem);
      m_rem += bits_per_byte;
    });
  }
  [[nodiscard]] constexpr auto next_tiny(unsigned n) {
    // assert(n <= max_bits_at_once);
    return prepare_next(n).map([n, this] {
      auto res = m_buf & ((1U << n) - 1U);
      m_rem -= n;
      m_buf >>= n;
      return res;
    });
  }

public:
  explicit constexpr bitstream(yoyo::reader *r) : m_reader{r} {}

  [[nodiscard]] constexpr auto align() noexcept {
    return (m_rem > 0) ? next_tiny(m_rem).map([](auto) {}) : mno::req<void>{};
  }

  template <unsigned N>
    requires(N <= max_bits_at_once)
  [[nodiscard]] constexpr auto next() {
    return next_tiny(N);
  }

  template <unsigned N> [[nodiscard]] constexpr mno::req<void> skip() {
    mno::req<unsigned> res{};
    auto rem = N;
    while (rem >= max_bits_at_once && res.is_valid()) {
      res = next<max_bits_at_once>();
      rem -= max_bits_at_once;
    }
    return res.fmap([this](auto) { return next<N % max_bits_at_once>(); })
        .map([](auto) {});
  }

  [[nodiscard]] constexpr mno::req<unsigned> next(unsigned n) {
    // assert(n <= sizeof(unsigned) * bits_per_byte);

    mno::req<unsigned> res{0U};
    unsigned shift = 0;
    while (n > 0 && res.is_valid()) {
      auto bits = n > bits_per_byte ? bits_per_byte : n;
      res = mno::combine(
          [&](auto acc, auto tiny) {
            auto res = acc | (tiny << shift);
            n -= bits;
            shift += bits_per_byte;
            return res;
          },
          res, next_tiny(bits));
    }
    return res;
  }

  [[nodiscard]] constexpr auto eof() const noexcept {
    return m_reader->eof().map([this](auto eof) { return eof && m_rem == 0; });
  }
};

template <typename Reader> class ce_bitstream : public bitstream {
  Reader m_real_reader;

public:
  explicit constexpr ce_bitstream(Reader r)
      : bitstream{&m_real_reader}, m_real_reader{r} {};
};
} // namespace flate

using namespace flate;

static constexpr const yoyo::ce_reader data{0x8d, 0x52, 0x4d};

static_assert([] {
  auto r = data;
  bitstream b{&r};
  if (b.next<1>() != 1)
    return false;
  if (b.next<2>() != 0b10)
    return false; // NOLINT
  if (b.next<5>() != 17)
    return false; // NOLINT
  if (b.next<5>() != 18)
    return false; // NOLINT
  if (b.next<4>() != 10)
    return false; // NOLINT
  if (b.next<3>() != 6)
    return false; // NOLINT
  if (b.next<3>() != 4)
    return false; // NOLINT
  return true;
}());
// Skip nothing from beginning
static_assert([] {
  auto r = data;
  bitstream b{&r};
  return b.skip<0>().map([&] { return b.next<1>() == 1; }).unwrap(false);
}());
// Skip nothing from somewhere
static_assert([] {
  auto r = data;
  bitstream b{&r};
  if (b.next<1>() != 1)
    return false;
  return b.skip<2>().map([&] { return b.next<5>() == 17; }).unwrap(false);
}());
// Skip from beginning
static_assert([] {
  auto r = data;
  bitstream b{&r};
  return b.skip<3>().map([&] { return b.next<5>() == 17; }).unwrap(false);
}());
static_assert([] {
  constexpr const auto bits_to_skip = 1 + 2 + 5 + 5 + 4;
  auto r = data;
  bitstream b{&r};
  return b.skip<bits_to_skip>()
      .map([&] { return b.next<3>() == 6 && b.next<3>() == 4; })
      .unwrap(false);
}());
static_assert([] {
  auto r = data;
  bitstream b{&r};
  return b.next(1) == 1 && b.next(2) == 2 && b.next(5) == 17;
}());
static_assert([] {
  constexpr const yoyo::ce_reader data{0xA0, 0x5A, 0x05};
  auto r = data;
  bitstream b{&r};
  return b.skip<4>()
      .map([&] { return b.next(8) == 0xAA && b.next(8) == 0x55; })
      .unwrap(false);
}());
static_assert([] {
  constexpr const yoyo::ce_reader data{0x40, 0x23, 0x01};
  auto r = data;
  bitstream b{&r};
  return b.skip<4>().map([&] { return b.next(16) == 0x1234; }).unwrap(false);
}());
static_assert([] {
  auto r = data;
  bitstream b{&r};
  return b.skip<4>()
      .fmap([&] { return b.align(); })
      .fmap([&] {
        return b.align(); // Should NOT skip another byte
      })
      .map([&] { return b.next<8>() == 0x52; })
      .unwrap(false);
}());
