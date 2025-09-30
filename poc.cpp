#pragma leco tool

import flate;
import hai;
import print;
import traits;
import yoyo;

using namespace traits::ints;

// Brainf**k quine, should compress nicely
constexpr const uint8_t lorem_ipsum[] =
    "->+>+++>>+>++>+>+++>>+>++>>>+>+>+>++>+>>>>+++>+>>++>+>+++>>++>++>>+>>+>++>"
    "++>+>>>>+++>+>>>>++>++>>>>+>>++>+>+++>>>++>>++++++>>+>>++>+>>>>+++>>+++++>"
    ">+>+++>>>++>>++>>+>>++>+>+++>>>++>>+++++++++++++>>+>>++>+>+++>+>+++>>>++>>"
    "++++>>+>>++>+>>>>+++>>+++++>>>>++>>>>+>+>++>>+++>+>>>>+++>+>>>>+++>+>>>>++"
    "+>>++>++>+>+++>+>++>++>>>>>>++>+>+++>>>>>+++>>>++>+>+++>+>+>++>>>>>>++>>>+"
    ">>>++>+>>>>+++>+>>>+>>++>+>++++++++++++++++++>>>>+>+>>>+>>++>+>+++>>>++>>+"
    "+++++++>>+>>++>+>>>>+++>>++++++>>>+>++>>+++>+>+>++>+>+++>>>>>+++>>>+>+>>++"
    ">+>+++>>>++>>++++++++>>+>>++>+>>>>+++>>++++>>+>+++>>>>>>++>+>+++>>+>++>>>>"
    "+>+>++>+>>>>+++>>+++>>>+[[->>+<<]<+]+++++[->+++++++++<]>.[+]>>[<<+++++++[-"
    ">+++++++++<]>-.------------------->-[-<.<+>>]<[+]<+>>>]<<<[-[-[-[>>+<+++++"
    "+[->+++++<]]>++++++++++++++<]>+++<]++++++[->+++++++<]>+<<<-[->>>++<<<]>[->"
    ">.<<]<<]";

int main() {
  putfn("Original string has %d bytes", static_cast<int>(sizeof(lorem_ipsum)));

  hai::array<uint8_t> comp{10240};
  yoyo::memwriter wr{comp};
  yoyo::memreader rd{comp.begin(), comp.size()};

  flate::bitstream bs{&rd};

  hai::array<uint8_t> decomp{10240};

  flate::compress(wr, lorem_ipsum, sizeof(lorem_ipsum));
  putfn("Compressed to %d bytes", wr.raw_pos()); 

  return flate::huffman_reader::create(&bs)
      .fmap([&](auto &hr) {
        return hr.read_up_to(decomp.begin(), decomp.size());
      })
      .map([](auto n) { putfn("Got %d bytes back", n); })
      .map([] { return 0; })
      .log_error([&] {
        putfn("Total of %d bytes read before error", rd.raw_pos());
        return 1;
      });
}
