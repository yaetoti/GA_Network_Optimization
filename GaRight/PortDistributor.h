#pragma once

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

/**
 * Contains methods for distributing ports over routers.
 */
struct PortDistributor final {
  /**
   * Calculates offset for 0..1 random to limit the minimum number of ports per router.
   * Returns infinity if routers == ports.
   * Returns negative number if routers > ports.
   */
  static double MinRandomOffset(int64_t routers, int64_t ports, int64_t min) {
    return static_cast<double>(min * (routers - 1)) / static_cast<double>(ports - min * routers);
  }

  /**
   * Calculates offset for 0..1 random to limit the maximum number of ports per router.
   * Returns infinity if routers == ports.
   * Returns negative number if routers > ports.
   */
  static double MaxRandomOffset(int64_t routers, int64_t ports, int64_t max) {
    return static_cast<double>(max - ports) / static_cast<double>(ports - max * routers);
  }

  /**
   * Calculates the minimum number of ports per router for offset.
   * Considers a situation where generated probability is 1 for every router except for one with probability of 0.
   */
  static double MinPortsCount(int64_t routers, int64_t ports, double offset) {
    return offset / ((routers - 1) * (1.0 + offset) + offset) * ports;
  }

  /**
   * Calculates the maximum number of ports per router for offset.
   * Considers a situation where generated probability is 0 for every router except for one with probability of 1.
   */
  static double MaxPortsCount(int64_t routers, int64_t ports, double offset) {
    return (1 + offset) / ((routers - 1) * offset + 1.0 + offset) * ports;
  }

  /**
   * Uniformly distributes ports over routers.
   * \param offset Offset for generated membership probabilities. Should be >= 0.
   */
  static std::vector<size_t> RandomDistribution(size_t routers, size_t ports, double offset, std::mt19937_64& rng, std::uniform_real_distribution<>& dist) {
    // Handle exceptional cases
    assert(routers <= ports);
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


  // Testing functionality


private:
  /**
   * Test method to ensure that the result matches the number of ports.
   * Considers a situation where generated probability is 1 for every router except for one with probability of 0.
   */
  static double SimulateMinPorts(size_t routers, size_t ports, double offset) {
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

  /**
   * Test method to ensure that the result matches the number of ports.
   * Considers a situation where generated probability is 0 for every router except for one with probability of 1.
   */
  static double SimulateMaxPorts(size_t routers, size_t ports, double offset) {
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

  static void Test() {
    std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<> dist;

    const size_t hostsCount = 3;
    const size_t routersCount = 3;
    const size_t minPorts = 1;
    const size_t maxPorts = 8;

    double minOffset = MinRandomOffset(routersCount, hostsCount, minPorts);
    double maxOffset = MaxRandomOffset(routersCount, hostsCount, maxPorts);

    double minPortsMin = MinPortsCount(routersCount, hostsCount, minOffset);
    double maxPortsMin = MaxPortsCount(routersCount, hostsCount, minOffset);
    double minPortsMax = MinPortsCount(routersCount, hostsCount, maxOffset);
    double maxPortsMax = MaxPortsCount(routersCount, hostsCount, maxOffset);

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

#if 1
    // Generate and print
    std::vector<size_t> portsCount = RandomDistribution(routersCount, hostsCount, maxOffset, rng, dist);
    std::cout << "PortsCount: " << std::accumulate(portsCount.begin(), portsCount.end(), 0, std::plus()) << '\n';
    for (size_t count : portsCount) {
      std::cout << count << '\n';
    }
#else
    // Generate until the selection contains the selected element
    std::vector<size_t> portsCount;
    do {
      portsCount = RandomDistribution(routersCount, hostsCount, minOffset, rng, dist);
    } while (std::ranges::find(portsCount, 1) == portsCount.end());

    std::cout << "PortsCount: " << std::accumulate(portsCount.begin(), portsCount.end(), 0, std::plus()) << '\n';
    for (size_t count : portsCount) {
      std::cout << count << '\n';
    }
#endif
  }
};
