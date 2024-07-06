module flate;
import :dedup;
import hai;

static constexpr auto compress(yoyo::writer &w, const uint8_t *data,
                               unsigned size) {
  bitwriter bitw{size};
  bitw.write(1, 1); // Final block
  bitw.write(1, 2); // Static Huffman tables

  auto syms = dedup_all(data, size);
  static_huffman_encode(syms, bitw);
  bitw.flush();

  auto &buf = bitw.buffer();
  if (buf.size() != size)
    return w.write(buf.begin(), buf.size());

  // write using no-comp
  return mno::req<void>::failed("TODO: implement uncompressed case");
}

static_assert([] {
  // TODO: test extra bits

  constexpr const uint8_t data[]{"HUEHUEHUEHUE"};
  // H(72),   U(85),   E(69),   rep(9,3)[263], 0,       end
  // 01111000 10000101 01110101 0000111 00010  00110000 0000000

  hai::array<uint8_t> buf{16};
  yoyo::memwriter w{buf};

  if (!::compress(w, data, sizeof(data)).is_valid())
    throw 0;

  constexpr const auto assert = [](auto res, auto exp) {
    if (res != exp)
      throw 0;
  };
  assert(buf[0], 0b11110011);
  assert(buf[1], 0b00001000);
  assert(buf[2], 0b01110101);
  assert(buf[3], 0b10000101);
  assert(buf[4], 0b00001011);
  assert(buf[5], 0b00000110);
  assert(buf[6], 0b00000000);
  assert(w.raw_pos(), 7);
  return true;
}());

mno::req<void> flate::compress(yoyo::writer &w, const void *data,
                               unsigned size) {
  return ::compress(w, static_cast<const uint8_t *>(data), size);
}
