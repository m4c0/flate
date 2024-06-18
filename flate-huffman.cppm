export module flate:huffman;
import :bitstream;
import hai;
import yoyo;

namespace flate {
using uint_array = hai::array<unsigned>;

struct huffman_codes {
  // Counts per bit length
  uint_array counts{};
  // Symbol per offset
  uint_array indexes{};
};

static constexpr auto max(const uint_array &array) {
  unsigned max = array[0];
  for (auto i = 1; i < array.size(); i++) {
    if (array[i] < max)
      continue;

    max = array[i];
  }
  return max;
}

// section 3.2.2 of RFC 1951 - using a variant based on ZLIB algorithms
[[nodiscard]] constexpr auto create_huffman_codes(const uint_array &lengths) {
  const auto max_codes = lengths.size();
  const auto max_bits = max(lengths);
  huffman_codes res;
  res.counts = uint_array{max_bits + 1};

  for (auto &e : res.counts) {
    e = 0;
  }
  for (auto len : lengths) {
    res.counts[len]++;
  }

  uint_array offsets{max_bits + 1};
  offsets[1] = 0;
  for (auto bits = 1; bits < max_bits; bits++) {
    offsets[bits + 1] = offsets[bits] + res.counts[bits];
  }

  res.indexes = uint_array{offsets[max_bits] + res.counts[max_bits]};
  for (auto n = 0; n < max_codes; n++) {
    auto len = lengths[n];
    if (len != 0) {
      res.indexes[offsets[len]] = n;
      offsets[len]++;
    }
  }

  return res;
}

[[nodiscard]] constexpr auto decode_huffman(const huffman_codes &hc,
                                            bitstream *bits) {
  unsigned code = 0;
  unsigned first = 0;
  unsigned index = 0;
  for (const auto *it = hc.counts.begin() + 1; it != hc.counts.end(); ++it) {
    auto count = *it;

    auto b = bits->next<1>();
    if (!b.is_valid())
      return b;

    code |= b.unwrap(0);
    if (code < first + count) {
      return mno::req{hc.indexes[index + code - first]};
    }
    index += count;
    first = (first + count) << 1U;
    code <<= 1U;
  }
  return mno::req<unsigned>::failed("invalid huffman code");
}
} // namespace flate

using namespace flate;

static constexpr bool operator==(const auto &a, const auto &b) noexcept {
  if (a.size() != b.size())
    return false;
  for (auto i = 0; i < a.size(); i++) {
    if (a[i] != b[i])
      return false;
  }
  return true;
}
// Checks huffman table construction
static_assert([] {
  const auto expected_counts = uint_array::make(0, 0, 1, 5, 2);
  const auto expected_symbols = uint_array::make(5, 0, 1, 2, 3, 4, 6, 7);

  const auto hfc =
      create_huffman_codes(uint_array::make(3, 3, 3, 3, 3, 2, 4, 4));
  if (hfc.counts != expected_counts)
    return false;
  if (hfc.indexes != expected_symbols)
    return false;
  return true;
}());

// Checks again with unused symbols
static_assert([] {
  const auto expected_counts = uint_array::make(4, 0, 0, 5);
  const auto expected_symbols = uint_array::make(0, 2, 4, 6, 8);

  const auto hfc = create_huffman_codes(
      uint_array::make(3U, 0U, 3U, 0U, 3U, 0U, 3U, 0U, 3U));
  if (hfc.counts != expected_counts)
    return false;
  if (hfc.indexes != expected_symbols)
    return false;
  return true;
}());

// Checks symbol lookup
// 0 F 00
// 1 A 010
// 2 B 011
// 3 C 100
// 4 D 101
// 5 E 110
// 6 G 1110
// 7 H 1111
static_assert([] {
  const auto hfc =
      create_huffman_codes(uint_array::make(3, 3, 3, 3, 3, 2, 4, 4));
  auto r = yoyo::ce_reader{0b11100100, 0b01111011}; // NOLINT
  bitstream b{&r};

  const auto expected_result = uint_array::make(5, 2, 7, 3, 6);
  for (auto er : expected_result) {
    if (decode_huffman(hfc, &b) != er)
      return false;
  }
  return true;
}());
