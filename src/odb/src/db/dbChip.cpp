// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2019-2025, The OpenROAD Authors

// Generator Code Begin Cpp
#include "dbChip.h"

#include "dbBlock.h"
#include "dbBlockItr.h"
#include "dbDatabase.h"
#include "dbNameCache.h"
#include "dbProperty.h"
#include "dbPropertyItr.h"
#include "dbTable.h"
#include "dbTable.hpp"
#include "dbTech.h"
#include "odb/db.h"
#include "odb/dbSet.h"
namespace odb {
template class dbTable<_dbChip>;

bool _dbChip::operator==(const _dbChip& rhs) const
{
  if (_top != rhs._top) {
    return false;
  }
  if (*_prop_tbl != *rhs._prop_tbl) {
    return false;
  }

  // User Code Begin ==
  if (*_block_tbl != *rhs._block_tbl) {
    return false;
  }
  if (*_name_cache != *rhs._name_cache) {
    return false;
  }
  // User Code End ==
  return true;
}

bool _dbChip::operator<(const _dbChip& rhs) const
{
  if (_top >= rhs._top) {
    return false;
  }

  return true;
}

_dbChip::_dbChip(_dbDatabase* db)
{
  _prop_tbl = new dbTable<_dbProperty>(
      db, this, (GetObjTbl_t) &_dbChip::getObjectTable, dbPropertyObj);
  // User Code Begin Constructor
  _block_tbl = new dbTable<_dbBlock>(
      db, this, (GetObjTbl_t) &_dbChip::getObjectTable, dbBlockObj);
  _name_cache
      = new _dbNameCache(db, this, (GetObjTbl_t) &_dbChip::getObjectTable);

  _block_itr = new dbBlockItr(_block_tbl);

  _prop_itr = new dbPropertyItr(_prop_tbl);
  // User Code End Constructor
}

dbIStream& operator>>(dbIStream& stream, _dbChip& obj)
{
  stream >> obj._top;
  // User Code Begin >>
  stream >> *obj._block_tbl;
  stream >> *obj._prop_tbl;
  stream >> *obj._name_cache;
  // User Code End >>
  return stream;
}

dbOStream& operator<<(dbOStream& stream, const _dbChip& obj)
{
  dbOStreamScope scope(stream, "dbChip");
  stream << obj._top;
  // User Code Begin <<
  stream << *obj._block_tbl;
  stream << NamedTable("prop_tbl", obj._prop_tbl);
  stream << *obj._name_cache;
  // User Code End <<
  return stream;
}

dbObjectTable* _dbChip::getObjectTable(dbObjectType type)
{
  switch (type) {
    case dbPropertyObj:
      return _prop_tbl;
      // User Code Begin getObjectTable
    case dbBlockObj:
      return _block_tbl;
    // User Code End getObjectTable
    default:
      break;
  }
  return getTable()->getObjectTable(type);
}
void _dbChip::collectMemInfo(MemInfo& info)
{
  info.cnt++;
  info.size += sizeof(*this);

  _prop_tbl->collectMemInfo(info.children_["_prop_tbl"]);

  // User Code Begin collectMemInfo
  _block_tbl->collectMemInfo(info.children_["block"]);
  _name_cache->collectMemInfo(info.children_["name_cache"]);
  // User Code End collectMemInfo
}

_dbChip::~_dbChip()
{
  delete _prop_tbl;
  // User Code Begin Destructor
  delete _block_tbl;
  delete _name_cache;
  delete _block_itr;
  delete _prop_itr;
  // User Code End Destructor
}

////////////////////////////////////////////////////////////////////
//
// dbChip - Methods
//
////////////////////////////////////////////////////////////////////

// User Code Begin dbChipPublicMethods
dbBlock* dbChip::getBlock()
{
  _dbChip* chip = (_dbChip*) this;

  if (chip->_top == 0) {
    return nullptr;
  }

  return (dbBlock*) chip->_block_tbl->getPtr(chip->_top);
}

dbChip* dbChip::create(dbDatabase* db_)
{
  _dbDatabase* db = (_dbDatabase*) db_;

  if (db->_chip != 0) {
    return nullptr;
  }

  _dbChip* chip = db->_chip_tbl->create();
  db->_chip = chip->getOID();
  return (dbChip*) chip;
}

dbChip* dbChip::getChip(dbDatabase* db_, uint dbid_)
{
  _dbDatabase* db = (_dbDatabase*) db_;
  return (dbChip*) db->_chip_tbl->getPtr(dbid_);
}

void dbChip::destroy(dbChip* chip_)
{
  _dbChip* chip = (_dbChip*) chip_;
  _dbDatabase* db = chip->getDatabase();
  dbProperty::destroyProperties(chip);
  db->_chip_tbl->destroy(chip);
  db->_chip = 0;
}
// User Code End dbChipPublicMethods
}  // namespace odb
// Generator Code End Cpp