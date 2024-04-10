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

  TopologyConfiguration configuration = TopologyConfiguration::CreateRandom(input, random);
  std::cout << configuration;

  Console::GetInstance()->Pause();
  return 0;
}
