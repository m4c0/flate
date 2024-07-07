export module flate:dedup;
import :namffuh;
import hai;

namespace flate {
constexpr symbols::symbol dedup(const uint8_t *data, int len, int &ptr) {
  // static constexpr const auto max_dist = 32768;
  static constexpr const auto max_dist = 1024;
  static constexpr const auto max_len = 258;

  if (ptr < 0 || ptr >= len)
    return {type::end};

  int l_len = 0;
  int l_dist = 0;

  for (auto i = ptr - 1; i >= 0 && (ptr - i < max_dist); i--) {
    if (data[i] != data[ptr])
      continue;

    int j{};
    for (; ptr + j < len && j < max_len; j++) {
      if (data[i + j] != data[ptr + j])
        break;
    }
    if (j > l_len) {
      l_len = j;
      l_dist = ptr - i;
    }
  }

  if (l_len <= 2) // min_dist = 3
    return {type::raw, 0, 0, data[ptr++]};

  ptr += l_len;
  return {type::repeat, static_cast<unsigned>(l_len),
          static_cast<unsigned>(l_dist)};
}

constexpr hai::varray<symbols::symbol> dedup_all(const uint8_t *data, int len) {
  hai::varray<symbols::symbol> res{static_cast<unsigned>(len + 1)};
  int ptr = 0;
  do {
    res.push_back(dedup(data, len, ptr));
  } while (res[res.size() - 1].type != type::end);
  return res;
}
} // namespace flate

static constexpr void assert_symbol(symbol sym, symbol expected) {
  if (sym.type != expected.type)
    throw 0;
  if (sym.len != expected.len)
    throw 0;
  if (sym.dist != expected.dist)
    throw 0;
  if (sym.c != expected.c)
    throw 0;
}
static_assert([] {
  constexpr const uint8_t data[]{"HEYHEYHEY YOYO YO"};

  int ptr{};
  const auto assert_next = [&](symbol expected) {
    assert_symbol(dedup(data, sizeof(data), ptr), expected);
  };

  assert_next({type::raw, 0, 0, 'H'});
  assert_next({type::raw, 0, 0, 'E'});
  assert_next({type::raw, 0, 0, 'Y'});
  assert_next({type::repeat, 6, 3});
  assert_next({type::raw, 0, 0, ' '});
  assert_next({type::raw, 0, 0, 'Y'});
  assert_next({type::raw, 0, 0, 'O'});
  assert_next({type::raw, 0, 0, 'Y'});
  assert_next({type::raw, 0, 0, 'O'});
  assert_next({type::repeat, 3, 5});
  assert_next({type::raw, 0, 0, '\0'});
  assert_next({type::end});
  return true;
}());
static_assert([] {
  constexpr const uint8_t data[300]{};

  int ptr{};
  const auto assert_next = [&](symbol expected) {
    assert_symbol(dedup(data, sizeof(data), ptr), expected);
  };

  assert_next({type::raw, 0, 0, '\0'});
  assert_next({type::repeat, 258, 1});
  assert_next({type::repeat, 41, 1});
  assert_next({type::end});
  return true;
}());

static_assert([] {
  constexpr const uint8_t data[]{"HEYHEY"};

  auto res = dedup_all(data, sizeof(data));
  assert_symbol(res[0], {type::raw, 0, 0, 'H'});
  assert_symbol(res[1], {type::raw, 0, 0, 'E'});
  assert_symbol(res[2], {type::raw, 0, 0, 'Y'});
  assert_symbol(res[3], {type::repeat, 3, 3});
  assert_symbol(res[4], {type::raw, 0, 0, '\0'});
  assert_symbol(res[5], {type::end});
  return true;
}());
