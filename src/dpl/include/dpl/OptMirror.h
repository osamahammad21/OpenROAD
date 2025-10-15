// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025, The OpenROAD Authors

#pragma once

#include <cstdint>
#include <vector>

namespace utl {
class Logger;
}
namespace dpl {
class Node;
class Network;
class PlacementDRC;

using utl::Logger;

class OptimizeMirroring
{
 public:
  OptimizeMirroring(Logger* logger,
                    Network* network,
                    PlacementDRC* placement_drc);

  int run();

 private:
  int mirrorCandidates(std::vector<Node*>& mirror_candidates);
  std::vector<Node*> findMirrorCandidates();

  int64_t hpwl(Node* node);  // Sum of ITerm hpwl's.

  Logger* logger_ = nullptr;
  Network* network_ = nullptr;
  PlacementDRC* placement_drc_ = nullptr;

  // Net bounding box size on nets with more instance terminals
  // than this are ignored.
  static constexpr int mirror_max_iterm_count_ = 100;
};

}  // namespace dpl
