#include "CPA.hpp"

#include <rankcpp/Dimensions.hpp>
#include <rankcpp/Key.hpp>
#include <rankcpp/ScoresTable.hpp>

#include <rankcpp/BoostBigUint.hpp>
#include <rankcpp/Rank.hpp>
#include <rankcpp/WeightTable.hpp>

#include <CLI/CLI.hpp>

#include <gsl/span>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <limits>
#include <random>
#include <string>
#include <type_traits>
#include <vector>

namespace rankcpp {

void simulateCPA(std::size_t traceCount, double snr, std::uint64_t rngSeed,
                 std::uint32_t precision) {
  std::mt19937 rng(rngSeed);
  (void) precision;

  //  - random key
  Key<128> key = randomKey<128>(rng);

  SimulatedHWCPA cpa(key.asBytes(), traceCount, snr, rngSeed + 1);
  auto scores = cpa.nextRandomAttack();


  // Rank
  using RankType = BoostBigUint<128>;
  using WeightType = std::uint64_t;
  scores.log2();
  scores.abs();
  auto const weights = mapToWeight<double, WeightType>(scores, precision);
  auto const keyWeight = weights.weightForKey(key);
  std::cout << " keyWeight: " << keyWeight << " / ";

  auto const ckRank = rank<RankType>(keyWeight, weights);
  std::cout << "ck: " << ckRank << " / " << log2(ckRank) << "\n";
}

} /* namespace rankcpp */

auto main(int argc, char **argv) -> int {
  CLI::App app{"Run the key rank algorithm on a scores table"};

  std::string filename = "default";
  app.add_option("-f,--file", filename, "A help string");

  CLI11_PARSE(app, argc, argv);

  rankcpp::simulateCPA(100, 0.25, 1234, 16);

  return 0;
}
