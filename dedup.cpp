module flate;

namespace flate {
constexpr symbols::symbol dedup(const uint8_t *data, unsigned len,
                                unsigned &ptr) {
  if (ptr < 0 || ptr >= len)
    return {type::end};

  return {type::raw, 0, 0, data[ptr++]};
}
} // namespace flate

static_assert([] {
  constexpr const uint8_t data[]{"HEYHEYHEY YO YO YO"};

  unsigned ptr{};
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

  // assert_next({type::end});
  return true;
}());
