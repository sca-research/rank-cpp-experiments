#pragma once

#include <rankcpp/Dimensions.hpp>
#include <rankcpp/ScoresTable.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <random>
#include <vector>

namespace rankcpp {

template <std::size_t ByteCount = 16>
class SimulatedHWCPA {
public:
  SimulatedHWCPA(std::array<uint8_t, ByteCount> key, std::size_t traceCount,
                 double snr, std::uint64_t rngSeed)
      : key_(key), traceCount_(traceCount), generator(rngSeed),
        normalDistribution(0.0, std::sqrt(2.0 / snr)),
        uniformDistribution(0, 255) {}

  ScoresTable<double> nextRandomAttack() {
    // Generate next random plaintexts
    std::vector<uint8_t> allPlaintextBytes(traceCount_ * ByteCount);
    std::generate(allPlaintextBytes.begin(), allPlaintextBytes.end(),
                  [&] { return uniformDistribution(generator); });

    // Generate next trace values
    // traces_for_byte_0||traces_for_byte_1||...
    std::vector<double> allTraceSamples(traceCount_ * ByteCount);
    for (std::size_t byteIndex = 0; byteIndex < ByteCount; byteIndex++) {
      for (std::size_t traceIndex = 0; traceIndex < traceCount_; traceIndex++) {
        uint8_t const plaintextByte =
            allPlaintextBytes[byteIndex * traceCount_ + traceIndex];
        uint8_t const intermediateValue = sBox(plaintextByte ^ key_[byteIndex]);
        double const leakage = hammingWeight(intermediateValue);
        double const noise = normalDistribution(generator);
        allTraceSamples[byteIndex * traceCount_ + traceIndex] = leakage + noise;
      }
    }

    // Prepare distinguishing table scores
    Dimensions const dims(ByteCount, 8);
    ScoresTable<double> scores(dims);
    for (std::size_t byteIndex = 0; byteIndex < ByteCount; byteIndex++) {
      std::array<double, 256> corrs;
      for (std::size_t subkey = 0; subkey < 256; subkey++) {
        std::vector<double> hypValues(traceCount_);
        for (std::size_t traceIndex = 0; traceIndex < traceCount_; traceIndex++) {
          uint8_t const plaintextByte =
              allPlaintextBytes[byteIndex * traceCount_ + traceIndex];
          uint64_t const intermediateValue =
              sBox(plaintextByte ^ static_cast<uint8_t>(subkey));
          hypValues[traceIndex] = hammingWeight(intermediateValue);
        }
        // Now correlate
        double const corr = pearsonsCorrelation(
            allTraceSamples.begin() + (byteIndex * traceCount_),
            allTraceSamples.begin() + (byteIndex * traceCount_) + traceCount_,
            hypValues.begin());
        corrs[subkey] = std::fabs(corr);
      }
      scores.addScores(dims.asSpans()[byteIndex], std::cbegin(corrs),
                       std::cend(corrs));
    }

    return scores;
  }

private:
  std::array<uint8_t, ByteCount> const key_;
  uint64_t const traceCount_;
  std::mt19937 generator;
  std::normal_distribution<double> normalDistribution;
  std::uniform_int_distribution<uint8_t> uniformDistribution;

  double pearsonsCorrelation(std::vector<double>::const_iterator xBegin,
                             std::vector<double>::const_iterator xEnd,
                             std::vector<double>::const_iterator yBegin) {
    uint64_t const size = (uint64_t)std::distance(xBegin, xEnd);
    double x = 0.0;
    double x2 = 0.0;
    double y = 0.0;
    double y2 = 0.0;
    double xy = 0.0;
    for (uint64_t index = 0; index < size; index++) {
      x += xBegin[index];
      x2 += (xBegin[index] * xBegin[index]);
      y += yBegin[index];
      y2 += (yBegin[index] * yBegin[index]);
      xy += (xBegin[index] * yBegin[index]);
    }
    double const n = static_cast<double>(size);
    double const xMean = x / n;
    double const yMean = y / n;
    double const rNumerator = xy - (n * xMean * yMean);
    double const rDenominator = std::sqrt(x2 - (n * xMean * xMean)) *
                                std::sqrt(y2 - (n * yMean * yMean));
    return rNumerator / rDenominator;
  }

  double hammingWeight(std::uint64_t value) {
    std::uint64_t hw = value;
    hw -= (hw >> 1) & 0x5555555555555555;
    hw = (hw & 0x3333333333333333) + ((hw >> 2) & 0x3333333333333333);
    hw = (hw + (hw >> 4)) & 0x0f0f0f0f0f0f0f0f;
    hw = (hw * 0x0101010101010101) >> 56;
    return static_cast<double>(hw);
  }

  static uint8_t sBox(uint32_t index) {
    static const uint8_t a[] = {
        0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B,
        0xFE, 0xD7, 0xAB, 0x76, 0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0,
        0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0, 0xB7, 0xFD, 0x93, 0x26,
        0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
        0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2,
        0xEB, 0x27, 0xB2, 0x75, 0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0,
        0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84, 0x53, 0xD1, 0x00, 0xED,
        0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
        0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F,
        0x50, 0x3C, 0x9F, 0xA8, 0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5,
        0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2, 0xCD, 0x0C, 0x13, 0xEC,
        0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
        0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14,
        0xDE, 0x5E, 0x0B, 0xDB, 0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C,
        0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79, 0xE7, 0xC8, 0x37, 0x6D,
        0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
        0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F,
        0x4B, 0xBD, 0x8B, 0x8A, 0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E,
        0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E, 0xE1, 0xF8, 0x98, 0x11,
        0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
        0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F,
        0xB0, 0x54, 0xBB, 0x16};
    return a[index];
  }
};

} /* namespace rankcpp */
