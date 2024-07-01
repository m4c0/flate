module flate;

namespace flate {
constexpr symbols::symbol dedup(const uint8_t *data, int len, int &ptr) {
  static constexpr const auto max_dist = 32768;
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
} // namespace flate

static_assert([] {
  constexpr const uint8_t data[]{"HEYHEYHEY YOYO YO"};

  int ptr{};
  const auto assert_next = [&](symbol expected) {
    auto sym = dedup(data, sizeof(data), ptr);
    if (sym.type != expected.type)
      throw 0;
    if (sym.len != expected.len)
      throw 0;
    if (sym.dist != expected.dist)
      throw 0;
    if (sym.c != expected.c)
      throw 0;
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
    auto sym = dedup(data, sizeof(data), ptr);
    if (sym.type != expected.type)
      throw 0;
    if (sym.len != expected.len)
      throw 0;
    if (sym.dist != expected.dist)
      throw 0;
    if (sym.c != expected.c)
      throw 0;
  };

  assert_next({type::raw, 0, 0, '\0'});
  assert_next({type::repeat, 258, 1});
  assert_next({type::repeat, 41, 1});
  assert_next({type::end});
  return true;
}());
