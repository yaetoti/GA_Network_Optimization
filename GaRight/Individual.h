#pragma once
#include "Topology.h"

struct Individual final {
  explicit Individual(const TopologyInput& input, TopologyRandom& random)
    : m_input(input)
    , m_random(random)
    , m_configuration(TopologyConfiguration::CreateRandom(input, random))
    , m_fitness(CalculateFitness(input, m_configuration)) {
  }

  explicit Individual(const TopologyInput& input, TopologyRandom& random, const TopologyConfiguration& configuration)
    : m_input(input)
    , m_random(random)
    , m_configuration(configuration)
    , m_fitness(CalculateFitness(input, configuration)) {
  }

  Individual(const Individual& other)
  : m_input(other.m_input)
  , m_random(other.m_random)
  , m_configuration(other.m_configuration)
  , m_fitness(other.m_fitness) {
  }

  Individual(Individual&& other) noexcept
  : m_input(other.m_input)
  , m_random(other.m_random)
  , m_configuration(std::move(other.m_configuration))
  , m_fitness(other.m_fitness) {
  }

  Individual& operator=(const Individual& other) {
    if (this == &other) {
      return *this;
    }
    m_random = other.m_random;
    m_configuration = other.m_configuration;
    m_fitness = other.m_fitness;
    return *this;
  }

  Individual& operator=(Individual&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    m_random = other.m_random;
    m_configuration = std::move(other.m_configuration);
    m_fitness = other.m_fitness;
    return *this;
  }

  static Individual Cross(const TopologyInput& input, const Individual& lhs, const Individual& rhs, TopologyRandom& random) {
    return Individual {
      input,
      random,
      TopologyConfiguration::Cross(input, lhs.m_configuration, rhs.m_configuration, random)
    };
  }

  static Individual Mutate(const TopologyInput& input, double probability, const Individual& individual, TopologyRandom& random) {
    return Individual {
      input,
      random,
      TopologyConfiguration::Mutate(input, probability, individual.m_configuration, random)
    };
  }

  const TopologyConfiguration& GetConfiguration() const {
    return m_configuration;
  }

  double GetFitness() const {
    return m_fitness;
  }

  friend std::ostream& operator<<(std::ostream& os, const Individual& obj) {
    os << obj.GetConfiguration();
    os << "Fitness:\n  " << obj.GetFitness() << '\n';
    os << "Port penalty:\n  " << CalculatePortPenalty(obj.m_input, obj.m_configuration) << '\n';
    os << "Difference:\n  " << CalculateTrafficDifference(obj.m_input, obj.m_configuration) << '\n';
    os << "Traffic:\n  " << CalculateTraffic(obj.m_input, obj.m_configuration) << '\n';
    return os;
  }

  static size_t CalculateTraffic(const TopologyInput& input, const TopologyConfiguration& conf) {
    const auto& loadData = conf.channelLoadMatrix.GetData();
    return std::accumulate(loadData.begin(), loadData.end(), static_cast<size_t>(0), std::plus());
  }

  static size_t CalculateTrafficDifference(const TopologyInput& input, const TopologyConfiguration& conf) {
    const auto& loadData = conf.channelLoadMatrix;
    const auto& bandwidthData = input.bandwidthMatrix;
    // Calculate sum|Ti-Bi|
    size_t accumulated = 0;
    for (size_t row = 0; row < input.routers; ++row) {
      for (size_t col = row + 1; col < input.routers; ++col) {
        // Avoid ULL type casting
        size_t traffic = loadData.At(row, col) + loadData.At(col, row);
        size_t bandwidth = bandwidthData.At(row, col);
        accumulated += std::max(traffic, bandwidth) - std::min(traffic, bandwidth);
      }
    }

    return accumulated;
  }

  static size_t CalculatePortPenalty(const TopologyInput& input, const TopologyConfiguration& conf) {
    size_t overhead = 0;
    for (size_t i = 0; i < input.routers; ++i) {
      size_t hosts = conf.subnetworkTable[i].size();
      size_t ports = input.portsCount[i];
      if (hosts > ports) {
        overhead += hosts - ports;
      }
    }

    return overhead;
  }

  static double CalculateFitness(const TopologyInput& input, const TopologyConfiguration& conf) {
    size_t trafficDifference = CalculateTrafficDifference(input, conf);
    return 1.0 / (trafficDifference + trafficDifference * CalculatePortPenalty(input, conf));
    return 1.0 / CalculateTraffic(input, conf);
  }

  friend void swap(Individual& lhs, Individual& rhs) noexcept {
    using std::swap;
    swap(lhs.m_random, rhs.m_random);
    swap(lhs.m_configuration, rhs.m_configuration);
    swap(lhs.m_fitness, rhs.m_fitness);
  }

private:
  const TopologyInput& m_input;
  TopologyRandom& m_random;
  TopologyConfiguration m_configuration;
  double m_fitness;
};
