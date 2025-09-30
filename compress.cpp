module flate;
import :dedup;
import hai;

static constexpr hai::varray<unsigned char> compress(const uint8_t * data, unsigned size) {
  bitwriter bitw{size};
  bitw.write(1, 1); // Final block
  bitw.write(1, 2); // Static Huffman tables

  auto syms = dedup_all(data, size);
  static_huffman_encode(syms, bitw);
  bitw.flush();
  return bitw.take();

  // We could also use "stored" as optimisation if the uncompressed size is
  // smaller than the buffer size plus 1 plus 2 "words".
  //
  // The spec expects us to do something like:
  // 1 bit = 1    "last block"
  // 2 bits = 00  "stored"
  // 1 "word" = length (no idea what is a "word" for them)
  // 1 "word" = ~length
  // Data follows
}

static_assert([] {
  // TODO: test extra bits

  constexpr const uint8_t data[]{"HUEHUEHUEHUE"};
  // H(72),   U(85),   E(69),   rep(9,3)[263], 0,       end
  // 01111000 10000101 01110101 0000111 00010  00110000 0000000

  auto buf = ::compress(data, sizeof(data));

  constexpr const auto assert = [](auto res, auto exp) { if (res != exp) throw 0; };
  assert(buf[0], 0b11110011);
  assert(buf[1], 0b00001000);
  assert(buf[2], 0b01110101);
  assert(buf[3], 0b10000101);
  assert(buf[4], 0b00100011);
  assert(buf[5], 0b00000110);
  assert(buf[6], 0b00000000);
  assert(buf.size(), 7);
  return true;
}());

hai::varray<unsigned char> flate::compress(const void * data, unsigned size) {
  return ::compress(static_cast<const uint8_t *>(data), size);
}
