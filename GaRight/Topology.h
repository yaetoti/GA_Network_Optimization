#pragma once

#include "Matrix.h"
#include "TopologyGenerator.h"

#include <numeric>
#include <random>
#include <set>

struct TopologyRandom final {
  std::mt19937_64 rng;
  std::uniform_real_distribution<double> dist;
};

/// Pre-generated topology data.
struct TopologyInput final {
  /// Hosts count.
  size_t hosts;
  /// Routers count.
  size_t routers;
  /// Table of routers' ports count.
  std::vector<size_t> portsCount;
  /// Matrix of single-sided traffic between hosts.
  Matrix<size_t> trafficMatrix;
  /// Symmetrical matrix of bandwidth of channels between routers.
  SymmetricalMatrix<size_t> bandwidthMatrix;

  friend std::ostream& operator<<(std::ostream& os, const TopologyInput& input) {
    os << "Ports: " << std::accumulate(input.portsCount.begin(), input.portsCount.end(), static_cast<size_t>(0), std::plus()) << '\n';
    for (size_t i = 0; i < input.portsCount.size(); ++i) {
      os << "  [" << i << "]: " << input.portsCount[i] << "\n";
    }
    os << "Traffic:\n";
    os << input.trafficMatrix;
    os << "Bandwidth:\n";
    os << input.bandwidthMatrix;

    return os;
  }
};

/// Topology configuration. (Chromosome)
struct TopologyConfiguration final {
  /// Table of default gateway for each host.
  std::vector<size_t> membershipTable;
  /// Table of hosts of each router. (Inverse of membershipTable)
  std::vector<std::set<size_t>> subnetworkTable;
  /// Table of router types.
  std::vector<RouterType> routerTypeTable;
  /// Symmetrical matrix of two-sided channel load.
  SymmetricalMatrix<size_t> channelLoadMatrix;

  friend std::ostream& operator<<(std::ostream& os, const TopologyConfiguration& conf) {
    os << "Membership table:\n";
    for (size_t i = 0; i < conf.membershipTable.size(); ++i) {
      os << "  [" << i << "]: " << conf.membershipTable[i] << "\n";
    }

    os << "Subnetwork table:\n";
    for (size_t i = 0; i < conf.subnetworkTable.size(); ++i) {
      os << "  [" << i << "]: ";
      std::ranges::copy(conf.subnetworkTable[i], std::ostream_iterator<size_t>(os, " "));
      os << '\n';
    }

    os << "Router type table:\n  ";
    for (size_t i = 0; i < conf.routerTypeTable.size(); ++i) {
      os << static_cast<size_t>(conf.routerTypeTable[i]) << " ";
    }
    os << '\n';

    os << "LoadMatrix:\n";
    os << conf.channelLoadMatrix;

    return os;
  }

  static TopologyConfiguration CreateRandom(const TopologyInput& input, TopologyRandom& random) {
    auto membershipTable = TopologyGenerator::CreateMembershipTable(input.hosts, input.routers, random.rng);
    auto subnetworkTable = TopologyGenerator::CreateSubnetworkTable(input.hosts, input.routers, membershipTable);
    auto routerTypeTable = TopologyGenerator::CreateRouterTypeTable(input.routers, random.rng);
    auto channelLoadMatrix = TopologyGenerator::CreateLoadMatrix(input.hosts, input.routers, { input.trafficMatrix, subnetworkTable, routerTypeTable });

    return TopologyConfiguration {
      std::move(membershipTable),
      std::move(subnetworkTable),
      std::move(routerTypeTable),
      std::move(channelLoadMatrix)
    };
  }

  static TopologyConfiguration Cross(const TopologyInput& input, const TopologyConfiguration& lhs, const TopologyConfiguration& rhs, TopologyRandom& random) {
    // Cross membership table (Uniform crossover)
    std::vector<size_t> membershipTable;
    membershipTable.reserve(input.hosts);

    for (size_t i = 0; i < input.hosts; ++i) {
      if (random.dist(random.rng) > 0.5) {
        membershipTable.emplace_back(lhs.membershipTable.at(i));
      }
      else {
        membershipTable.emplace_back(rhs.membershipTable.at(i));
      }
    }

    // Cross router type table (Uniform crossover)
    std::vector<RouterType> routerTypeTable;
    routerTypeTable.reserve(input.routers);

    for (size_t i = 0; i < input.routers; ++i) {
      if (random.dist(random.rng) > 0.5) {
        routerTypeTable.emplace_back(lhs.routerTypeTable.at(i));
      }
      else {
        routerTypeTable.emplace_back(rhs.routerTypeTable.at(i));
      }
    }

    // Create new configuration
    auto subnetworkTable = TopologyGenerator::CreateSubnetworkTable(input.hosts, input.routers, membershipTable);
    auto loadMatrix = TopologyGenerator::CreateLoadMatrix(input.hosts, input.routers, {
        input.trafficMatrix,
        subnetworkTable,
        routerTypeTable
      });

    return TopologyConfiguration {
      std::move(membershipTable),
      std::move(subnetworkTable),
      std::move(routerTypeTable),
      std::move(loadMatrix)
    };
  }

  static TopologyConfiguration Mutate(const TopologyInput& input, double probability, const TopologyConfiguration& conf, TopologyRandom& random) {
    // Mutate membership table
    std::vector<size_t> membershipTable = conf.membershipTable;

    for (size_t i = 0; i < input.hosts; ++i) {
      if (random.dist(random.rng) <= probability) {
        membershipTable[i] = random.rng() % input.routers;
      }
    }

    // Mutate router type table
    std::vector<RouterType> routerTypeTable = conf.routerTypeTable;

    for (size_t i = 0; i < input.routers; ++i) {
      if (random.dist(random.rng) <= probability) {
        routerTypeTable[i] = static_cast<RouterType>(random.rng() % static_cast<size_t>(RouterType::COUNT));
      }
    }

    // Create new configuration
    auto subnetworkTable = TopologyGenerator::CreateSubnetworkTable(input.hosts, input.routers, membershipTable);
    auto loadMatrix = TopologyGenerator::CreateLoadMatrix(input.hosts, input.routers, {
        input.trafficMatrix,
        subnetworkTable,
        routerTypeTable
      });

    return TopologyConfiguration {
      std::move(membershipTable),
      std::move(subnetworkTable),
      std::move(routerTypeTable),
      std::move(loadMatrix)
    };
  }
};
