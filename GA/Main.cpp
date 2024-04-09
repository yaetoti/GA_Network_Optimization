#include <algorithm>
#include <iostream>
#include <numeric>
#include <random>
#include <Windows.h>
#include <ConsoleLib/Console.h>

float Function(float x) {
  x += 3; // Shift 3 left
  return x * x * x + 3 * x * x - 2;
}

float Derivative(float x) {
  // zeros: -5 and -3
  x += 3; // Shift 3 left
  return 3 * x * x + 6 * x;
}

float CalculateFitness(float x) {
  return std::abs(1 / Derivative(x));
}

uint32_t FloatToInt(float v) {
  return *reinterpret_cast<const uint32_t*>(&v);
}

float IntToFloat(uint32_t v) {
  return *reinterpret_cast<const float*>(&v);
}

struct Individual {
  Individual()
    : m_x { 0 }
    , m_fitness { CalculateFitness(0) } {
  }

  explicit Individual(float x)
    : m_x { x }
    , m_fitness { CalculateFitness(x) } {
  }

  float GetX() const {
    return m_x;
  }

  float GetFitness() const {
    return m_fitness;
  }

  friend std::ostream& operator<<(std::ostream& os, const Individual& obj) {
    return os << "{ x: " << obj.m_x << ", fitness: " << obj.m_fitness << " }";
  }

  friend std::wostream& operator<<(std::wostream& os, const Individual& obj) {
    return os << L"{ x: " << obj.m_x << L", fitness: " << obj.m_fitness << L" }";
  }

  static Individual Cross(const Individual& v1, const Individual& v2, std::random_device& device, std::uniform_real_distribution<float>& gen) {
    uint32_t x1 = FloatToInt(v1.GetX());
    uint32_t x2 = FloatToInt(v2.GetX());
    uint32_t result = 0;

    // Uniform crossover
    for (size_t i = 0; i < 8 * sizeof(uint32_t); ++i) {
      // Gen i-th bit
      uint32_t bit1 = (x1 >> i) & 1;
      uint32_t bit2 = (x2 >> i) & 1;
      // Shuffle
      if (gen(device) > 0.5f) {
        result |= bit1 << i;
      }
      else {
        result |= bit2 << i;
      }
    }

    return Individual { IntToFloat(result) };
  }

  static Individual Mutate(const Individual& v, float probability, std::random_device& device, std::uniform_real_distribution<float>& dist) {
    uint32_t x1 = FloatToInt(v.GetX());

    for (size_t i = 0; i < 8 * sizeof(uint32_t); ++i) {
      if (dist(device) <= probability) {
        x1 ^= 1 << i;
      }
    }

    return Individual { IntToFloat(x1) };
  }

private:
  float m_x;
  float m_fitness;
};

struct GreaterFitnessComparator {
  bool operator()(const Individual& lhs, const Individual& rhs) const {
    return lhs.GetFitness() > rhs.GetFitness();
  }
};

std::vector<float> CalculateCumulativeProbabilities(const std::vector<Individual>& population) {
  float fitnessSum = std::accumulate(population.begin(), population.end(), 0.0f, [](float old, const Individual& v) {
    return old + v.GetFitness();
  });

  std::vector<float> result;
  result.reserve(population.size());
  float accumulated = 0.0f;
  for (const auto& v : population) {
    accumulated += v.GetFitness() / fitnessSum;
    result.emplace_back(accumulated);
  }

  return result;
}

std::vector<Individual> RouletteSelect(const std::vector<Individual>& population, std::random_device& device, std::uniform_real_distribution<float>& dist) {
  std::vector<float> probabilities = CalculateCumulativeProbabilities(population);

  std::vector<Individual> result;
  result.reserve(population.size());
  for (size_t i = 0; i < population.size(); ++i) {
    float rand = dist(device);
    for (size_t j = 0; j < probabilities.size(); ++j) {
      if (rand <= probabilities[j]) {
        result.emplace_back(population[j]);
        break;
      }
    }
  }

  return result;
}

std::vector<Individual> DoSelection(std::vector<Individual> pool, float probability, std::random_device& device, std::mt19937_64& generator, std::uniform_real_distribution<float>& dist) {
  std::vector<Individual> result;
  result.reserve(pool.size());
  std::ranges::shuffle(pool, generator);

  for (size_t i = 0; i < pool.size(); i += 2) {
    Individual i1 = pool[i];
    Individual i2 = pool[i + 1];
    result.emplace_back(Individual::Mutate(
      Individual::Cross(i1, i2, device, dist),
      probability, device, dist));
    result.emplace_back(Individual::Mutate(
      Individual::Cross(i1, i2, device, dist),
      probability, device, dist));
  }

  return result;
}

// Task: Find x coordinate for any extrema of the function y = (x + 3)^3 + 3(x + 3)^2 - 2.
int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int) {
  Console::GetInstance()->RedirectStdHandles();

  std::random_device device;
  std::mt19937_64 generator(device());
  std::uniform_real_distribution<float> distribution;
  const size_t populationSize = 10;
  size_t iteration = 0;

  // Initialize population
  std::vector<Individual> population;
  population.reserve(populationSize);
  for (size_t i = 0; i < populationSize; ++i) {
    population.emplace_back((distribution(device) - 0.5f) * 200.0f);
  }
  std::ranges::sort(population, GreaterFitnessComparator());
  std::cout << '[' << iteration << "]: " << population[0] << '\n';

  do {
    // Roulette select
    std::vector<Individual> pool = RouletteSelect(population, device, distribution);
    population = DoSelection(pool, 0.01f, device, generator, distribution);
    std::ranges::sort(population, GreaterFitnessComparator());

    ++iteration;
    std::cout << '[' << iteration << "]: " << population[0] << '\n';

    Console::GetInstance()->Pause();
  } while (true);

  Console::GetInstance()->Pause();
  return 0;
}
