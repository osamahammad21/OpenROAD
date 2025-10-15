// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2025, The OpenROAD Authors

#include "dpl/OptMirror.h"

#include <cstdint>
#include <cstdlib>
#include <unordered_set>
#include <vector>

#include "PlacementDRC.h"
#include "infrastructure/network.h"
#include "odb/db.h"
#include "odb/dbTypes.h"
#include "utl/Logger.h"

namespace dpl {

using utl::DPL;

using odb::dbOrientType;

static dbOrientType orientMirrorY(const dbOrientType& orient);

OptimizeMirroring::OptimizeMirroring(Logger* logger,
                                     Network* network,
                                     PlacementDRC* placement_drc)
    : logger_(logger), network_(network), placement_drc_(placement_drc)
{
}

int OptimizeMirroring::run()
{
  auto mirror_candidates = findMirrorCandidates();
  // sort mirror candidates by hpwl in descending order
  std::sort(mirror_candidates.begin(),
            mirror_candidates.end(),
            [this](Node* a, Node* b) { return hpwl(a) > hpwl(b); });
  return mirrorCandidates(mirror_candidates);
}

std::vector<Node*> OptimizeMirroring::findMirrorCandidates()
{
  std::vector<Node*> mirror_candidates;
  std::unordered_set<Node*> existing;
  for (auto& edge : network_->getEdges()) {
    auto box = edge->getBBox();
    if (edge->getNumPins() >= mirror_max_iterm_count_) {
      continue;
    }
    for (auto& pin : edge->getPins()) {
      auto node = pin->getNode();
      auto inst = node->getDbInst();
      if (inst == nullptr) {
        continue;
      }
      if (inst->isFixed() || !inst->isCore()) {
        continue;
      }
      // if the pin is not on the edge of the box, skip it
      if (box.overlaps(pin->getLocation())) {
        continue;
      }
      if (existing.find(node) != existing.end()) {
        continue;
      }
      mirror_candidates.push_back(node);
      existing.insert(node);
      debugPrint(
          logger_, DPL, "opt_mirror", 1, "candidate {}", inst->getConstName());
    }
  }
  return mirror_candidates;
}

int OptimizeMirroring::mirrorCandidates(std::vector<Node*>& mirror_candidates)
{
  int mirror_count = 0;
  for (auto node : mirror_candidates) {
    // Use hpwl of all nets connected to the instance terms
    // before/after to determine incremental change to total hpwl.
    int64_t hpwl_before = hpwl(node);
    dbOrientType orient = node->getOrient();
    dbOrientType orient_my = orientMirrorY(orient);
    node->adjustCurrOrient(orient_my);
    int64_t hpwl_after = hpwl(node);
    bool valid = placement_drc_->checkDRC(node);
    if (!valid || hpwl_after > hpwl_before) {
      // Undo mirroring if hpwl is worse.
      node->adjustCurrOrient(orient);
    } else {
      debugPrint(logger_,
                 DPL,
                 "opt_mirror",
                 1,
                 "mirror {}",
                 node->getDbInst()->getConstName());
      mirror_count++;
    }
  }
  return mirror_count;
}

// apply mirror about Y axis to orient
static dbOrientType orientMirrorY(const dbOrientType& orient)
{
  switch (orient.getValue()) {
    case dbOrientType::R0:
      return dbOrientType::MY;
    case dbOrientType::MX:
      return dbOrientType::R180;
    case dbOrientType::MY:
      return dbOrientType::R0;
    case dbOrientType::R180:
      return dbOrientType::MX;
    case dbOrientType::R90:
      return dbOrientType::MXR90;
    case dbOrientType::MXR90:
      return dbOrientType::R90;
    case dbOrientType::R270:
      return dbOrientType::MYR90;
    case dbOrientType::MYR90:
      return dbOrientType::R270;
  }
  // make lame gcc happy
  std::abort();
  return dbOrientType::R0;
}

int64_t OptimizeMirroring::hpwl(Node* node)
{
  int64_t hpwl = 0;
  for (auto pin : node->getPins()) {
    if (pin->getEdge() == nullptr) {
      continue;
    }
    if (pin->getEdge()->getNumPins() >= mirror_max_iterm_count_) {
      continue;
    }
    hpwl += pin->getEdge()->hpwl();
  }
  return hpwl;
}

}  // namespace dpl
