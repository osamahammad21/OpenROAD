// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2019-2025, The OpenROAD Authors

// Generator Code Begin Cpp
#include "dbChipRegionInst.h"

#include "dbChipRegion.h"
#include "dbDatabase.h"
#include "dbTable.h"
#include "dbTable.hpp"
#include "odb/db.h"
// User Code Begin Includes
#include "dbChip.h"
#include "dbChipInst.h"
// User Code End Includes
namespace odb {
template class dbTable<_dbChipRegionInst>;

bool _dbChipRegionInst::operator==(const _dbChipRegionInst& rhs) const
{
  if (region_ != rhs.region_) {
    return false;
  }

  return true;
}

bool _dbChipRegionInst::operator<(const _dbChipRegionInst& rhs) const
{
  if (region_ >= rhs.region_) {
    return false;
  }

  return true;
}

_dbChipRegionInst::_dbChipRegionInst(_dbDatabase* db)
{
}

dbIStream& operator>>(dbIStream& stream, _dbChipRegionInst& obj)
{
  stream >> obj.region_;
  return stream;
}

dbOStream& operator<<(dbOStream& stream, const _dbChipRegionInst& obj)
{
  stream << obj.region_;
  return stream;
}

void _dbChipRegionInst::collectMemInfo(MemInfo& info)
{
  info.cnt++;
  info.size += sizeof(*this);
}

////////////////////////////////////////////////////////////////////
//
// dbChipRegionInst - Methods
//
////////////////////////////////////////////////////////////////////

// User Code Begin dbChipRegionInstPublicMethods
dbChipInst* dbChipRegionInst::getChipInst() const
{
  return (dbChipInst*) getImpl()->getOwner();
}

dbChipRegion* dbChipRegionInst::getChipRegion() const
{
  _dbChipRegionInst* obj = (_dbChipRegionInst*) this;
  _dbChip* _chip = (_dbChip*) getChipInst()->getMasterChip();
  return (dbChipRegion*) _chip->chip_region_tbl_->getPtr(obj->region_);
}
// User Code End dbChipRegionInstPublicMethods
}  // namespace odb
// Generator Code End Cpp