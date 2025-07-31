// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2019-2025, The OpenROAD Authors

// Generator Code Begin Header
#pragma once

#include "dbCore.h"
#include "odb/odb.h"

namespace odb {
class dbIStream;
class dbOStream;
class _dbDatabase;
class dbPropertyItr;
class _dbNameCache;
class dbBlockItr;
class _dbProperty;
class _dbBlock;

class _dbChip : public _dbObject
{
 public:
  _dbChip(_dbDatabase*);

  ~_dbChip();

  bool operator==(const _dbChip& rhs) const;
  bool operator!=(const _dbChip& rhs) const { return !operator==(rhs); }
  bool operator<(const _dbChip& rhs) const;
  dbObjectTable* getObjectTable(dbObjectType type);
  void collectMemInfo(MemInfo& info);

  dbId<_dbBlock> _top;
  dbTable<_dbBlock>* _block_tbl;
  _dbNameCache* _name_cache;
  dbBlockItr* _block_itr;
  dbPropertyItr* _prop_itr;
  dbTable<_dbProperty>* _prop_tbl;
};
dbIStream& operator>>(dbIStream& stream, _dbChip& obj);
dbOStream& operator<<(dbOStream& stream, const _dbChip& obj);
}  // namespace odb
// Generator Code End Header