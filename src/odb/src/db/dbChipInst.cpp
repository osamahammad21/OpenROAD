// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2019-2025, The OpenROAD Authors

// Generator Code Begin Cpp
#include "dbChipInst.h"

#include <string>

#include "dbChipRegionInst.h"
#include "dbDatabase.h"
#include "dbTable.h"
#include "dbTable.hpp"
#include "odb/db.h"
#include "odb/dbSet.h"
// User Code Begin Includes
#include "dbChip.h"
#include "dbChipRegion.h"
#include "dbChipRegionInst.h"
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
  if (*chip_region_inst_tbl_ != *rhs.chip_region_inst_tbl_) {
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
  chip_region_inst_tbl_ = new dbTable<_dbChipRegionInst>(
      db,
      this,
      (GetObjTbl_t) &_dbChipInst::getObjectTable,
      dbChipRegionInstObj);
}

dbIStream& operator>>(dbIStream& stream, _dbChipInst& obj)
{
  stream >> obj.name_;
  stream >> obj.loc_;
  stream >> obj.master_chip_;
  stream >> *obj.chip_region_inst_tbl_;
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
  stream << *obj.chip_region_inst_tbl_;
  // User Code Begin <<
  uint8_t orient = static_cast<uint8_t>(obj.orient_);
  stream << orient;
  // User Code End <<
  return stream;
}

dbObjectTable* _dbChipInst::getObjectTable(dbObjectType type)
{
  switch (type) {
    case dbChipRegionInstObj:
      return chip_region_inst_tbl_;
    default:
      break;
  }
  return getTable()->getObjectTable(type);
}
void _dbChipInst::collectMemInfo(MemInfo& info)
{
  info.cnt++;
  info.size += sizeof(*this);

  chip_region_inst_tbl_->collectMemInfo(
      info.children_["chip_region_inst_tbl_"]);
}

_dbChipInst::~_dbChipInst()
{
  delete chip_region_inst_tbl_;
}

////////////////////////////////////////////////////////////////////
//
// dbChipInst - Methods
//
////////////////////////////////////////////////////////////////////

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

dbSet<dbChipRegionInst> dbChipInst::getChipRegionInsts() const
{
  _dbChipInst* obj = (_dbChipInst*) this;
  return dbSet<dbChipRegionInst>(obj, obj->chip_region_inst_tbl_);
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
  _dbChipInst* _inst = _parent->chip_inst_tbl_->create();
  _inst->name_ = name;
  _inst->master_chip_ = _master->getOID();
  for (auto region : master->getChipRegions()) {
    _dbChipRegionInst* region_inst = _inst->chip_region_inst_tbl_->create();
    region_inst->region_ = region->getImpl()->getOID();
  }
  return (dbChipInst*) _inst;
}

void dbChipInst::destroy(dbChipInst* chip_inst)
{
  _dbChipInst* _chip_inst = (_dbChipInst*) chip_inst;
  _dbChip* _chip = (_dbChip*) chip_inst->getParentChip();
  for (auto region_inst : chip_inst->getChipRegionInsts()) {
    _chip_inst->chip_region_inst_tbl_->destroy(
        (_dbChipRegionInst*) region_inst);
  }
  _chip->chip_inst_tbl_->destroy(_chip_inst);
}
// User Code End dbChipInstPublicMethods
}  // namespace odb
// Generator Code End Cpp