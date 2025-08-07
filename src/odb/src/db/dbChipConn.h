// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2019-2025, The OpenROAD Authors

// Generator Code Begin Header
#pragma once

#include <string>

#include "dbCore.h"
#include "odb/dbId.h"
#include "odb/odb.h"
// User Code Begin Includes
// User Code End Includes

namespace odb {
class dbIStream;
class dbOStream;
class _dbDatabase;
class _dbChipRegionInst;
// User Code Begin Classes
// User Code End Classes

// User Code Begin Structs
// User Code End Structs

class _dbChipConn : public _dbObject
{
 public:
  // User Code Begin Enums
  // User Code End Enums

  _dbChipConn(_dbDatabase*);

  bool operator==(const _dbChipConn& rhs) const;
  bool operator!=(const _dbChipConn& rhs) const { return !operator==(rhs); }
  bool operator<(const _dbChipConn& rhs) const;
  void collectMemInfo(MemInfo& info);
  // User Code Begin Methods
  // User Code End Methods

  std::string name_;
  int thickness_;
  dbId<_dbChipRegionInst> top_region_;
  dbId<_dbChipRegionInst> bottom_region_;

  // User Code Begin Fields
  // User Code End Fields
};
dbIStream& operator>>(dbIStream& stream, _dbChipConn& obj);
dbOStream& operator<<(dbOStream& stream, const _dbChipConn& obj);
// User Code Begin General
// User Code End General
}  // namespace odb
// Generator Code End Header