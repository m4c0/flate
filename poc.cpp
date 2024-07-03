#pragma leco tool

import flate;
import hai;
import silog;
import traits;
import yoyo;

using namespace traits::ints;

constexpr const uint8_t lorem_ipsum[] = R"(
Ut et praesentium perspiciatis hic. Doloribus ut architecto non et eligendi. Hic aliquid ea corporis.

Ipsa asperiores architecto ad facere et. Vel et temporibus rerum beatae corporis. Eius odit et tenetur vero et et necessitatibus. Totam error dicta non in atque dolor omnis ab. Unde molestiae repellendus at qui est.

Facilis cum ea ullam qui et et fugit. Est atque non repellendus similique fugiat eaque adipisci. Ut inventore sequi vero voluptatem quibusdam aut nesciunt. Nostrum culpa non ipsam natus molestias.

Numquam iste est consequatur optio voluptatibus nihil. Tenetur non ipsa id. Quia consequatur doloribus et in occaecati maxime voluptate. Consectetur tempora laudantium non. Eos autem porro ut aliquam consequuntur.

Illum voluptatibus aut esse. Omnis asperiores praesentium quo voluptas quia quidem ut dolores. Ut quod nihil illo consequatur aut.
)";

int main() {
  silog::log(silog::info, "Original string has %d bytes",
             static_cast<int>(sizeof(lorem_ipsum)));

  hai::array<uint8_t> comp{10240};
  yoyo::memwriter wr{comp};
  yoyo::memreader rd{comp.begin(), comp.size()};

  flate::bitstream bs{&rd};

  hai::array<uint8_t> decomp{10240};

  return flate::compress(wr, lorem_ipsum, sizeof(lorem_ipsum))
      .map([&] {
        silog::log(silog::info, "Compressed to %d bytes", wr.raw_pos());
      })
      .fmap([&] { return flate::huffman_reader::create(&bs); })
      .fmap([&](auto &hr) {
        return hr.read_up_to(decomp.begin(), decomp.size());
      })
      .map([](auto n) { silog::log(silog::info, "Got %d bytes back", n); })
      .map([] { return 0; })
      .log_error([&] {
        silog::log(silog::info, "Total of %d bytes read", rd.raw_pos());
        return 1;
      });
}
