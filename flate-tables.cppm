export module flate:tables;
import :huffman;
import hai;

namespace flate::tables {
struct bit_pair {
  unsigned bits;
  unsigned second;
};
constexpr bool operator==(const bit_pair &s, const bit_pair &r) {
  return s.bits == r.bits && s.second == r.second;
}

constexpr const auto max_lens_code = 285U;
struct bit_pairs {
  bit_pair data[max_lens_code + 1];
};
constexpr const auto bitlens = [] {
  constexpr const auto first_parametric_code = 261U;
  constexpr const auto min_lens_code = 257U;

  bit_pairs res{};
  for (auto i = min_lens_code; i < first_parametric_code; i++) {
    res.data[i] = {0, i - min_lens_code + 3U};
  }
  for (auto i = first_parametric_code; i < max_lens_code; i++) {
    const auto bits = (i - first_parametric_code) / 4U;
    res.data[i] = {bits, res.data[i - 1].second + (1U << res.data[i - 1].bits)};
  }

  // For some reason, this does not follow the pattern
  res.data[max_lens_code] = {0, 258};
  return res;
}();
static_assert(bitlens.data[257] == bit_pair{0, 3});   // NOLINT
static_assert(bitlens.data[258] == bit_pair{0, 4});   // NOLINT
static_assert(bitlens.data[259] == bit_pair{0, 5});   // NOLINT
static_assert(bitlens.data[260] == bit_pair{0, 6});   // NOLINT
static_assert(bitlens.data[261] == bit_pair{0, 7});   // NOLINT
static_assert(bitlens.data[264] == bit_pair{0, 10});  // NOLINT
static_assert(bitlens.data[280] == bit_pair{4, 115}); // NOLINT
static_assert(bitlens.data[281] == bit_pair{5, 131}); // NOLINT
static_assert(bitlens.data[284] == bit_pair{5, 227}); // NOLINT
static_assert(bitlens.data[285] == bit_pair{0, 258}); // NOLINT

constexpr const auto max_dists_code = 29U;
struct bit_dists {
  bit_pair data[max_dists_code + 1];
};
static constexpr const auto bitdists = [] {
  bit_dists res{};
  res.data[0] = {0, 1};
  res.data[1] = {0, 2};
  for (auto i = 2U; i <= max_dists_code; i++) {
    const auto bits = (i - 2U) / 2U;
    res.data[i] = {bits, res.data[i - 1].second + (1U << res.data[i - 1].bits)};
  }
  return res;
}();
static_assert(bitdists.data[0] == bit_pair{0, 1});       // NOLINT
static_assert(bitdists.data[1] == bit_pair{0, 2});       // NOLINT
static_assert(bitdists.data[2] == bit_pair{0, 3});       // NOLINT
static_assert(bitdists.data[3] == bit_pair{0, 4});       // NOLINT
static_assert(bitdists.data[4] == bit_pair{1, 5});       // NOLINT
static_assert(bitdists.data[5] == bit_pair{1, 7});       // NOLINT
static_assert(bitdists.data[6] == bit_pair{2, 9});       // NOLINT
static_assert(bitdists.data[29] == bit_pair{13, 24577}); // NOLINT

struct huff_tables {
  huffman_codes hlist;
  huffman_codes hdist;
};
[[nodiscard]] constexpr huff_tables
create_tables(const hai::array<unsigned> &hlist_hdist, unsigned hlist_len) {
  return huff_tables{
      .hlist = create_huffman_codes(hlist_hdist.begin(), hlist_len),
      .hdist = create_huffman_codes(hlist_hdist.begin() + hlist_len,
                                    hlist_hdist.size() - hlist_len),
  };
}
static_assert([] {
  const auto data = hai::array<unsigned>::make(2, 2, 2, 1, 1);
  const auto tables = create_tables(data, 3);
  if (tables.hlist.counts[2] != 3)
    return false;
  if (tables.hlist.counts[1] != 0)
    return false;
  if (tables.hdist.counts.size() == 3)
    return false;
  if (tables.hdist.counts[1] != 2)
    return false;
  return true;
}());

constexpr auto create_fixed_huffman_table() {
  hai::array<unsigned> fixed_hlist{288};
  for (auto i = 0; i < 144; i++)
    fixed_hlist[i] = 8;
  for (auto i = 144; i < 256; i++)
    fixed_hlist[i] = 9;
  for (auto i = 256; i < 280; i++)
    fixed_hlist[i] = 7;
  for (auto i = 280; i < 288; i++)
    fixed_hlist[i] = 8;

  return huff_tables{
      .hlist = create_huffman_codes(fixed_hlist.begin(), fixed_hlist.size()),
      .hdist = {},
  };
}
static_assert(create_fixed_huffman_table().hlist.counts[0] == 0);
static_assert(create_fixed_huffman_table().hlist.counts[7] == 24);
static_assert(create_fixed_huffman_table().hlist.counts[8] == 144 + 8);
static_assert(create_fixed_huffman_table().hlist.counts[9] == 112);
} // namespace flate::tables
