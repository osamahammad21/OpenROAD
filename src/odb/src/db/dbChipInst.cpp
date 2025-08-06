// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2019-2025, The OpenROAD Authors

// Generator Code Begin Cpp
#include "dbChipInst.h"

#include <string>

#include "dbDatabase.h"
#include "dbTable.h"
#include "dbTable.hpp"
#include "odb/db.h"

// User Code Begin Includes
#include "dbChip.h"
#include "odb/dbTransform.h"
#include "odb/dbTypes.h"
// User Code End Includes
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
  if (master_chip_ != rhs.master_chip_) {
    return false;
  }

  // User Code Begin ==
  if (orient_ != rhs.orient_) {
    return false;
  }
  // User Code End ==
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
  if (master_chip_ >= rhs.master_chip_) {
    return false;
  }

  // User Code Begin <
  if (orient_ >= rhs.orient_) {
    return false;
  }
  // User Code End <
  return true;
}

_dbChipInst::_dbChipInst(_dbDatabase* db)
{
}

dbIStream& operator>>(dbIStream& stream, _dbChipInst& obj)
{
  stream >> obj.name_;
  stream >> obj.loc_;
  stream >> obj.master_chip_;
  // User Code Begin >>
  uint8_t orient;
  stream >> orient;
  obj.orient_ = static_cast<dbOrientType::Value>(orient);
  // User Code End >>
  return stream;
}

dbOStream& operator<<(dbOStream& stream, const _dbChipInst& obj)
{
  stream << obj.name_;
  stream << obj.loc_;
  stream << obj.master_chip_;
  // User Code Begin <<
  uint8_t orient = static_cast<uint8_t>(obj.orient_);
  stream << orient;
  // User Code End <<
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

// User Code Begin dbChipInstPublicMethods
void dbChipInst::setOrient(dbOrientType orient)
{
  _dbChipInst* obj = (_dbChipInst*) this;
  obj->orient_ = orient.getValue();
}

dbOrientType dbChipInst::getOrient() const
{
  _dbChipInst* obj = (_dbChipInst*) this;
  return dbOrientType(obj->orient_);
}

dbTransform dbChipInst::getTransform() const
{
  _dbChipInst* obj = (_dbChipInst*) this;
  dbTransform transform;
  transform.setOrient(dbOrientType(obj->orient_));
  transform.setOffset(Point(obj->loc_.x(), obj->loc_.y()));
  return transform;
}

dbChip* dbChipInst::getParentChip() const
{
  return (dbChip*) getImpl()->getOwner();
}

dbChip* dbChipInst::getMasterChip() const
{
  // get database
  _dbChipInst* obj = (_dbChipInst*) this;
  _dbDatabase* db = obj->getDatabase();
  return (dbChip*) db->_chip_tbl->getPtr(obj->master_chip_);
}

dbChipInst* dbChipInst::create(dbChip* parent,
                               dbChip* master,
                               const std::string& name)
{
  _dbChip* _master = (_dbChip*) master;
  _dbChip* _parent = (_dbChip*) parent;
  _dbChipInst* inst = _parent->chip_inst_tbl_->create();
  inst->name_ = name;
  inst->master_chip_ = _master->getOID();
  return (dbChipInst*) inst;
}

void dbChipInst::destroy(dbChipInst* chip_inst)
{
  _dbChip* chip = (_dbChip*) chip_inst->getParentChip();
  chip->chip_inst_tbl_->destroy((_dbChipInst*) chip_inst);
}
// User Code End dbChipInstPublicMethods
}  // namespace odb
// Generator Code End Cpp