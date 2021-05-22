#include "IO.hpp"

#include <rankcpp/BoostBigUint.hpp>
#include <rankcpp/Dimensions.hpp>
#include <rankcpp/Key.hpp>
#include <rankcpp/Rank.hpp>
#include <rankcpp/ScoresTable.hpp>
#include <rankcpp/WeightTable.hpp>

#include <CLI/CLI.hpp>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>

namespace rankcpp {

using DurationType = std::chrono::duration<std::uint64_t, std::nano>;

template <std::uint32_t KeyLenBits, typename DimensionsType>
void estimateRank(DimensionsType dimensions, std::uint32_t precision,
                  std::filesystem::path filepath,
                  std::filesystem::path keyFilepath) {
  // read scores
  auto const scores = readScoresFromFile(dimensions, filepath);
  auto const key = readKeyFromFile<KeyLenBits>(keyFilepath);

  // rank
  using RankType = BoostBigUint<KeyLenBits>;
  using WeightType = std::uint64_t;
  auto const weights = mapToWeight<double, WeightType>(scores, precision);
  auto const keyWeight = weights.weightForKey(key);
  std::cout << "keyWeight: " << keyWeight << "\n";
  auto const start = std::chrono::high_resolution_clock::now();
  auto const ckRank = rank<RankType>(keyWeight, weights);
  auto const stop = std::chrono::high_resolution_clock::now();
  auto const durationNs =
      static_cast<double>(DurationType{stop - start}.count());
  auto const durationS = durationNs / 1000.0 / 1000.0 / 1000.0;
  std::cout << "rank: " << ckRank << "\n";
  std::cout << "rank (log2): " << log2(ckRank) << "\n";
  std::cout << "duration (s): " << durationS << "\n";
}

} /* namespace rankcpp */

auto main(int argc, char **argv) -> int {
  CLI::App app{"Run the key rank algorithm on a scores table"};

  std::filesystem::path filepath = "scores.bin";
  app.add_option("-f,--file", filepath, "Read scores to from file");

  std::filesystem::path keyFilepath = "key.txt";
  app.add_option("-k,--keyfile", keyFilepath, "Read key to from file");

  std::uint32_t precision = 16;
  app.add_option("-p,--precision", precision, "Rank precision in bits");

  CLI11_PARSE(app, argc, argv);

  constexpr std::size_t const ByteCount = 16;
  constexpr std::size_t const KeyLenBits = ByteCount * 8;
  rankcpp::Dimensions dimensions(ByteCount, 8);
  rankcpp::estimateRank<KeyLenBits>(dimensions, precision, filepath,
                                    keyFilepath);

  return 0;
}