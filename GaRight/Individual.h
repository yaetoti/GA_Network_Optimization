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
    os << "Traffic:\n  " << CalculateTraffic(obj.m_input, obj.m_configuration) << '\n';
    return os;
  }

  static size_t CalculateTraffic(const TopologyInput& input, const TopologyConfiguration& conf) {
    const auto& loadData = conf.channelLoadMatrix.GetData();
    return std::accumulate(loadData.begin(), loadData.end(), static_cast<size_t>(0), std::plus());
  }

  static double CalculateFitness(const TopologyInput& input, const TopologyConfiguration& conf) {
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
