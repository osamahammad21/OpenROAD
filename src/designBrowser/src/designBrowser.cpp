#include "designBrowser/designBrowser.hh"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

#include "db_sta/dbReadVerilog.hh"
#include "opendb/db.h"
#include "sta/Liberty.hh"
#include "sta/PortDirection.hh"
#include "sta/VerilogNamespace.hh"
#include "utl/Logger.h"

namespace ord {
using std::endl;
using std::fixed;
using std::left;
using std::ofstream;
using std::ostringstream;
using std::pair;
using std::right;
using std::setprecision;
using std::setw;
using std::size_t;
using std::sort;
using std::string;
using std::to_string;
using std::vector;

// ***********************************************************
// ***** Utility Function: Convert double to string prec(2)
// ***********************************************************
string numberToString(double a)
{
  ostringstream streamObj;
  streamObj << fixed;
  streamObj << setprecision(2);
  streamObj << a;
  string str = streamObj.str();
  return str;
}
// ***********************************************************
// ***** Utility Function: Sorting comparison function
// ***********************************************************
template <class K, class V>
bool cmpPair(pair<K, V>& a, pair<K, V>& b)
{
  return a.second < b.second;
}

// ***********************************************************
// ***** Utility Functions:   Reports
// ***********************************************************
void printLogicalInfo(std::ofstream& file,
                      odb::dbModule* mod,
                      int level,
                      const ModuleData& data)
{
  file << "\"module_name\" : \"" << mod->getName() << "\"," << std::endl;
  file << "\"level\" : " << level << "," << std::endl;
  file << "\"pins\" : " << mod->getNumPins() << "," << std::endl;
  file << "\"total_insts\" : " << data.getTotalStdCells() << "," << std::endl;
  file << "\"total_macros\" : " << data.total_macros << "," << std::endl;
  file << "\"total_area\" : " << numberToString(data.getTotalArea()) << ","
       << std::endl;
  file << "\"local_insts\" : " << data.std_cells << "," << std::endl;
  file << "\"local_macros\" : " << data.macros << "," << std::endl;
}

void printAreaInfo(std::ofstream& file,
                   odb::dbModule* mod,
                   const ModuleData& data)
{
  file << std::endl;
  file << std::setw(40) << " " << std::left << std::setw(20) << "Number"
       << std::setw(20) << "Area(um^2)" << std::endl;
  file << std::string(80, '-') << std::endl;
  file << std::setw(40) << std::left << "Standard Cells" << std::setw(20)
       << data.getTotalStdCells();
  file << std::setw(20) << numberToString(data.getTotalStdCellArea())
       << std::endl;
  file << std::setw(40) << "Combinational Standard Cells" << std::setw(20)
       << data.total_comb_cells;
  file << std::setw(20) << numberToString(data.total_comb_cell_area)
       << std::endl;
  file << std::setw(40) << "Inv/Buffer Cells" << std::setw(20)
       << data.total_buf_inv_cells;
  file << std::setw(20) << numberToString(data.total_buf_inv_cell_area)
       << std::endl;
  file << std::setw(40) << "Sequential Cells" << std::setw(20)
       << data.total_seq_cells;
  file << std::setw(20) << numberToString(data.total_seq_cell_area)
       << std::endl;
  file << std::setw(40) << "Hard Macros" << std::setw(20) << data.total_macros;
  file << std::setw(20) << numberToString(data.total_macro_area) << std::endl;
  file << std::string(80, '-') << std::endl;
  file << std::setw(60) << std::left << "Total Area";
  file << std::setw(20) << numberToString(data.getTotalArea()) << std::endl;
}

void printHierInfo(std::ofstream& file,
                   odb::dbModule* mod,
                   const ModuleData& data)
{
  file << std::setw(15) << numberToString(data.getTotalArea()) + " |";
  file << std::setw(25)
       << std::to_string(data.getTotalStdCells()) + "/"
              + numberToString(data.getTotalStdCellArea()) + " |";
  file << std::setw(25)
       << std::to_string(data.total_macros) + "/"
              + numberToString(data.total_macro_area) + " |";
  file << std::setw(20)
       << std::to_string(data.std_cells) + "/"
              + numberToString(data.std_cell_area) + " |";
  file << std::setw(20)
       << std::to_string(data.macros) + "/" + numberToString(data.macro_area)
              + " |";
  file << std::setw(15) << std::to_string(mod->getChildren().size()) + " |";
  if (mod->getModInst() == nullptr) {
    file << std::setw(70) << std::left
         << std::string(mod->getName()) + " || NULL" << std::right << std::endl;
  } else {
    file << std::setw(70) << std::left
         << std::string(mod->getModInst()->getHierarchalName())
                + std::string(" || ") + mod->getName()
         << std::right << std::endl;
  }
}


// ************************************************************************
// ************ Class designBrowserKernel
// ************************************************************************
void designBrowserKernel::createDBModule(sta::Instance* inst, odb::dbModule* parent)
{
  sta::Cell* cell = _network->cell(inst);
  auto block = _db->getChip()->getBlock();
  auto mod = odb::dbModule::create(block, _network->name(cell));
  odb::dbModInst* modinst = nullptr;
  if (mod == nullptr)
    logger_->error(
        utl::DBR, 1, "dbModule creation failed for {}", _network->name(cell));
  if (parent == nullptr)
    block->setTopModule(mod);
  else {
    modinst = odb::dbModInst::create(parent, mod, _network->name(inst));
    if (modinst == nullptr)
      logger_->error(utl::DBR,
                     2,
                     "dbModInst creation failed for {}:{}",
                     _network->name(cell),
                     _network->name(inst));
  }
  sta::InstanceChildIterator* child_iter = _network->childIterator(inst);
  while (child_iter->hasNext()) {
    sta::Instance* child = child_iter->next();
    if (_network->isHierarchical(child)) {
      createDBModule(child, mod);
    } else {
      string path = "";
      if (modinst != nullptr)
        path = modinst->getHierarchalName() + "/" + _network->name(child);
      else
        path = _network->name(child);
      auto dbinst = block->findInst(path.c_str());
      if (dbinst == nullptr)
        logger_->error(utl::DBR,
                       3,
                       "Inst {} is not found in database for module {}",
                       _network->name(child),
                       mod->getName());
      mod->addInst(dbinst);
    }
  }

  int pin_count = 0;
  sta::InstancePinIterator* pin_iter = _network->pinIterator(inst);
  while (pin_iter->hasNext()) {
    sta::Pin* pin = pin_iter->next();
    pin_count++;
  }
  mod->setNumPins(pin_count);
  if (mod->getChildren().reversible() && mod->getChildren().orderReversed())
    mod->getChildren().reverse();
  if (mod->getInsts().reversible() && mod->getInsts().orderReversed())
    mod->getInsts().reverse();
}

int getTermCount(sta::LibertyCell* libcell)
{
  int term_count = 0;
  if(libcell == nullptr)
    return 0;
  sta::LibertyCellPortIterator port_iter(libcell); 
  while(port_iter.hasNext())
  {
    sta::LibertyPort *port = port_iter.next();
    term_count++;
  }
  return term_count;
}

void designBrowserKernel::updateModuleInfo(odb::dbModule* mod)
{
  ModuleData data;
  for (auto inst : mod->getInsts()) {
    auto master = inst->getMaster();
    auto cell = _network->findAnyCell(master->getName().c_str());
    auto libcell = _network->libertyCell(cell);
    int term_count = getTermCount(libcell);
    double cellArea = area(master);
    if (master->isBlock()) {
      data.macros++;
      data.macro_area += cellArea;
    } else {
      data.std_cell_area += cellArea;
      data.std_cells++;

      if (libcell->isBuffer() || libcell->isInverter()) {
        data.total_buf_inv_cells++;
        data.total_buf_inv_cell_area
            += cellArea;
      }
      
      if(libcell->hasSequentials())
      {
        data.total_seq_cells++;
        data.total_seq_cell_area += cellArea;
      } else {
        data.total_comb_cells++;
        data.total_comb_cell_pins += term_count;
        data.total_comb_cell_area += cellArea;
      }
    }
    data.total_macros = data.macros;
    data.num_pins += term_count;
    data.macros_map[master]++;
  }
  data.total_macro_area = data.macro_area;
  for (auto child : mod->getChildren()) {
    auto childMod = child->getMaster();
    uint cid = childMod->getId();
    if (!module_data_[cid].valid)
      updateModuleInfo(childMod);
    data.appendData(module_data_[cid]);
  }
  data.valid = true;
  module_data_[mod->getId()] = data;
}

//////////////////////////////////////////////////////////////////////////////
// Note:  dbuToMeters, metersToDbu, area are copied from Resizer.cc file
double designBrowserKernel::dbuToMicrons(int dist) const
{
  double dbu = _db->getTech()->getDbUnitsPerMicron();
  return dist / dbu;
}

int designBrowserKernel::micronsToDbu(double dist) const
{
  int dbu = _db->getTech()->getDbUnitsPerMicron();
  return dist * dbu;
}

double designBrowserKernel::area(odb::dbMaster* master)
{
  return dbuToMicrons(master->getWidth()) * dbuToMicrons(master->getHeight());
}
////////////////////////////////////////////////////////////////////////////////

void designBrowserKernel::linkDesignBrowserKernel(ord::dbVerilogNetwork* network,
                                                  dbDatabase* db)
{
  init(network, db);
  createDBModule(network->topInstance());
  updateModuleInfo(db->getChip()->getBlock()->getTopModule());
}

void designBrowserKernel::reportMacroUtil(odb::dbModule* mod, ofstream& file)
{
  for (auto child : mod->getChildren())
    reportMacroUtil(child->getMaster(), file);
  for (auto inst : mod->getInsts()) {
    auto master = inst->getMaster();
    if (master->isBlock()) {
      string pathName = inst->getName();
      if (mod->getModInst() != nullptr) {
        pathName = mod->getModInst()->getHierarchalName() + pathName;
      }
      file << master->getName() << "," << pathName << ","
           << dbuToMicrons(master->getWidth());
      file << "," << dbuToMicrons(master->getHeight()) << endl;
    }
  }
}

void designBrowserKernel::traverseDepthUtil(odb::dbModule* mod,
                                            int value,
                                            ofstream& file)
{
  if (value > _level) {
    return;
  }
  file << setw(10) << to_string(value++) + " |";
  printHierInfo(file, mod, module_data_[mod->getId()]);
  file << endl;
  for (auto child : mod->getChildren()) {
    traverseDepthUtil(child->getMaster(), value, file);
  }
}

void designBrowserKernel::traverseDesign(string name, ofstream& file)
{
  auto mod = _db->getChip()->getBlock()->findModule(name.c_str());
  if (mod == nullptr)
    logger_->error(utl::DBR, 4, "Could not find module {}", name);
  traverseDepthUtil(mod, 0, file);
}

void designBrowserKernel::reportLogicAreaKeyTermCountUtil(odb::dbModule* mod,
                                                          ofstream& file)
{
  auto data = module_data_[mod->getId()];
  vector<pair<odb::dbMaster*, uint>> sorted_cells;
  for (auto macro : data.macros_map) {
    auto cell = _network->findAnyCell(macro.first->getName().c_str());
    auto libcell = _network->libertyCell(cell);
    sorted_cells.push_back({macro.first, (uint) getTermCount(libcell)});
  }
  sort(sorted_cells.begin(), sorted_cells.end(), cmpPair<odb::dbMaster*, uint>);
  for (auto cell : sorted_cells) {
    auto master = cell.first;
    int term_count = cell.second;
    float cell_area = area(master);
    int cell_count = data.macros_map[master];
    file << left << setw(40) << master->getName() << setw(20) << term_count
         << setw(20) << cell_count << setw(20) << numberToString(cell_area);
    file << setw(20) << numberToString(cell_area * cell_count) << endl;
  }
}

void designBrowserKernel::reportLogicAreaKeyCellCountUtil(odb::dbModule* mod,
                                                          ofstream& file)
{
  auto data = module_data_[mod->getId()];
  vector<pair<odb::dbMaster*, uint>> sorted_cells;
  for (auto macro : data.macros_map)
    sorted_cells.push_back(macro);

  sort(sorted_cells.begin(), sorted_cells.end(), cmpPair<odb::dbMaster*, uint>);
  for (auto cell : sorted_cells) {
    auto master = cell.first;
    auto stacell = _network->findAnyCell(master->getName().c_str());
    auto libcell = _network->libertyCell(stacell);
    int term_count = getTermCount(libcell);;
    float cell_area = area(master);
    int cell_count = cell.second;
    file << left << setw(40) << master->getName() << setw(20) << term_count
         << setw(20) << cell_count << setw(20) << numberToString(cell_area);
    file << setw(20) << numberToString(cell_area * cell_count) << endl;
  }
}

void designBrowserKernel::reportDesignFileUtil(odb::dbModule* mod,
                                               int level,
                                               ofstream& file)
{
  level = level + 1;
  file << "{" << endl;
  file << "\"local_instance_name\" : \"" << mod->getModInst()->getName()
       << "\"," << endl;
  file << "\"instance_name\" : \"" << mod->getModInst()->getHierarchalName()
       << "\"," << endl;
  file << "\"module\" : {" << endl;
  printLogicalInfo(file, mod, level, module_data_[mod->getId()]);
  file << "\"module_instances\" : [" << endl;
  size_t i = 0;
  uint childrenSz = mod->getChildren().size();
  if (childrenSz > 0) {
    uint cnt = 0;
    for (auto child : mod->getChildren()) {
      reportDesignFileUtil(child->getMaster(), level, file);
      if (++cnt < childrenSz)
        file << "," << endl;
    }
  }
  file << "]" << endl;
  file << "}" << endl;
  file << "}" << endl;
}

void designBrowserKernel::reportLogicArea(string name,
                                          ofstream& file,
                                          bool detailed,
                                          const char* key)
{
  auto mod = _db->getChip()->getBlock()->findModule(name.c_str());
  if (mod == nullptr)
    logger_->error(utl::DBR, 5, "Could not find module {}", name);
  printAreaInfo(file, mod, module_data_[mod->getId()]);

  if (detailed) {
    file << endl;
    file << endl;
    file << string(120, '*') << endl;
    file << left << setw(40) << "Cell name" << setw(20) << "# Pins" << setw(20)
         << "Number" << setw(20) << "Area" << setw(20) << "Total Area" << endl;
    if (string(key) == "cell_count") {
      reportLogicAreaKeyCellCountUtil(mod, file);
    } else {
      reportLogicAreaKeyTermCountUtil(mod, file);
    }
    file << "The average pins of combinational cell is  "
         << module_data_[mod->getId()].total_comb_cell_pins * 1.0
                / module_data_[mod->getId()].total_comb_cells
         << endl;
  }
}

void designBrowserKernel::reportMacro(string name, ofstream& file)
{
  auto mod = _db->getChip()->getBlock()->findModule(name.c_str());
  if (mod == nullptr)
    logger_->error(utl::DBR, 6, "Could not find module {}", name);
  reportMacroUtil(mod, file);
}

void designBrowserKernel::reportDesignFile(ofstream& file)
{
  file << "{" << endl;
  file << "\"logical_hierarchy\" : {" << endl;
  auto mod = _db->getChip()->getBlock()->getTopModule();
  printLogicalInfo(file, mod, 0, module_data_[mod->getId()]);
  file << "\"module_instances\" : [" << endl;
  uint childrenSz = mod->getChildren().size();
  if (childrenSz > 0) {
    uint cnt = 0;
    for (auto child : mod->getChildren()) {
      reportDesignFileUtil(child->getMaster(), 0, file);
      if (++cnt < childrenSz)
        file << "," << endl;
    }
  }

  file << "]" << endl;
  file << "}" << endl;
  file << "}" << endl;
}

//***************************************************************************************************
//******** Interface functions
//***************************************************************************************************
designBrowserKernel* makeDesignBrowser(utl::Logger* logger)
{
  return new designBrowserKernel(logger);
}

void deleteDesignBrowser(designBrowserKernel* browser)
{
  delete browser;
}

void dbLinkDesignBrowser(designBrowserKernel* browser,
                         ord::dbVerilogNetwork* network,
                         odb::dbDatabase* db)
{
  browser->linkDesignBrowserKernel(network, db);
}

void dbDesignBrowser(designBrowserKernel* browser,
                     const char* name,
                     const char* file_name,
                     int level)
{
  ofstream output_file;
  output_file.open(file_name);
  output_file << string(100, '*') << endl;
  output_file << "Note: for all pairs a/b, a is the number of items, b is the "
                 "area of iterms in unit of um^2"
              << endl;
  output_file << string(100, '*') << endl;
  output_file << endl;
  output_file << setw(10) << "Level |";
  output_file << setw(15) << "Total Area |";
  output_file << setw(25) << "Total Std Cell Insts |";
  output_file << setw(25) << "Total Hard Macros |";
  output_file << setw(20) << "Std Cell Insts |";
  output_file << setw(20) << "Hard Macros |";
  output_file << setw(15) << "# submodules |";
  output_file << setw(70) << left << "Module Name/Instance Name";
  output_file << right << endl;
  browser->specifyLevel(level);
  browser->traverseDesign(string(name), output_file);
  output_file.close();
}

void dbReportLogicArea(designBrowserKernel* browser,
                       const char* name,
                       const char* file_name,
                       bool detailed,
                       const char* key)
{
  ofstream output_file;
  output_file.open(file_name);
  browser->reportLogicArea(string(name), output_file, detailed, key);
  output_file.close();
}

void dbReportMacro(designBrowserKernel* browser,
                   const char* name,
                   const char* file_name)
{
  ofstream output_file;
  output_file.open(file_name);
  browser->reportMacro(string(name), output_file);
  output_file.close();
}

void dbReportDesignFile(designBrowserKernel* browser, const char* file_name)
{
  ofstream output_file;
  output_file.open(file_name);
  browser->reportDesignFile(output_file);
  output_file.close();
}

}  // namespace ord
