#include "Individual.h"
#include "PortDistributor.h"
#include "Topology.h"
#include "TopologyInputGenerator.h"

#include <cassert>
#include <ostream>
#include <Windows.h>
#include <ConsoleLib/Console.h>

struct GreaterFitnessComparator {
  bool operator()(const Individual& lhs, const Individual& rhs) const {
    return lhs.GetFitness() > rhs.GetFitness();
  }
};

std::vector<double> CalculateCumulativeProbabilities(const std::vector<Individual>& population) {
  double fitnessSum = std::accumulate(population.begin(), population.end(), 0.0f, [](double old, const Individual& v) {
    return old + v.GetFitness();
  });

  std::vector<double> result;
  result.reserve(population.size());
  double accumulated = 0.0f;

  for (const auto& v : population) {
    accumulated += v.GetFitness() / fitnessSum;
    result.emplace_back(accumulated);
  }

  return result;
}

std::vector<Individual> RouletteSelect(const std::vector<Individual>& population, TopologyRandom& random) {
  std::vector<double> probabilities = CalculateCumulativeProbabilities(population);
  std::vector<Individual> result;
  result.reserve(population.size());

  for (size_t i = 0; i < population.size(); ++i) {
    double rand = random.dist(random.rng);
    for (size_t j = 0; j < probabilities.size(); ++j) {
      if (rand <= probabilities[j]) {
        result.emplace_back(population[j]);
        break;
      }
    }
  }

  return result;
}

std::vector<Individual> DoSelection(const TopologyInput& input, std::vector<Individual> pool, double probability, TopologyRandom& random) {
  std::vector<Individual> result;
  result.reserve(pool.size());
  std::ranges::shuffle(pool, random.rng);

  for (size_t i = 0; i < pool.size(); i += 2) {
    Individual i1 = pool[i];
    Individual i2 = pool[i + 1];
    result.emplace_back(Individual::Mutate(
      input,
      probability,
      Individual::Cross(input, i1, i2, random),
      random));
    result.emplace_back(Individual::Mutate(
      input,
      probability,
      Individual::Cross(input, i1, i2, random),
      random));
  }

  return result;
}

int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int) {
  Console::GetInstance()->RedirectStdHandles();

  // Input pre-generation

  const size_t hostsCount = 12;
  const size_t routersCount = 3;
  const size_t minPorts = 2;

  const double minOffset = PortDistributor::MinRandomOffset(routersCount, hostsCount, minPorts);
  std::cout << minOffset << '\n';

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

  // Population initialization

  const size_t populationSize = 10;
  size_t iteration = 0;

  std::vector<Individual> population;
  population.reserve(populationSize);
  for (size_t i = 0; i < populationSize; ++i) {
    population.emplace_back(input, random);
  }
  std::ranges::sort(population, GreaterFitnessComparator());
  std::cout << '[' << iteration << "]:\n" << population[0] << '\n';

  // Selection

  while (population[0].GetFitness() != std::numeric_limits<double>::infinity()) {
    // Roulette select
    std::vector<Individual> pool = RouletteSelect(population, random);
    population = DoSelection(input, pool, 0.01f, random);
    std::ranges::sort(population, GreaterFitnessComparator());

    ++iteration;
    std::cout << '[' << iteration << "]:\n" << population[0] << '\n';

    Console::GetInstance()->Pause();
  }

  std::cout << "End of selection. Press any key to exit.\n";
  while (true) {
    Console::GetInstance()->Pause();
  }

  return 0;
}
