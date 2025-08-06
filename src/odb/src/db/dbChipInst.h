// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2019-2025, The OpenROAD Authors

// Generator Code Begin Header
#pragma once

#include <string>

#include "dbCore.h"
#include "odb/dbTypes.h"
#include "odb/geom.h"
#include "odb/odb.h"

namespace odb {
class dbIStream;
class dbOStream;
class _dbDatabase;
class _dbChipRegionInst;
class _dbChip;

class _dbChipInst : public _dbObject
{
 public:
  _dbChipInst(_dbDatabase*);

  ~_dbChipInst();

  bool operator==(const _dbChipInst& rhs) const;
  bool operator!=(const _dbChipInst& rhs) const { return !operator==(rhs); }
  bool operator<(const _dbChipInst& rhs) const;
  dbObjectTable* getObjectTable(dbObjectType type);
  void collectMemInfo(MemInfo& info);

  std::string name_;
  Point3D loc_;
  dbOrientType::Value orient_;
  dbId<_dbChip> master_chip_;
  dbTable<_dbChipRegionInst>* chip_region_inst_tbl_;
};
dbIStream& operator>>(dbIStream& stream, _dbChipInst& obj);
dbOStream& operator<<(dbOStream& stream, const _dbChipInst& obj);
}  // namespace odb
// Generator Code End Header