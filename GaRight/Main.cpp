#include "PortDistributor.h"
#include "TopologyInputGenerator.h"

#include <cassert>
#include <ostream>
#include <set>
#include <Windows.h>
#include <ConsoleLib/Console.h>

enum class RouterType {
  /// Routes traffic
  SWITCH,
  /// Broadcasts all received traffic
  HUB,
  COUNT
};

struct LoadOptions final {
  const Matrix<size_t>& trafficMatrix;
  const std::vector<std::set<size_t>>& subnetworkTable;
  const std::vector<RouterType>& routerTypeTable;
};

std::vector<size_t> CreateMembershipTable(size_t hosts, size_t routers, std::mt19937_64& rng) {
  std::vector<size_t> result;
  result.reserve(hosts);
  for (size_t i = 0; i < hosts; ++i) {
    result.emplace_back(rng() % routers);
  }

  return result;
}

std::vector<std::set<size_t>> CreateSubnetworkTable(size_t hosts, size_t routers, const std::vector<size_t>& membershipTable) {
  std::vector<std::set<size_t>> result(routers);
  // Membership table -> LAN vector
  for (size_t i = 0; i < routers; ++i) {
    for (size_t j = 0; j < hosts; ++j) {
      if (membershipTable[j] == i) {
        result[i].emplace(j);
      }
    }
  }

  return result;
}

std::vector<RouterType> CreateRouterTypeTable(size_t routers, std::mt19937_64& rng) {
  std::vector<RouterType> result;
  result.reserve(routers);

  for (size_t i = 0; i < routers; ++i) {
    result.emplace_back(static_cast<RouterType>(rng() % static_cast<size_t>(RouterType::COUNT)));
  }

  return result;
}

SymmetricalMatrix<size_t> CreateLoadMatrix(size_t hosts, size_t routers, const LoadOptions& options) {
  SymmetricalMatrix<size_t> loadMatrix(routers);

  for (size_t router1 = 0; router1 < routers; ++router1) {
    const std::set<size_t>& set1 = options.subnetworkTable[router1];

    for (size_t router2 = 0; router2 < routers; ++router2) {
      if (router1 == router2) {
        continue;
      }

      if (options.routerTypeTable[router1] == RouterType::SWITCH) {
        // Switch routes traffic. Only outer traffic matters.
        const std::set<size_t>& set2 = options.subnetworkTable[router2];

        for (size_t host1 : set1) {
          for (size_t host2 : set2) {
            loadMatrix.Set(router1, router2, loadMatrix.At(router1, router2) + options.trafficMatrix.At(host1, host2));
          }
        }
      }
      else {
        // Hub broadcasts all traffic. (Simplified model. The right one requires creating spanning tree and routing - 4-7 days).
        for (size_t host1 : set1) {
          for (size_t host2 = 0; host2 < hosts; ++host2) {
            loadMatrix.Set(router1, router2, loadMatrix.At(router1, router2) + options.trafficMatrix.At(host1, host2));
          }
        }
      }
    }
  }

  return loadMatrix;
}

int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int) {
  Console::GetInstance()->RedirectStdHandles();

  std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
  std::uniform_real_distribution<> dist;

  const size_t hostsCount = 3;
  const size_t routersCount = 3;
  const size_t minPorts = 1;

  double minOffset = PortDistributor::MinRandomOffset(routersCount, hostsCount, minPorts);

  std::vector<size_t> portsCount;
  do {
    portsCount = PortDistributor::RandomDistribution(routersCount, hostsCount, minOffset, rng, dist);
  } while (std::ranges::find(portsCount, 1) == portsCount.end());

  std::cout << minOffset << '\n';
  std::cout << "Elements: " << std::accumulate(portsCount.begin(), portsCount.end(), 0, std::plus()) << '\n';
  for (size_t i = 0; i < portsCount.size(); ++i) {
    std::cout << "[" << i << "]: " << portsCount[i] << "\n";
  }

  // Matrices
  Matrix<size_t> trafficMatrix = TopologyInputGenerator::CreateTrafficMatrix(hostsCount, { 0.5, 4500, 500 }, rng, dist);
  std::cout << "\nTraffic:\n";
  std::cout << trafficMatrix;
  SymmetricalMatrix<size_t> bandwidthMatrix = TopologyInputGenerator::CreateBandwidthMatrix(routersCount, { 20000, 3000 }, rng);
  std::cout << "\nBandwidth:\n";
  std::cout << bandwidthMatrix;

  // Individual data
  std::vector<size_t> hostMembershipTable = CreateMembershipTable(hostsCount, routersCount, rng);
  std::cout << "\nMembership Table:\n";
  for (size_t i = 0; i < hostsCount; ++i) {
    std::cout << "[" << i << "]: " << hostMembershipTable[i] << "\n";
  }

  std::vector<std::set<size_t>> subnetworkTable = CreateSubnetworkTable(hostsCount, routersCount, hostMembershipTable);
  std::cout << "\nLAN Table:\n";
  for (size_t i = 0; i < routersCount; ++i) {
    std::cout << "[" << i << "]: ";
    std::ranges::copy(subnetworkTable[i], std::ostream_iterator<size_t>(std::cout, " "));
    std::cout << '\n';
  }

  std::vector<RouterType> routerTypeTable = CreateRouterTypeTable(routersCount, rng);
  std::cout << "\nRouter Type Table:\n";
  for (size_t i = 0; i < routersCount; ++i) {
    std::cout << static_cast<size_t>(routerTypeTable[i]) << " ";
  }
  std::cout << '\n';

  SymmetricalMatrix<size_t> channelLoadMatrix = CreateLoadMatrix(hostsCount, routersCount, { trafficMatrix, subnetworkTable, routerTypeTable });
  std::cout << "\nLoadMatrix:\n";
  std::cout << channelLoadMatrix;

  Console::GetInstance()->Pause();
  return 0;
}
