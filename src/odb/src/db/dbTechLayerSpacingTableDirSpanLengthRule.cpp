// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2019-2025, The OpenROAD Authors

// Generator Code Begin Cpp
#include "dbTechLayerSpacingTableDirSpanLengthRule.h"

#include <cstdint>
#include <cstring>

#include "dbCore.h"
#include "dbDatabase.h"
#include "dbProperty.h"
#include "dbTable.h"
#include "dbTechLayer.h"
#include "odb/db.h"
// User Code Begin Includes
#include <algorithm>
#include <iterator>
#include <ranges>
#include <vector>

#include "dbVector.h"
// User Code End Includes
namespace odb {
template class dbTable<_dbTechLayerSpacingTableDirSpanLengthRule>;

bool _dbTechLayerSpacingTableDirSpanLengthRule::operator==(
    const _dbTechLayerSpacingTableDirSpanLengthRule& rhs) const
{
  // NOLINTBEGIN(readability-simplify-boolean-expr)
  if (flags_.wrong_direction != rhs.flags_.wrong_direction) {
    return false;
  }
  if (flags_.same_mask != rhs.flags_.same_mask) {
    return false;
  }
  if (flags_.except_eol != rhs.flags_.except_eol) {
    return false;
  }
  if (eol_width_ != rhs.eol_width_) {
    return false;
  }

  return true;
  // NOLINTEND(readability-simplify-boolean-expr)
}

bool _dbTechLayerSpacingTableDirSpanLengthRule::operator<(
    const _dbTechLayerSpacingTableDirSpanLengthRule& rhs) const
{
  return true;
}

_dbTechLayerSpacingTableDirSpanLengthRule::
    _dbTechLayerSpacingTableDirSpanLengthRule(_dbDatabase* db)
{
  flags_ = {};
  eol_width_ = 0;
}

static dbIStream& operator>>(
    dbIStream& stream,
    dbTechLayerSpacingTableDirSpanLengthRule::ExactSpanLengthSpacing& obj)
{
  stream >> obj.span_length_1;
  stream >> obj.span_length_2;
  stream >> obj.prl;
  stream >> obj.exact_spacings;
  return stream;
}
dbIStream& operator>>(dbIStream& stream,
                      _dbTechLayerSpacingTableDirSpanLengthRule& obj)
{
  uint32_t flags_bit_field;
  stream >> flags_bit_field;
  static_assert(sizeof(obj.flags_) == sizeof(flags_bit_field));
  std::memcpy(&obj.flags_, &flags_bit_field, sizeof(flags_bit_field));
  stream >> obj.eol_width_;
  stream >> obj.prl_tbl_;
  stream >> obj.span_length_tbl_;
  stream >> obj.spacing_tbl_;
  stream >> obj.exact_spacing_tbl_;
  stream >> obj.exact_span_length_spacing_tbl_;
  return stream;
}

static dbOStream& operator<<(
    dbOStream& stream,
    const dbTechLayerSpacingTableDirSpanLengthRule::ExactSpanLengthSpacing& obj)
{
  stream << obj.span_length_1;
  stream << obj.span_length_2;
  stream << obj.prl;
  stream << obj.exact_spacings;
  return stream;
}
dbOStream& operator<<(dbOStream& stream,
                      const _dbTechLayerSpacingTableDirSpanLengthRule& obj)
{
  uint32_t flags_bit_field;
  static_assert(sizeof(obj.flags_) == sizeof(flags_bit_field));
  std::memcpy(&flags_bit_field, &obj.flags_, sizeof(obj.flags_));
  stream << flags_bit_field;
  stream << obj.eol_width_;
  stream << obj.prl_tbl_;
  stream << obj.span_length_tbl_;
  stream << obj.spacing_tbl_;
  stream << obj.exact_spacing_tbl_;
  stream << obj.exact_span_length_spacing_tbl_;
  return stream;
}

void _dbTechLayerSpacingTableDirSpanLengthRule::collectMemInfo(MemInfo& info)
{
  info.cnt++;
  info.size += sizeof(*this);

  info.children["prl_tbl"].add(prl_tbl_);
  info.children["span_length_tbl"].add(span_length_tbl_);
  info.children["exact_spacing_tbl"].add(exact_spacing_tbl_);

  // User Code Begin collectMemInfo
  MemInfo& spacing_info = info.children["spacing_tbl"];
  for (const auto& spacing : spacing_tbl_) {
    spacing_info.add(spacing);
  }
  // add() only accounts for the struct bodies, so the nested vectors have to
  // be walked explicitly.
  MemInfo& exact_span_info = info.children["exact_span_length_spacing_tbl"];
  exact_span_info.add(exact_span_length_spacing_tbl_);
  for (const auto& entry : exact_span_length_spacing_tbl_) {
    exact_span_info.add(entry.exact_spacings);
  }
  // User Code End collectMemInfo
}

////////////////////////////////////////////////////////////////////
//
// dbTechLayerSpacingTableDirSpanLengthRule - Methods
//
////////////////////////////////////////////////////////////////////

void dbTechLayerSpacingTableDirSpanLengthRule::setEolWidth(int eol_width)
{
  _dbTechLayerSpacingTableDirSpanLengthRule* obj
      = (_dbTechLayerSpacingTableDirSpanLengthRule*) this;

  obj->eol_width_ = eol_width;
}

int dbTechLayerSpacingTableDirSpanLengthRule::getEolWidth() const
{
  _dbTechLayerSpacingTableDirSpanLengthRule* obj
      = (_dbTechLayerSpacingTableDirSpanLengthRule*) this;
  return obj->eol_width_;
}

void dbTechLayerSpacingTableDirSpanLengthRule::setWrongDirection(
    bool wrong_direction)
{
  _dbTechLayerSpacingTableDirSpanLengthRule* obj
      = (_dbTechLayerSpacingTableDirSpanLengthRule*) this;

  obj->flags_.wrong_direction = wrong_direction;
}

bool dbTechLayerSpacingTableDirSpanLengthRule::isWrongDirection() const
{
  _dbTechLayerSpacingTableDirSpanLengthRule* obj
      = (_dbTechLayerSpacingTableDirSpanLengthRule*) this;

  return obj->flags_.wrong_direction;
}

void dbTechLayerSpacingTableDirSpanLengthRule::setSameMask(bool same_mask)
{
  _dbTechLayerSpacingTableDirSpanLengthRule* obj
      = (_dbTechLayerSpacingTableDirSpanLengthRule*) this;

  obj->flags_.same_mask = same_mask;
}

bool dbTechLayerSpacingTableDirSpanLengthRule::isSameMask() const
{
  _dbTechLayerSpacingTableDirSpanLengthRule* obj
      = (_dbTechLayerSpacingTableDirSpanLengthRule*) this;

  return obj->flags_.same_mask;
}

void dbTechLayerSpacingTableDirSpanLengthRule::setExceptEol(bool except_eol)
{
  _dbTechLayerSpacingTableDirSpanLengthRule* obj
      = (_dbTechLayerSpacingTableDirSpanLengthRule*) this;

  obj->flags_.except_eol = except_eol;
}

bool dbTechLayerSpacingTableDirSpanLengthRule::isExceptEol() const
{
  _dbTechLayerSpacingTableDirSpanLengthRule* obj
      = (_dbTechLayerSpacingTableDirSpanLengthRule*) this;

  return obj->flags_.except_eol;
}

dbTechLayerSpacingTableDirSpanLengthRule*
dbTechLayerSpacingTableDirSpanLengthRule::create(dbTechLayer* parent)
{
  _dbTechLayer* _parent = (_dbTechLayer*) parent;
  return (dbTechLayerSpacingTableDirSpanLengthRule*)
      _parent->spacing_table_dir_span_length_rules_tbl_->create();
}
void dbTechLayerSpacingTableDirSpanLengthRule::destroy(
    dbTechLayerSpacingTableDirSpanLengthRule* obj)
{
  _dbTechLayer* _parent = (_dbTechLayer*) obj->getImpl()->getOwner();
  dbProperty::destroyProperties(obj);
  _parent->spacing_table_dir_span_length_rules_tbl_->destroy(
      (_dbTechLayerSpacingTableDirSpanLengthRule*) obj);
}
// User Code Begin dbTechLayerSpacingTableDirSpanLengthRulePublicMethods

int _dbTechLayerSpacingTableDirSpanLengthRule::getSpanLengthIdx(
    const int span_length) const
{
  // The spec matches the last row whose span length is strictly less than the
  // object's span length, so an object at or below the first row is unmatched.
  auto pos = std::ranges::lower_bound(span_length_tbl_, span_length);
  return (int) std::distance(span_length_tbl_.begin(), pos) - 1;
}

int _dbTechLayerSpacingTableDirSpanLengthRule::getPrlIdx(const int prl) const
{
  auto pos = std::ranges::lower_bound(prl_tbl_, prl);
  return (int) std::distance(prl_tbl_.begin(), pos) - 1;
}

void dbTechLayerSpacingTableDirSpanLengthRule::setSpacingTable(
    const std::vector<int>& span_length_tbl,
    const std::vector<int>& prl_tbl,
    const std::vector<std::vector<int>>& spacing_tbl,
    const std::vector<int>& exact_spacing_tbl)
{
  _dbTechLayerSpacingTableDirSpanLengthRule* obj
      = (_dbTechLayerSpacingTableDirSpanLengthRule*) this;
  obj->span_length_tbl_ = span_length_tbl;
  obj->prl_tbl_ = prl_tbl;
  obj->exact_spacing_tbl_ = exact_spacing_tbl;
  obj->spacing_tbl_.clear();
  for (const auto& spacing : spacing_tbl) {
    dbVector<int> row;
    row = spacing;
    obj->spacing_tbl_.push_back(row);
  }
}

void dbTechLayerSpacingTableDirSpanLengthRule::getSpacingTable(
    std::vector<int>& span_length_tbl,
    std::vector<int>& prl_tbl,
    std::vector<std::vector<int>>& spacing_tbl,
    std::vector<int>& exact_spacing_tbl) const
{
  _dbTechLayerSpacingTableDirSpanLengthRule* obj
      = (_dbTechLayerSpacingTableDirSpanLengthRule*) this;
  span_length_tbl = obj->span_length_tbl_;
  prl_tbl = obj->prl_tbl_;
  exact_spacing_tbl = obj->exact_spacing_tbl_;
  spacing_tbl.clear();
  for (const auto& spacing : obj->spacing_tbl_) {
    spacing_tbl.push_back(spacing);
  }
}

void dbTechLayerSpacingTableDirSpanLengthRule::addExactSpanLengthSpacing(
    const int span_length_1,
    const int span_length_2,
    const int prl,
    const std::vector<int>& exact_spacings)
{
  _dbTechLayerSpacingTableDirSpanLengthRule* obj
      = (_dbTechLayerSpacingTableDirSpanLengthRule*) this;
  obj->exact_span_length_spacing_tbl_.push_back(
      {span_length_1, span_length_2, prl, exact_spacings});
}

void dbTechLayerSpacingTableDirSpanLengthRule::getExactSpanLengthSpacings(
    std::vector<ExactSpanLengthSpacing>& tbl) const
{
  _dbTechLayerSpacingTableDirSpanLengthRule* obj
      = (_dbTechLayerSpacingTableDirSpanLengthRule*) this;
  tbl = obj->exact_span_length_spacing_tbl_;
}

int dbTechLayerSpacingTableDirSpanLengthRule::getSpacing(
    const int span_length_1,
    const int span_length_2,
    const int prl) const
{
  _dbTechLayerSpacingTableDirSpanLengthRule* obj
      = (_dbTechLayerSpacingTableDirSpanLengthRule*) this;

  const int col = obj->getPrlIdx(prl);
  if (col < 0) {
    // prl is at or below the first column: the rule does not apply.
    return 0;
  }
  const int row1 = obj->getSpanLengthIdx(span_length_1);
  const int row2 = obj->getSpanLengthIdx(span_length_2);
  if (row1 < 0 && row2 < 0) {
    return 0;
  }
  const int spacing1 = (row1 < 0) ? 0 : obj->spacing_tbl_[row1][col];
  const int spacing2 = (row2 < 0) ? 0 : obj->spacing_tbl_[row2][col];
  // A zero cell means that object is unconstrained, and the spec says the
  // other object's requirement is then not checked either.
  if (spacing1 == 0 || spacing2 == 0) {
    return 0;
  }
  return std::max(spacing1, spacing2);
}

bool dbTechLayerSpacingTableDirSpanLengthRule::hasExactSpacing(
    const int span_length) const
{
  _dbTechLayerSpacingTableDirSpanLengthRule* obj
      = (_dbTechLayerSpacingTableDirSpanLengthRule*) this;
  const int row = obj->getSpanLengthIdx(span_length);
  if (row < 0 || row >= (int) obj->exact_spacing_tbl_.size()) {
    return false;
  }
  return obj->exact_spacing_tbl_[row] >= 0;
}

int dbTechLayerSpacingTableDirSpanLengthRule::getExactSpacing(
    const int span_length) const
{
  _dbTechLayerSpacingTableDirSpanLengthRule* obj
      = (_dbTechLayerSpacingTableDirSpanLengthRule*) this;
  const int row = obj->getSpanLengthIdx(span_length);
  if (row < 0 || row >= (int) obj->exact_spacing_tbl_.size()) {
    return -1;
  }
  return obj->exact_spacing_tbl_[row];
}

// User Code End dbTechLayerSpacingTableDirSpanLengthRulePublicMethods
}  // namespace odb
// Generator Code End Cpp