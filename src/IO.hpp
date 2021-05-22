#pragma once

#include <rankcpp/Key.hpp>
#include <rankcpp/ScoresTable.hpp>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

namespace rankcpp {

inline void writeScoresToFile(ScoresTable<double> const &scores,
                              std::filesystem::path filepath) {
  std::ofstream outFile(filepath, std::ios::out | std::ios::binary);
  // std::cout << scores.allScores()[0] << "\n";
  for (auto const &score : scores.allScores()) {
    outFile.write(reinterpret_cast<const char *>(&score), sizeof(double));
  }
}

template <std::uint32_t KeyLenBits>
inline void writeKeyToFile(Key<KeyLenBits> const &key,
                           std::filesystem::path filepath) {
  std::ofstream outFile(filepath, std::ios::out);
  outFile << std::hex;
  for (auto const &keyByte : key.asBytes()) {
    outFile << std::setw(2) << std::setfill('0')
            << static_cast<std::uint32_t>(keyByte);
  }
}

template <typename DimensionsType>
inline auto readScoresFromFile(DimensionsType dimensions,
                               std::filesystem::path filepath)
    -> ScoresTable<double> {
  // read bytes
  std::ifstream ifs(filepath, std::ios::binary | std::ios::ate);
  if (!ifs)
    throw std::runtime_error(std::strerror(errno));

  auto end = ifs.tellg();
  ifs.seekg(0, std::ios::beg);
  auto size = std::size_t(end - ifs.tellg());
  if (size == 0)
    throw std::runtime_error(std::strerror(errno));

  std::vector<std::byte> buf(size);

  if (!ifs.read((char *)buf.data(), buf.size()))
    throw std::runtime_error(std::strerror(errno));

  // reinterpret as doubles
  std::vector<double> allScores(buf.size() / sizeof(double));
  for (std::size_t i = 0; i < allScores.size(); i++) {
    auto const first = std::cbegin(buf) + i * sizeof(double);
    auto const last = first + sizeof(double);
    std::copy(first, last, reinterpret_cast<std::byte *>(&allScores[i]));
  }

  ScoresTable<double> scores(dimensions, allScores);
  return scores;
}

template <std::uint32_t KeyLenBits>
inline auto readKeyFromFile(std::filesystem::path filepath) -> Key<KeyLenBits> {
  std::ifstream ifs(filepath);
  std::string const keyHex((std::istreambuf_iterator<char>(ifs)),
                           (std::istreambuf_iterator<char>()));

  Key<KeyLenBits> const key(keyHex);
  return key;
}

} /* namespace rankcpp */
