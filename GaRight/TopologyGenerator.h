#pragma once

#include "Matrix.h"

#include <random>
#include <set>

enum class RouterType {
  /// Routes traffic
  SWITCH,
  /// Broadcasts all received traffic
  HUB,
  COUNT
};

struct TopologyGenerator final {
  struct LoadOptions final {
    const Matrix<size_t>& trafficMatrix;
    const std::vector<std::set<size_t>>& subnetworkTable;
    const std::vector<RouterType>& routerTypeTable;
  };

  static std::vector<size_t> CreateMembershipTable(size_t hosts, size_t routers, std::mt19937_64& rng) {
    std::vector<size_t> result;
    result.reserve(hosts);
    for (size_t i = 0; i < hosts; ++i) {
      result.emplace_back(rng() % routers);
    }

    return result;
  }

  static std::vector<std::set<size_t>> CreateSubnetworkTable(size_t hosts, size_t routers, const std::vector<size_t>& membershipTable) {
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

  static std::vector<RouterType> CreateRouterTypeTable(size_t routers, std::mt19937_64& rng) {
    std::vector<RouterType> result;
    result.reserve(routers);

    for (size_t i = 0; i < routers; ++i) {
      result.emplace_back(static_cast<RouterType>(rng() % static_cast<size_t>(RouterType::COUNT)));
    }

    return result;
  }

  static SymmetricalMatrix<size_t> CreateLoadMatrix(size_t hosts, size_t routers, const LoadOptions& options) {
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
};
