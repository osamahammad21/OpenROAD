// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2019-2025, The OpenROAD Authors

// Generator Code Begin Cpp
#include "dbChipRegion.h"

#include <string>

#include "dbDatabase.h"
#include "dbTable.h"
#include "dbTable.hpp"
#include "dbTechLayer.h"
#include "odb/db.h"
// User Code Begin Includes
#include "dbChip.h"
#include "dbTech.h"
// User Code End Includes
namespace odb {
template class dbTable<_dbChipRegion>;

bool _dbChipRegion::operator==(const _dbChipRegion& rhs) const
{
  if (name_ != rhs.name_) {
    return false;
  }
  if (side_ != rhs.side_) {
    return false;
  }
  if (layer_ != rhs.layer_) {
    return false;
  }
  if (box_ != rhs.box_) {
    return false;
  }

  return true;
}

bool _dbChipRegion::operator<(const _dbChipRegion& rhs) const
{
  if (name_ >= rhs.name_) {
    return false;
  }
  if (side_ >= rhs.side_) {
    return false;
  }
  if (layer_ >= rhs.layer_) {
    return false;
  }
  if (box_ >= rhs.box_) {
    return false;
  }

  return true;
}

_dbChipRegion::_dbChipRegion(_dbDatabase* db)
{
}

dbIStream& operator>>(dbIStream& stream, _dbChipRegion& obj)
{
  stream >> obj.name_;
  stream >> obj.side_;
  stream >> obj.layer_;
  stream >> obj.box_;
  return stream;
}

dbOStream& operator<<(dbOStream& stream, const _dbChipRegion& obj)
{
  stream << obj.name_;
  stream << obj.side_;
  stream << obj.layer_;
  stream << obj.box_;
  return stream;
}

void _dbChipRegion::collectMemInfo(MemInfo& info)
{
  info.cnt++;
  info.size += sizeof(*this);
}

////////////////////////////////////////////////////////////////////
//
// dbChipRegion - Methods
//
////////////////////////////////////////////////////////////////////

std::string dbChipRegion::getName() const
{
  _dbChipRegion* obj = (_dbChipRegion*) this;
  return obj->name_;
}

void dbChipRegion::setBox(Rect box)
{
  _dbChipRegion* obj = (_dbChipRegion*) this;

  obj->box_ = box;
}

Rect dbChipRegion::getBox() const
{
  _dbChipRegion* obj = (_dbChipRegion*) this;
  return obj->box_;
}

// User Code Begin dbChipRegionPublicMethods
dbChip* dbChipRegion::getChip() const
{
  return (dbChip*) getImpl()->getOwner();
}

dbChipRegion::Side dbChipRegion::getSide() const
{
  _dbChipRegion* obj = (_dbChipRegion*) this;
  return dbChipRegion::Side(obj->side_);
}

dbTechLayer* dbChipRegion::getLayer() const
{
  _dbChipRegion* obj = (_dbChipRegion*) this;
  if (!obj->layer_.isValid()) {
    return nullptr;
  }
  auto chip = getChip();
  if (chip == nullptr) {
    return nullptr;
  }
  auto block = chip->getBlock();
  if (block == nullptr) {
    return nullptr;
  }
  auto tech = block->getTech();
  if (tech == nullptr) {
    return nullptr;
  }
  _dbTech* _tech = (_dbTech*) tech;
  return (dbTechLayer*) _tech->_layer_tbl->getPtr(obj->layer_);
}

void dbChipRegion::setLayer(dbTechLayer* layer)
{
  _dbChipRegion* obj = (_dbChipRegion*) this;
  if (layer == nullptr) {
    obj->layer_ = 0;
  } else {
    obj->layer_ = layer->getImpl()->getOID();
  }
}

dbChipRegion* dbChipRegion::create(dbChip* chip,
                                   const std::string& name,
                                   Side side)
{
  _dbChip* _chip = (_dbChip*) chip;
  _dbChipRegion* region = _chip->chip_region_tbl_->create();
  region->name_ = name;
  region->side_ = static_cast<uint8_t>(side);
  return (dbChipRegion*) region;
}

void dbChipRegion::destroy(dbChipRegion* chip_region)
{
  _dbChip* chip = (_dbChip*) chip_region->getChip();
  dbProperty::destroyProperties(chip_region);
  chip->chip_region_tbl_->destroy((_dbChipRegion*) chip_region);
}
// User Code End dbChipRegionPublicMethods
}  // namespace odb
// Generator Code End Cpp