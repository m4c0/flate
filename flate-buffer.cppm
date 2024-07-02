export module flate:buffer;
import :symbols;
import hai;
import jute;

namespace flate {
constexpr const auto max_repeat_jump = 32768;
template <unsigned buf_size = max_repeat_jump * 2> class buffer {

  hai::array<uint8_t> m_buf{buf_size};
  unsigned m_rd{};
  unsigned m_wr{};

  [[nodiscard]] static constexpr auto wrap(unsigned n, unsigned d) noexcept {
    return (n + d) % buf_size;
  }
  constexpr void put(uint8_t c) noexcept {
    m_buf[m_wr] = c;
    m_wr = wrap(m_wr, 1);
  }

public:
  [[nodiscard]] constexpr bool empty() const noexcept { return m_rd == m_wr; }
  [[nodiscard]] constexpr uint8_t read() noexcept {
    auto p = m_rd;
    m_rd = wrap(m_rd, 1);
    return m_buf[p];
  }

  constexpr bool visit(const symbols::symbol &r) {
    switch (r.type) {
    case symbols::type::raw:
      put(r.c);
      return true;
    case symbols::type::repeat:
      for (unsigned i = 0; i < r.len; i++) {
        put(m_buf[wrap(m_wr, -r.dist)]);
      }
      m_rd = wrap(m_wr, -r.len);
      return true;
    case symbols::type::end:
      return false;
    case symbols::type::nil:
      // should never happen
      break;
    }

    throw 0;
  }
};
} // namespace flate

using namespace flate;
using namespace flate::symbols;

static_assert(buffer{}.empty());

static constexpr const auto assert = [](auto b) {
  if (!b)
    throw 0;
};

template <auto N>
static constexpr auto visit_and_read(buffer<N> &b, uint8_t c) {
  if (!b.visit(symbol{type::raw, 0, 0, c}))
    return false;
  if (b.empty())
    return false;
  if (b.read() != c)
    return false;
  return b.empty();
}
template <auto N>
static constexpr void visit_and_read(buffer<N> &b, jute::view str) {
  for (uint8_t c : str)
    assert(b.visit(symbol{type::raw, 0, 0, c}));

  assert(!b.empty());

  for (auto c : str)
    assert(b.read() == c);

  assert(b.empty());
}
template <auto N>
static constexpr auto visit_and_read(buffer<N> &b, jute::view str,
                                     unsigned dist) {
  if (!b.visit(
          symbol{type::repeat, static_cast<unsigned int>(str.size()), dist}))
    return false;
  for (uint8_t c : str) {
    if (b.empty())
      return false;
    if (b.read() != c)
      return false;
  }
  return b.empty();
}

static_assert([] {
  buffer b{};
  if (!visit_and_read(b, 'y'))
    return false;
  if (!visit_and_read(b, 'u'))
    return false;
  if (!visit_and_read(b, 'p'))
    return false;
  return true;
}());
static_assert([] {
  buffer b{};
  if (!visit_and_read(b, 't'))
    return false;
  if (!visit_and_read(b, 'e'))
    return false;
  if (!visit_and_read(b, 's'))
    return false;
  if (!visit_and_read(b, "te", 3))
    return false;
  return true;
}());
static_assert([] {
  buffer b{};
  if (!visit_and_read(b, 't'))
    return false;
  if (!visit_and_read(b, 'e'))
    return false;
  if (!visit_and_read(b, 's'))
    return false;
  if (!visit_and_read(b, "te", 3))
    return false;
  if (!visit_and_read(b, ' '))
    return false;
  if (!visit_and_read(b, "test", 6))
    return false; // NOLINT
  return true;
}());
static_assert([] {
  buffer<4> b{};
  visit_and_read(b, "aa");
  // D.ABC -- We can write past overflows
  visit_and_read(b, "bcd");
  // DC.BC -- We can read past underflow
  assert(visit_and_read(b, "c", 2));
  // C.CCD -- We can read past underflow and back into overflow
  assert(visit_and_read(b, "cdc", 3));
  return true;
}());
