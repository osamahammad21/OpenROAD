// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2019-2025, The OpenROAD Authors

// Generator Code Begin Cpp
#include "dbChipConn.h"

#include <string>

#include "dbChipRegionInst.h"
#include "dbDatabase.h"
#include "dbTable.h"
#include "dbTable.hpp"
#include "odb/db.h"
// User Code Begin Includes
#include "dbChip.h"
#include "dbChipRegionInst.h"
// User Code End Includes
namespace odb {
template class dbTable<_dbChipConn>;

bool _dbChipConn::operator==(const _dbChipConn& rhs) const
{
  if (name_ != rhs.name_) {
    return false;
  }
  if (thickness_ != rhs.thickness_) {
    return false;
  }
  if (top_region_ != rhs.top_region_) {
    return false;
  }
  if (bottom_region_ != rhs.bottom_region_) {
    return false;
  }

  // User Code Begin ==
  // User Code End ==
  return true;
}

bool _dbChipConn::operator<(const _dbChipConn& rhs) const
{
  if (name_ >= rhs.name_) {
    return false;
  }
  if (thickness_ >= rhs.thickness_) {
    return false;
  }
  if (top_region_ >= rhs.top_region_) {
    return false;
  }
  if (bottom_region_ >= rhs.bottom_region_) {
    return false;
  }

  // User Code Begin <
  // User Code End <
  return true;
}

_dbChipConn::_dbChipConn(_dbDatabase* db)
{
  // User Code Begin Constructor
  // User Code End Constructor
}

dbIStream& operator>>(dbIStream& stream, _dbChipConn& obj)
{
  stream >> obj.name_;
  stream >> obj.thickness_;
  stream >> obj.top_region_;
  stream >> obj.bottom_region_;
  // User Code Begin >>
  // User Code End >>
  return stream;
}

dbOStream& operator<<(dbOStream& stream, const _dbChipConn& obj)
{
  stream << obj.name_;
  stream << obj.thickness_;
  stream << obj.top_region_;
  stream << obj.bottom_region_;
  // User Code Begin <<
  // User Code End <<
  return stream;
}

void _dbChipConn::collectMemInfo(MemInfo& info)
{
  info.cnt++;
  info.size += sizeof(*this);

  // User Code Begin collectMemInfo
  // User Code End collectMemInfo
}

// User Code Begin PrivateMethods
// User Code End PrivateMethods

////////////////////////////////////////////////////////////////////
//
// dbChipConn - Methods
//
////////////////////////////////////////////////////////////////////

std::string dbChipConn::getName() const
{
  _dbChipConn* obj = (_dbChipConn*) this;
  return obj->name_;
}

void dbChipConn::setThickness(int thickness)
{
  _dbChipConn* obj = (_dbChipConn*) this;

  obj->thickness_ = thickness;
}

int dbChipConn::getThickness() const
{
  _dbChipConn* obj = (_dbChipConn*) this;
  return obj->thickness_;
}

// User Code Begin dbChipConnPublicMethods
dbChip* dbChipConn::getChip() const
{
  return (dbChip*) getImpl()->getOwner();
}

dbChipRegionInst* dbChipConn::getTopRegion() const
{
  _dbChipConn* obj = (_dbChipConn*) this;
  if (!obj->top_region_.isValid()) {
    return nullptr;
  }
  // TODO: Implement proper dbId resolution for dbChipRegionInst
  // Need to find the correct way to resolve dbId<_dbChipRegionInst> references
  return nullptr;
}

dbChipRegionInst* dbChipConn::getBottomRegion() const
{
  _dbChipConn* obj = (_dbChipConn*) this;
  if (!obj->bottom_region_.isValid()) {
    return nullptr;
  }
  // TODO: Implement proper dbId resolution for dbChipRegionInst
  // Need to find the correct way to resolve dbId<_dbChipRegionInst> references
  return nullptr;
}

dbChipConn* dbChipConn::create(dbChip* chip,
                               const std::string& name,
                               dbChipRegionInst* top_region,
                               dbChipRegionInst* bottom_region)
{
  _dbChip* _chip = (_dbChip*) chip;
  _dbChipConn* conn = _chip->chip_conn_tbl_->create();
  conn->name_ = name;
  if (top_region) {
    conn->top_region_ = top_region->getImpl()->getOID();
  }
  if (bottom_region) {
    conn->bottom_region_ = bottom_region->getImpl()->getOID();
  }
  return (dbChipConn*) conn;
}

void dbChipConn::destroy(dbChipConn* chip_conn)
{
  _dbChip* chip = (_dbChip*) chip_conn->getChip();
  chip->chip_conn_tbl_->destroy((_dbChipConn*) chip_conn);
}
// User Code End dbChipConnPublicMethods
}  // namespace odb
// Generator Code End Cpp