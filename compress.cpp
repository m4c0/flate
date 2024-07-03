module flate;
import :dedup;
import hai;

static constexpr auto compress(yoyo::writer &w, const uint8_t *data,
                               unsigned size) {
  bitwriter bitw{size};

  auto syms = dedup_all(data, size);
  static_huffman_encode(syms, bitw);

  if (bitw.buffer().size() == size)
    // write using no-comp
    return mno::req<void>{};

  // write using comp
  return mno::req<void>{};
}

static_assert([] {
  constexpr const uint8_t data[]{"HUEHUEHUE"};

  hai::array<uint8_t> buf{6};
  yoyo::memwriter w{buf};

  if (!::compress(w, data, sizeof(data)).is_valid())
    throw 0;

  return true;
}());

mno::req<void> flate::compress(yoyo::writer &w, const void *data,
                               unsigned size) {
  return ::compress(w, static_cast<const uint8_t *>(data), size);
}
