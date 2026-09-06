// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2019-2025, The OpenROAD Authors

// Generator Code Begin Header
#pragma once

#include <cstdint>

#include "dbCore.h"
#include "dbVector.h"
#include "odb/db.h"

namespace odb {
class dbIStream;
class dbOStream;
class _dbDatabase;

struct dbTechLayerSpacingTableDirSpanLengthRuleFlags
{
  bool wrong_direction : 1;
  bool same_mask : 1;
  bool except_eol : 1;
  uint32_t spare_bits : 29;
};

class _dbTechLayerSpacingTableDirSpanLengthRule : public _dbObject
{
 public:
  _dbTechLayerSpacingTableDirSpanLengthRule(_dbDatabase*);

  bool operator==(const _dbTechLayerSpacingTableDirSpanLengthRule& rhs) const;
  bool operator!=(const _dbTechLayerSpacingTableDirSpanLengthRule& rhs) const
  {
    return !operator==(rhs);
  }
  bool operator<(const _dbTechLayerSpacingTableDirSpanLengthRule& rhs) const;
  void collectMemInfo(MemInfo& info);
  // User Code Begin Methods

  // Index of the last row whose span length is strictly less than
  // span_length, or -1 when no row qualifies (the rule does not apply to an
  // object of that span length).
  int getSpanLengthIdx(int span_length) const;

  // Index of the right-most column whose prl is strictly less than prl, or -1
  // when prl is at or below the first column (the rule does not apply).
  int getPrlIdx(int prl) const;
  // User Code End Methods

  dbTechLayerSpacingTableDirSpanLengthRuleFlags flags_;
  int eol_width_;
  dbVector<int> prl_tbl_;
  dbVector<int> span_length_tbl_;
  dbVector<dbVector<int>> spacing_tbl_;
  dbVector<int> exact_spacing_tbl_;
  dbVector<dbTechLayerSpacingTableDirSpanLengthRule::ExactSpanLengthSpacing>
      exact_span_length_spacing_tbl_;
};
dbIStream& operator>>(dbIStream& stream,
                      _dbTechLayerSpacingTableDirSpanLengthRule& obj);
dbOStream& operator<<(dbOStream& stream,
                      const _dbTechLayerSpacingTableDirSpanLengthRule& obj);
}  // namespace odb
// Generator Code End Header