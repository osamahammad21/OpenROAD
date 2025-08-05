// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2019-2025, The OpenROAD Authors

// Generator Code Begin Cpp
#include "dbChipInst.h"

#include <string>

#include "dbDatabase.h"
#include "dbTable.h"
#include "dbTable.hpp"
#include "odb/db.h"

namespace odb {
template class dbTable<_dbChipInst>;

bool _dbChipInst::operator==(const _dbChipInst& rhs) const
{
  if (name_ != rhs.name_) {
    return false;
  }
  if (loc_ != rhs.loc_) {
    return false;
  }
  if (orient_ != rhs.orient_) {
    return false;
  }

  return true;
}

bool _dbChipInst::operator<(const _dbChipInst& rhs) const
{
  if (name_ >= rhs.name_) {
    return false;
  }
  if (loc_ >= rhs.loc_) {
    return false;
  }
  if (orient_ >= rhs.orient_) {
    return false;
  }

  return true;
}

_dbChipInst::_dbChipInst(_dbDatabase* db)
{
  orient_ = 0;
}

dbIStream& operator>>(dbIStream& stream, _dbChipInst& obj)
{
  stream >> obj.name_;
  stream >> obj.loc_;
  stream >> obj.orient_;
  return stream;
}

dbOStream& operator<<(dbOStream& stream, const _dbChipInst& obj)
{
  stream << obj.name_;
  stream << obj.loc_;
  stream << obj.orient_;
  return stream;
}

void _dbChipInst::collectMemInfo(MemInfo& info)
{
  info.cnt++;
  info.size += sizeof(*this);
}

////////////////////////////////////////////////////////////////////
//
// dbChipInst - Methods
//
////////////////////////////////////////////////////////////////////

void dbChipInst::setName(const std::string& name)
{
  _dbChipInst* obj = (_dbChipInst*) this;

  obj->name_ = name;
}

std::string dbChipInst::getName() const
{
  _dbChipInst* obj = (_dbChipInst*) this;
  return obj->name_;
}

void dbChipInst::setLoc(Point3D loc)
{
  _dbChipInst* obj = (_dbChipInst*) this;

  obj->loc_ = loc;
}

Point3D dbChipInst::getLoc() const
{
  _dbChipInst* obj = (_dbChipInst*) this;
  return obj->loc_;
}

}  // namespace odb
// Generator Code End Cpp