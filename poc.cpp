#pragma leco tool

import flate;
import hai;
import print;
import traits;

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

  auto comp = flate::compress(lorem_ipsum, sizeof(lorem_ipsum));
  putfn("Compressed to %d bytes", comp.size()); 

  flate::bitstream bs { comp.begin(), comp.size() };
  hai::array<uint8_t> decomp{10240};

  return flate::huffman_reader::create(&bs)
      .fmap([&](auto &hr) {
        return hr.read_up_to(decomp.begin(), decomp.size());
      })
      .map([](auto n) { putfn("Got %d bytes back", n); })
      .map([] { return 0; })
      .log_error([&] { return 1; });
}
