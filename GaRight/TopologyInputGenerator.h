#pragma once
#include "Matrix.h"

#include <random>

struct TopologyInputGenerator final {
  struct TrafficOptions final {
    double nonZeroChance;
    size_t amount;
    size_t offset;
  };

  struct BandwidthOptions final {
    size_t amount;
    size_t offset;
  };

  /**
   * Generates a matrix with one-sided traffic between hosts
   */
  static Matrix<size_t> CreateTrafficMatrix(size_t hosts, const TrafficOptions& options, std::mt19937_64& rng, std::uniform_real_distribution<>& dist) {
    Matrix<size_t> matrix(hosts, hosts);

    for (size_t row = 0; row < hosts; ++row) {
      for (size_t col = 0; col < hosts; ++col) {
        if (row == col) {
          continue;
        }

        matrix(row, col) = dist(rng) > options.nonZeroChance ? 0 : rng() % options.amount + options.offset;
      }
    }

    return matrix;
  }

  /**
   * Generates a symmetrical matrix with bandwidth of channels between routers
   */
  static SymmetricalMatrix<size_t> CreateBandwidthMatrix(size_t routers, const BandwidthOptions& options, std::mt19937_64& rng) {
    SymmetricalMatrix<size_t> matrix(routers);

    for (size_t row = 0; row < routers; ++row) {
      for (size_t col = row + 1; col < routers; ++col) {
        if (row == col) {
          continue;
        }

        matrix.Set(row, col, rng() % options.amount + options.offset);
      }
    }

    return matrix;
  }
};
