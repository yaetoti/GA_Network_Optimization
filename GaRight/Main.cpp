#include <cassert>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>
#include <Windows.h>
#include <ConsoleLib/Console.h>

std::vector<size_t> RandomPortDistribution(size_t routers, size_t ports, double offset, std::mt19937_64& rng, std::uniform_real_distribution<>& dist) {
  // Handle exceptional cases
  assert(routers > ports);
  if (routers == ports) {
    return std::vector<size_t>(routers, 1);
  }

  // Generate probability distribution
  std::vector<double> probabilities;
  probabilities.reserve(routers);
  for (size_t i = 0; i < routers; ++i) {
    // For 12H 3R gives 3 to 5 ports  
    probabilities.emplace_back(dist(rng) + offset);
  }
  double accumulated = std::accumulate(probabilities.begin(), probabilities.end(), 0.0, std::plus());

  // Calculate port count
  std::vector<size_t> result;
  result.reserve(routers);
  double error = 0.0;
  for (size_t i = 0; i < routers; ++i) {
    double portsCount = probabilities[i] / accumulated * ports;
    double roundedCount = std::round(portsCount + error);
    // Error accumulation
    error += portsCount - roundedCount;
    result.emplace_back(static_cast<size_t>(roundedCount));
  }

  return result;
}

double MinPortRandomOffset(int64_t routers, int64_t ports, int64_t min) {
  return static_cast<double>(min * (routers - 1)) / static_cast<double>(ports - min * routers);
}

double MaxPortRandomOffset(int64_t routers, int64_t ports, int64_t max) {
  return static_cast<double>(max - ports) / static_cast<double>(ports - max * routers);
}

double MinPortsCount(int64_t routers, int64_t ports, double offset) {
  return offset / ((routers - 1) * (1.0 + offset) + offset) * ports;
}

double MaxPortsCount(int64_t routers, int64_t ports, double offset) {
  return (1 + offset) / ((routers - 1) * offset + 1.0 + offset) * ports;
}

double SimulateMinPorts(size_t routers, size_t ports, double offset) {
  std::vector<double> probabilities;
  probabilities.reserve(routers);
  for (size_t i = 0; i < routers - 1; ++i) {
    probabilities.emplace_back(1.0 + offset);
  }
  probabilities.emplace_back(offset);

  double accumulated = std::accumulate(probabilities.begin(), probabilities.end(), 0.0, std::plus());
  double result = 0.0;
  double error = 0.0;
  for (size_t i = 0; i < routers; ++i) {
    double portsCount = probabilities[i] / accumulated * ports;
    double roundedCount = std::round(portsCount + error);
    error += portsCount - roundedCount;
    result += roundedCount;
  }

  return result;
}

double SimulateMaxPorts(size_t routers, size_t ports, double offset) {
  std::vector<double> probabilities;
  probabilities.reserve(routers);
  for (size_t i = 0; i < routers - 1; ++i) {
    probabilities.emplace_back(offset);
  }
  probabilities.emplace_back(1.0 + offset);

  double accumulated = std::accumulate(probabilities.begin(), probabilities.end(), 0.0, std::plus());
  double result = 0.0;
  double error = 0.0;
  for (size_t i = 0; i < routers; ++i) {
    double portsCount = probabilities[i] / accumulated * ports;
    double roundedCount = std::round(portsCount + error);
    error += portsCount - roundedCount;
    result += roundedCount;
  }

  return result;
}

int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int) {
  Console::GetInstance()->RedirectStdHandles();

  std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
  std::uniform_real_distribution<> dist;

  const size_t hostsCount = 3;
  const size_t routersCount = 3;
  const size_t minPorts = 1;
  const size_t maxPorts = 8;

  double minOffset = MinPortRandomOffset(routersCount, hostsCount, minPorts);
  double maxOffset = MaxPortRandomOffset(routersCount, hostsCount, maxPorts);
#if 1
  double minPortsMin = MinPortsCount(routersCount, hostsCount, minOffset);
  double maxPortsMin = MaxPortsCount(routersCount, hostsCount, minOffset);
  double minPortsMax = MinPortsCount(routersCount, hostsCount, maxOffset);
  double maxPortsMax = MaxPortsCount(routersCount, hostsCount, maxOffset);
#else
  double minPortsMin = MinPortsCount(routersCount, hostsCount, (minOffset + maxOffset) / 2.0);
  double maxPortsMin = MaxPortsCount(routersCount, hostsCount, (minOffset + maxOffset) / 2.0);
  double minPortsMax = MinPortsCount(routersCount, hostsCount, (minOffset + maxOffset) / 2.0);
  double maxPortsMax = MaxPortsCount(routersCount, hostsCount, (minOffset + maxOffset) / 2.0);
#endif

  double minPortsResultMin = SimulateMinPorts(routersCount, hostsCount, minOffset);
  double minPortsResultMax = SimulateMinPorts(routersCount, hostsCount, maxOffset);
  double maxPortsResultMin = SimulateMaxPorts(routersCount, hostsCount, minOffset);
  double maxPortsResultMax = SimulateMaxPorts(routersCount, hostsCount, maxOffset);

  std::cout << "Min: " << minOffset << '\n';
  std::cout << "Max: " << maxOffset << '\n';
  std::cout << minPortsMin << " to " << maxPortsMin << '\n';
  std::cout << minPortsMax << " to " << maxPortsMax << '\n';
  std::cout << "Actual: " << minPortsResultMin << " to " << minPortsResultMax << '\n';
  std::cout << "Actual: " << maxPortsResultMin << " to " << maxPortsResultMax << '\n';

#if 0
  std::vector<size_t> portsCount = RandomPortDistribution(routersCount, hostsCount, maxOffset, rng, dist);
  std::cout << "PortsCount: " << std::accumulate(portsCount.begin(), portsCount.end(), 0, std::plus()) << '\n';
  for (size_t count : portsCount) {
    std::cout << count << '\n';
  }
#else
  std::vector<size_t> portsCount;
  do {
    portsCount = RandomPortDistribution(routersCount, hostsCount, minOffset, rng, dist);
  } while (std::ranges::find(portsCount, 1) == portsCount.end());

  std::cout << "PortsCount: " << std::accumulate(portsCount.begin(), portsCount.end(), 0, std::plus()) << '\n';
  for (size_t count : portsCount) {
    std::cout << count << '\n';
  }
#endif


  Console::GetInstance()->Pause();
  return 0;
}
