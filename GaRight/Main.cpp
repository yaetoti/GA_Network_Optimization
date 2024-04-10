#include "PortDistributor.h"
#include "Topology.h"
#include "TopologyInputGenerator.h"

#include <cassert>
#include <ostream>
#include <Windows.h>
#include <ConsoleLib/Console.h>

double CalculateFitness(const TopologyInput& input, const TopologyConfiguration& configuration) {
  return 0.0;
}

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

private:
  const TopologyInput& m_input;
  TopologyRandom& m_random;
  TopologyConfiguration m_configuration;
  double m_fitness;
};

int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int) {
  Console::GetInstance()->RedirectStdHandles();

  // I
  const size_t hostsCount = 3;
  const size_t routersCount = 3;
  const size_t minPorts = 1;

  const double minOffset = PortDistributor::MinRandomOffset(routersCount, hostsCount, minPorts);
  std::cout << minOffset << '\n';

  // I
  TopologyRandom random {
    std::mt19937_64(std::chrono::steady_clock::now().time_since_epoch().count()),
    std::uniform_real_distribution()
  };

  TopologyInput input {
    hostsCount,
    routersCount,
    PortDistributor::RandomDistribution(routersCount, hostsCount, minOffset, random.rng, random.dist),
    TopologyInputGenerator::CreateTrafficMatrix(hostsCount, { 0.5, 4500, 500 }, random.rng, random.dist),
    TopologyInputGenerator::CreateBandwidthMatrix(routersCount, { 20000, 3000 }, random.rng)
  };
  std::cout << input << '\n';

  TopologyConfiguration conf1 = TopologyConfiguration::CreateRandom(input, random);
  TopologyConfiguration conf2 = TopologyConfiguration::CreateRandom(input, random);
  std::cout << conf1 << '\n';
  std::cout << conf2 << '\n';

  TopologyConfiguration crossed = TopologyConfiguration::Cross(input, conf1, conf2, random);
  std::cout << "Crossed:\n";
  std::cout << '\n' << crossed << '\n';

  TopologyConfiguration mutated = TopologyConfiguration::Mutate(input, 0.1, crossed, random);
  std::cout << "Mutated:\n";
  std::cout << '\n' << mutated << '\n';

  Console::GetInstance()->Pause();
  return 0;
}
