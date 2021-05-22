#include "CPA.hpp"
#include "IO.hpp"

#include <rankcpp/Dimensions.hpp>
#include <rankcpp/Key.hpp>
#include <rankcpp/ScoresTable.hpp>

#include <CLI/CLI.hpp>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <random>

namespace rankcpp {

template <std::size_t ByteCount = 16>
void simulateCPA(std::size_t traceCount, double snr, std::uint64_t rngSeed,
                 std::filesystem::path filepath,
                 std::filesystem::path keyFilepath) {
  constexpr std::size_t const BitCount = ByteCount * 8;

  // generate a random key
  std::mt19937 rng(rngSeed);
  auto const key = randomKey<BitCount>(rng);

  // do CPA and get a scores table
  SimulatedHWCPA<ByteCount> cpa(key.asBytes(), traceCount, snr, rngSeed + 1);
  auto scores = cpa.nextRandomAttack();
  scores.log2();
  scores.abs();

  // write scores to disk
  writeScoresToFile(scores, filepath);
  writeKeyToFile(key, keyFilepath);
}

} /* namespace rankcpp */

auto main(int argc, char **argv) -> int {
  CLI::App app{
      "Simulate a CPA on an arbitrary number of AES SubBytes operations"};

  std::filesystem::path filepath = "scores.bin";
  app.add_option("-f,--file", filepath, "Write scores to this file");

  std::filesystem::path keyFilepath = "key.txt";
  app.add_option("-k,--keyfile", keyFilepath, "Write key to this file");

  std::size_t traceCount = 100;
  app.add_option("-t,--trace-count", traceCount,
                 "Number of traces to simulate");

  double snr = 0.125;
  app.add_option("-s,--snr", snr, "SNR for simulated traces");

  std::uint64_t rngSeed = 0xABCDEF;
  app.add_option("-r,--seed", rngSeed, "Rng seed for simulation of trace data");

  CLI11_PARSE(app, argc, argv);

  constexpr std::size_t const ByteCount = 128 / 8;
  rankcpp::simulateCPA<ByteCount>(traceCount, snr, rngSeed, filepath,
                                  keyFilepath);

  return 0;
}