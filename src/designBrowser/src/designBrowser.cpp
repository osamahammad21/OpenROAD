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
using ord::dbVerilogNetwork;

using sta::Cell;
using sta::Instance;
using sta::InstanceChildIterator;
using sta::InstancePinIterator;
using sta::instanceVerilogName;
using sta::LibertyCell;
using sta::LibertyCellPortIterator;
using sta::LibertyPort;
using sta::Net;
using sta::NetConnectedPinIterator;
using sta::NetIterator;
using sta::NetPinIterator;
using sta::NetTermIterator;
using sta::Pin;
using sta::PinSeq;
using sta::PortDirection;
using sta::Term;

using odb::dbDatabase;
using odb::dbMaster;
using odb::dbMTerm;
using odb::dbSet;
using odb::dbSigType;

using std::cout;
using std::endl;
using std::fixed;
using std::left;
using std::map;
using std::ofstream;
using std::ostringstream;
using std::pair;
using std::queue;
using std::right;
using std::setprecision;
using std::setw;
using std::size_t;
using std::sort;
using std::string;
using std::to_string;
using std::vector;

// ***********************************************************
// ***** Utility Function:   Convert number to string
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

bool cmpPair(pair<LibertyCell*, int>& a, pair<LibertyCell*, int>& b)
{
  return a.second < b.second;
}

// This function copied from dbSTA
bool Module::hasTerminals(Net* net)
{
  NetTermIterator* term_iter = _network->termIterator(net);
  bool has_terms = term_iter->hasNext();
  delete term_iter;
  return has_terms;
}

void Module::calculateAverageCombFanouts()
{
  calculateAverageCombFanoutsUtil(_inst);
}

void Module::calculateAverageCombFanoutsUtil(Instance* inst)
{
  NetIterator* net_iter = _network->netIterator(inst);
  while (net_iter->hasNext()) {
    Net* net = net_iter->next();
    if (!hasTerminals(net)) {
      bool comb_net_flag = true;
      bool driver_flag = false;
      vector<Instance*> loads;

      NetConnectedPinIterator* pin_iter = _network->connectedPinIterator(net);
      while (comb_net_flag && pin_iter->hasNext()) {
        Pin* pin = pin_iter->next();
        if (_network->isLeaf(pin)) {
          Instance* inst_temp = _network->instance(pin);
          Cell* cell = _network->cell(inst_temp);
          LibertyCell* liberty_cell = _network->libertyCell(cell);
          comb_net_flag = !(liberty_cell->hasSequentials());

          PortDirection* port_dir = _network->direction(pin);
          string dir;

          if (port_dir == PortDirection::input()) {
            dir = string("load(input)");
          } else if (port_dir == PortDirection::output()) {
            dir = string("driver(output)");
          } else if (port_dir == PortDirection::bidirect()) {
            dir = string("inout");
          } else if (port_dir == PortDirection::tristate()) {
            dir = string("driver(output)");
          } else {
            dir = string("inout");
          }

          if (dir == string("driver(output)")) {
            driver_flag = true;
          } else if (dir == string("load(input)")) {
            loads.push_back(inst);
          }
        }
      }

      if (comb_net_flag && driver_flag) {
        _total_comb_nets++;
        _total_comb_fanouts = _total_comb_fanouts + loads.size();
      }
    }
  }

  delete net_iter;

  InstanceChildIterator* child_iter = _network->childIterator(inst);
  while (child_iter->hasNext()) {
    Instance* child = child_iter->next();
    calculateAverageCombFanoutsUtil(child);
  }
}

void Module::printNetUtil(Net* net, ofstream& file)
{
  NetTermIterator* term_iter = _network->termIterator(net);
  while (term_iter->hasNext()) {
    file << "This net has a terminal" << endl;
    Term* term = term_iter->next();
    Pin* pin = _network->pin(term);
    Instance* inst = _network->instance(pin);

    const char* inst_name = _network->pathName(inst);
    const char* port_name = _network->portName(pin);

    if (inst == _network->topInstance()) {
      inst_name = "NULL";
    }

    string dir;
    PortDirection* port_dir = _network->direction(pin);
    if (port_dir == PortDirection::input()) {
      dir = string("external_driver(input)");
    } else if (port_dir == PortDirection::output()) {
      dir = string("external_load(output)");
    } else if (port_dir == PortDirection::bidirect()) {
      dir = string("inout");
    } else if (port_dir == PortDirection::tristate()) {
      dir = string("external_load(output)");
    } else {
      dir = string("inout");
    }

    file << "instance name:   " << setw(40) << inst_name
         << "    port name:  " << setw(20) << port_name
         << "   direction:   " << dir << endl;
  }

  NetPinIterator* pin_iter = _network->pinIterator(net);

  while (pin_iter->hasNext()) {
    Pin* pin = pin_iter->next();
    if (_network->isTopLevelPort(pin)) {
      file << "This is a top level port" << endl;
    }

    Instance* inst = _network->instance(pin);
    const char* inst_name = _network->pathName(inst);
    const char* port_name = _network->portName(pin);

    string dir;
    PortDirection* port_dir = _network->direction(pin);
    if (port_dir == PortDirection::input()) {
      dir = string("load(input)");
    } else if (port_dir == PortDirection::output()) {
      dir = string("driver(output)");
    } else if (port_dir == PortDirection::bidirect()) {
      dir = string("inout");
    } else if (port_dir == PortDirection::tristate()) {
      dir = string("driver(output)");
    } else {
      dir = string("inout");
    }

    file << "instance name:   " << setw(40) << inst_name
         << "    port name:  " << setw(20) << port_name
         << "   direction:   " << dir << endl;
  }
}

// *******************************************************
// ************ Struct Node
// *******************************************************
Node* createNode(Module* module)
{
  Node* node = new Node;
  node->_module = module;
  node->_parent = nullptr;
  return node;
}

// *******************************************************
// *

// *******************************************************
// ************ Struct ModuleInfo
// *******************************************************
void designBrowserKernel::updateModuleInfo(odb::dbModule* mod)
{  
  ModuleData data;
  for(auto inst : mod->getInsts())
  {
    auto master = inst->getMaster();
    double cellArea = area(master);
    if(master->isBlock())
    {
      data.macros++;
      data.macro_area += cellArea;
    }
    else {
      data.std_cell_area += cellArea;
      data.std_cells++;
      if(master->isSequential())
      {
        data.total_seq_cells++;
        data.total_seq_cell_area+= cellArea;
      } else {
        data.total_comb_cells++;
        data.total_comb_cell_pins+= master->getMTermCount();
        data.total_comb_cell_area+= cellArea;
      }
    }
    data.total_macros = data.macros;
    data.num_pins += master->getMTermCount();
  }
  data.total_macro_area = data.macro_area;
  for(auto child : mod->getChildren())
  {
    auto childMod = child->getMaster();
    uint cid = childMod->getId();
    if(!module_info_[cid].valid)
      updateModuleInfo(childMod);
    data.appendData(module_info_[cid]);
  }
  data.valid = true;
  module_info_[mod->getId()] = data;
}

// ************************************************************************
// ************ Class designBrowserKernel
// ************************************************************************
void designBrowserKernel::linkDesignBrowserKernelUtil(Instance* inst)
{
  Module current_module(_network, inst);

  NetIterator* net_iter = _network->netIterator(inst);
  while (net_iter->hasNext()) {
    Net* net = net_iter->next();
    current_module.addNet(net);
  }

  InstanceChildIterator* child_iter = _network->childIterator(inst);
  while (child_iter->hasNext()) {
    Instance* child = child_iter->next();
    if (_network->isHierarchical(child)) {
      current_module.addSubmodule(child);
      _module_queue.push(child);
    } else {
      Cell* cell = _network->cell(child);
      const char* child_name = _network->name(cell);
      dbMaster* master = _db->findMaster(child_name);

      if (master->isBlock()) {
        current_module.addMacro(child);
      } else {
        current_module.addStdCell(child);
      }
    }
  }

  int pin_count = 0;
  InstancePinIterator* pin_iter = _network->pinIterator(inst);
  while (pin_iter->hasNext()) {
    Pin* pin = pin_iter->next();
    pin_count++;
  }

  current_module.specifyNumberPins(pin_count);

  _module_list.push_back(current_module);

  while (!_module_queue.empty()) {
    Instance* submodule_inst = _module_queue.front();
    _module_queue.pop();
    linkDesignBrowserKernelUtil(submodule_inst);
  }
}

void designBrowserKernel::createDBModule(Instance* inst, odb::dbModule* parent)
{
  sta::Cell* cell = _network->cell(inst);
  auto block = _db->getChip()->getBlock();
  auto mod = odb::dbModule::create(block, _network->name(cell));
  odb::dbModInst* modinst = nullptr;
  if(mod == nullptr)
    logger_->error(utl::DBR, 1, "dbModule creation failed for {}", _network->name(cell));
  if (parent==nullptr)
    block->setTopModule(mod);
  else{
    modinst = odb::dbModInst::create(parent, mod, _network->name(inst));
    if(modinst == nullptr)
      logger_->error(utl::DBR, 2, "dbModInst creation failed for {}:{}", _network->name(cell), _network->name(inst));
  }
  InstanceChildIterator* child_iter = _network->childIterator(inst);
  while (child_iter->hasNext()) {
    Instance* child = child_iter->next();
    if (_network->isHierarchical(child)) {
      createDBModule(child, mod);
    } else {
      string path = "";
      if(modinst != nullptr)
        path = modinst->getHierarchalName() + "/" + _network->name(child);
      else
        path = _network->name(child);
      auto dbinst = block->findInst(path.c_str());
      if (dbinst == nullptr)
        logger_->error(utl::DBR, 3, "Inst {} is not found in database for module {}", _network->name(child), mod->getName());
      mod->addInst(dbinst);
      auto childCell = _network->cell(child);
      auto libcell = _network->libertyCell(childCell);
      //buf_inv_cell hack
      if(libcell->isBuffer() || libcell->isInverter()){
        module_info_[mod->getId()].total_buf_inv_cells++;
        module_info_[mod->getId()].total_buf_inv_cell_area += area(dbinst->getMaster());
      }
    }
  }

  int pin_count = 0;
  InstancePinIterator* pin_iter = _network->pinIterator(inst);
  while (pin_iter->hasNext()) {
    Pin* pin = pin_iter->next();
    pin_count++;
  }
  mod->setNumPins(pin_count);
}

vector<Module>::iterator designBrowserKernel::moduleFind(Instance* inst)
{
  vector<Module>::iterator vec_module_it = _module_list.begin();
  while (vec_module_it != _module_list.end()) {
    if (vec_module_it->getInstance() == inst) {
      return vec_module_it;
    }
    vec_module_it++;
  }

  return vec_module_it;
}

void designBrowserKernel::createTreeUtil(Node* node)
{
  size_t num_children = node->_module->getNumberSubmodules();

  if (num_children == 0) {
    return;
  } else {
    vector<Instance*> submodules = node->_module->getSubmodules();
    vector<Instance*>::iterator vec_it;
    for (vec_it = submodules.begin(); vec_it != submodules.end(); vec_it++) {
      vector<Module>::iterator vec_module_it = moduleFind(*vec_it);
      Node* child = createNode(&(*vec_module_it));
      child->_parent = node;
      node->_children.push_back(child);
      createTreeUtil(node->_children[(node->_children).size() - 1]);
    }
  }
}

int designBrowserKernel::specifyTotalStdCellsUtil(Node* node)
{
  int value = 0;
  vector<Node*>::iterator vec_it;
  for (vec_it = (node->_children).begin(); vec_it != (node->_children).end();
       vec_it++) {
    value = value + specifyTotalStdCellsUtil(*vec_it);
  }
  value = value + node->_module->getNumberStdCells();
  node->_module->specifyTotalStdCells(value);
  return value;
}

int designBrowserKernel::specifyTotalMacrosUtil(Node* node)
{
  int value = 0;
  vector<Node*>::iterator vec_it;
  for (vec_it = (node->_children).begin(); vec_it != (node->_children).end();
       vec_it++) {
    value = value + specifyTotalMacrosUtil(*vec_it);
  }
  value = value + node->_module->getNumberMacros();
  node->_module->specifyTotalMacros(value);
  return value;
}

int designBrowserKernel::specifyTotalCombCellPinsUtil(Node* node)
{
  int value = 0;
  vector<Node*>::iterator vec_it;
  for (vec_it = (node->_children).begin(); vec_it != (node->_children).end();
       vec_it++) {
    value = value + specifyTotalCombCellPinsUtil(*vec_it);
  }

  vector<Instance*>::iterator vec_inst_it;
  vector<Instance*> cells = node->_module->getStdCells();

  for (vec_inst_it = cells.begin(); vec_inst_it != cells.end(); vec_inst_it++) {
    Cell* cell = _network->cell(*vec_inst_it);
    LibertyCell* liberty_cell = _network->libertyCell(cell);
    bool is_seq = liberty_cell->hasSequentials();

    if (!is_seq) {
      int term_count = 0;
      LibertyCellPortIterator port_iter(liberty_cell);
      while (port_iter.hasNext()) {
        LibertyPort* port = port_iter.next();
        term_count++;
      }
      value = value + term_count;
    }
  }

  node->_module->specifyTotalCombCellPins(value);
  return value;
}

int designBrowserKernel::specifyTotalCombSeqCellsUtil(Node* node)
{
  int value = 0;
  vector<Node*>::iterator vec_it;
  for (vec_it = (node->_children).begin(); vec_it != (node->_children).end();
       vec_it++) {
    value = value + specifyTotalCombSeqCellsUtil(*vec_it);
  }

  vector<Instance*>::iterator vec_inst_it;
  vector<Instance*> cells = node->_module->getStdCells();
  for (vec_inst_it = cells.begin(); vec_inst_it != cells.end(); vec_inst_it++) {
    Cell* cell = _network->cell(*vec_inst_it);
    LibertyCell* liberty_cell = _network->libertyCell(cell);
    const char* cell_name = _network->name(cell);
    dbMaster* master = _db->findMaster(cell_name);
    bool is_seq = liberty_cell->hasSequentials();
    if (!is_seq) {
      value++;
    }
  }

  node->_module->specifyTotalCombCells(value);
  node->_module->specifyTotalSeqCells(node->_module->getTotalStdCells()
                                      - value);
  return value;
}

int designBrowserKernel::specifyTotalBufInvCellsUtil(Node* node)
{
  int value = 0;
  vector<Node*>::iterator vec_it;
  for (vec_it = (node->_children).begin(); vec_it != (node->_children).end();
       vec_it++) {
    value = value + specifyTotalBufInvCellsUtil(*vec_it);
  }

  vector<Instance*>::iterator vec_inst_it;
  vector<Instance*> cells = node->_module->getStdCells();
  for (vec_inst_it = cells.begin(); vec_inst_it != cells.end(); vec_inst_it++) {
    Cell* cell = _network->cell(*vec_inst_it);
    LibertyCell* liberty_cell = _network->libertyCell(cell);
    bool is_buf = liberty_cell->isBuffer();
    bool is_inv = liberty_cell->isInverter();
    if (is_buf || is_inv) {
      value++;
    }
  }

  node->_module->specifyTotalBufInvCells(value);
  return value;
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

double designBrowserKernel::area(dbMaster* master)
{
  return dbuToMicrons(master->getWidth()) * dbuToMicrons(master->getHeight());
}
////////////////////////////////////////////////////////////////////////////////

void designBrowserKernel::addTotalMapCellsUtil(Node* node, Module* module)
{
  vector<Instance*>::iterator vec_inst_it;

  vector<Instance*> cells = node->_module->getStdCells();

  for (vec_inst_it = cells.begin(); vec_inst_it != cells.end(); vec_inst_it++) {
    module->addTotalMapCell(*vec_inst_it);
  }

  vector<Instance*> macros = node->_module->getMacros();

  for (vec_inst_it = macros.begin(); vec_inst_it != macros.end();
       vec_inst_it++) {
    module->addTotalMapCell(*vec_inst_it);
  }

  vector<Node*>::iterator vec_it;

  for (vec_it = (node->_children).begin(); vec_it != (node->_children).end();
       vec_it++) {
    addTotalMapCellsUtil(*vec_it, module);
  }
}

double designBrowserKernel::specifyTotalAreaUtil(Node* node)
{
  double value = 0.0;
  vector<Node*>::iterator vec_it;
  for (vec_it = (node->_children).begin(); vec_it != (node->_children).end();
       vec_it++) {
    value = value + specifyTotalAreaUtil(*vec_it);
  }

  vector<Instance*>::iterator vec_inst_it;

  vector<Instance*> cells = node->_module->getStdCells();

  for (vec_inst_it = cells.begin(); vec_inst_it != cells.end(); vec_inst_it++) {
    Cell* cell = _network->cell(*vec_inst_it);
    const char* cell_name = _network->name(cell);
    dbMaster* master = _db->findMaster(cell_name);
    value = value + area(master);
  }

  vector<Instance*> macros = node->_module->getMacros();

  for (vec_inst_it = macros.begin(); vec_inst_it != macros.end();
       vec_inst_it++) {
    Cell* cell = _network->cell(*vec_inst_it);
    const char* cell_name = _network->name(cell);
    dbMaster* master = _db->findMaster(cell_name);
    value = value + area(master);
  }

  node->_module->specifyTotalArea(value);
  return value;
}

double designBrowserKernel::specifyTotalStdCellAreaUtil(Node* node)
{
  double value = 0.0;
  vector<Node*>::iterator vec_it;
  for (vec_it = (node->_children).begin(); vec_it != (node->_children).end();
       vec_it++) {
    value = value + specifyTotalStdCellAreaUtil(*vec_it);
  }

  vector<Instance*>::iterator vec_inst_it;
  vector<Instance*> cells = node->_module->getStdCells();
  double std_cell_area = 0.0;
  for (vec_inst_it = cells.begin(); vec_inst_it != cells.end(); vec_inst_it++) {
    Cell* cell = _network->cell(*vec_inst_it);
    const char* cell_name = _network->name(cell);
    dbMaster* master = _db->findMaster(cell_name);
    std_cell_area = std_cell_area + area(master);
  }

  value = value + std_cell_area;
  node->_module->specifyStdCellArea(std_cell_area);
  node->_module->specifyTotalStdCellArea(value);
  return value;
}

void designBrowserKernel::reportMacroUtil(Node* node, ofstream& file)
{
  vector<Node*>::iterator vec_it;
  for (vec_it = (node->_children).begin(); vec_it != (node->_children).end();
       vec_it++) {
    reportMacroUtil(*vec_it, file);
  }

  vector<Instance*>::iterator vec_inst_it;
  vector<Instance*> macros = node->_module->getMacros();

  if (macros.size() != 0) {
    for (vec_inst_it = macros.begin(); vec_inst_it != macros.end();
         vec_inst_it++) {
      Cell* cell = _network->cell(*vec_inst_it);
      LibertyCell* liberty_cell = _network->libertyCell(cell);
      const char* cell_name = _network->name(cell);
      dbMaster* master = _db->findMaster(cell_name);
      const char* instance_name = _network->pathName(*vec_inst_it);
      file << cell_name << "," << instance_name << ","
           << dbuToMicrons(master->getWidth());
      file << "," << dbuToMicrons(master->getHeight()) << endl;
    }
  }
}

double designBrowserKernel::specifyTotalCombSeqCellAreaUtil(Node* node)
{
  double value = 0.0;
  vector<Node*>::iterator vec_it;
  for (vec_it = (node->_children).begin(); vec_it != (node->_children).end();
       vec_it++) {
    value = value + specifyTotalCombSeqCellAreaUtil(*vec_it);
  }

  vector<Instance*>::iterator vec_inst_it;
  vector<Instance*> cells = node->_module->getStdCells();
  for (vec_inst_it = cells.begin(); vec_inst_it != cells.end(); vec_inst_it++) {
    Cell* cell = _network->cell(*vec_inst_it);
    LibertyCell* liberty_cell = _network->libertyCell(cell);
    const char* cell_name = _network->name(cell);
    dbMaster* master = _db->findMaster(cell_name);
    if (liberty_cell->hasSequentials()) {
      value = value + area(master);
    }
  }

  node->_module->specifyTotalCombCellArea(value);
  node->_module->specifyTotalSeqCellArea(node->_module->getTotalStdCellArea()
                                         - value);
  return value;
}

double designBrowserKernel::specifyTotalBufInvCellAreaUtil(Node* node)
{
  double value = 0.0;
  vector<Node*>::iterator vec_it;
  for (vec_it = (node->_children).begin(); vec_it != (node->_children).end();
       vec_it++) {
    value = value + specifyTotalBufInvCellAreaUtil(*vec_it);
  }

  vector<Instance*>::iterator vec_inst_it;
  vector<Instance*> cells = node->_module->getStdCells();
  for (vec_inst_it = cells.begin(); vec_inst_it != cells.end(); vec_inst_it++) {
    Cell* cell = _network->cell(*vec_inst_it);
    LibertyCell* liberty_cell = _network->libertyCell(cell);
    bool is_buf = liberty_cell->isBuffer();
    bool is_inv = liberty_cell->isInverter();
    const char* name = _network->name(cell);
    if (is_buf || is_inv) {
      dbMaster* master = _db->findMaster(name);
      value = value + area(master);
    }
  }
  node->_module->specifyTotalBufInvCellArea(value);
  return value;
}

double designBrowserKernel::specifyTotalMacroAreaUtil(Node* node)
{
  double value = 0.0;
  vector<Node*>::iterator vec_it;
  for (vec_it = (node->_children).begin(); vec_it != (node->_children).end();
       vec_it++) {
    value = value + specifyTotalMacroAreaUtil(*vec_it);
  }

  vector<Instance*>::iterator vec_inst_it;
  vector<Instance*> macros = node->_module->getMacros();
  double macro_area = 0.0;
  for (vec_inst_it = macros.begin(); vec_inst_it != macros.end();
       vec_inst_it++) {
    Cell* cell = _network->cell(*vec_inst_it);
    const char* cell_name = _network->name(cell);
    dbMaster* master = _db->findMaster(cell_name);
    macro_area = macro_area + area(master);
  }

  value = value + macro_area;
  node->_module->specifyTotalMacroArea(value);
  node->_module->specifyMacroArea(macro_area);
  return value;
}

void designBrowserKernel::createTree()
{
  Module* top_module = &(_module_list[0]);
  _root = createNode(top_module);
  createTreeUtil(_root);
  specifyTotalStdCellsUtil(_root);
  specifyTotalMacrosUtil(_root);
  specifyTotalCombSeqCellsUtil(_root);
  specifyTotalBufInvCellsUtil(_root);
  specifyTotalAreaUtil(_root);
  specifyTotalStdCellAreaUtil(_root);
  specifyTotalMacroAreaUtil(_root);
  specifyTotalCombSeqCellAreaUtil(_root);
  specifyTotalBufInvCellAreaUtil(_root);
}

void designBrowserKernel::linkDesignBrowserKernel(dbVerilogNetwork* network,
                                                  dbDatabase* db)
{
  init(network, db);
  linkDesignBrowserKernelUtil(network->topInstance());
  createDBModule(network->topInstance());
  createTree();
  updateModuleInfo(db->getChip()->getBlock()->getTopModule());
}

void designBrowserKernel::traverseDepthUtil(Node* node,
                                            int value,
                                            ofstream& file)
{
  if (value > _level) {
    return;
  }
  file << setw(10) << to_string(value++) + " |";
  node->_module->printHierInfo(file);
  file << endl;
  vector<Node*>::iterator vec_it;
  for (vec_it = (node->_children).begin(); vec_it != (node->_children).end();
       vec_it++) {
    traverseDepthUtil(*vec_it, value, file);
  }
}

Node* designBrowserKernel::findNode(Node* node, string name)
{
  queue<Node*> node_queue;
  node_queue.push(node);
  while (!node_queue.empty()) {
    Node* current_node = node_queue.front();
    node_queue.pop();
    string module_name = current_node->_module->getName();
    if (module_name == name) {
      return current_node;
    }

    vector<Node*>::iterator vec_it;
    for (vec_it = (current_node->_children).begin();
         vec_it != (current_node->_children).end();
         vec_it++) {
      node_queue.push(*vec_it);
    }
  }

  cout << "Cannot find such a module" << endl;
}

void designBrowserKernel::traverseDesign(string name, ofstream& file)
{
  Node* node = findNode(_root, name);
  traverseDepthUtil(node, 0, file);
}

void designBrowserKernel::reportLogicAreaKeyTermCountUtil(Node* node,
                                                          ofstream& file)
{
  map<LibertyCell*, int> total_map_cells = node->_module->getTotalMapCells();
  map<LibertyCell*, int>::iterator map_it;
  vector<pair<LibertyCell*, int>> sorted_cells;
  for (map_it = total_map_cells.begin(); map_it != total_map_cells.end();
       map_it++) {
    LibertyCell* liberty_cell = map_it->first;
    int term_count = 0;
    LibertyCellPortIterator port_iter(liberty_cell);
    while (port_iter.hasNext()) {
      LibertyPort* port = port_iter.next();
      term_count++;
    }
    sorted_cells.push_back(pair<LibertyCell*, int>(liberty_cell, term_count));
  }
  sort(sorted_cells.begin(), sorted_cells.end(), cmpPair);

  file << endl;
  file << endl;
  file << string(120, '*') << endl;
  file << left << setw(40) << "Cell name" << setw(20) << "# Pins" << setw(20)
       << "Number" << setw(20) << "Area" << setw(20) << "Total Area" << endl;

  vector<pair<LibertyCell*, int>>::iterator vec_it;
  for (vec_it = sorted_cells.begin(); vec_it != sorted_cells.end(); vec_it++) {
    LibertyCell* liberty_cell = vec_it->first;
    int term_count = vec_it->second;
    float area = liberty_cell->area();
    const char* cell_name = liberty_cell->name();
    int cell_count = total_map_cells[liberty_cell];

    file << left << setw(40) << cell_name << setw(20) << term_count << setw(20)
         << cell_count << setw(20) << numberToString(area);
    file << setw(20) << numberToString(area * cell_count) << endl;
  }
}

void designBrowserKernel::reportLogicAreaKeyCellCountUtil(Node* node,
                                                          ofstream& file)
{
  map<LibertyCell*, int> total_map_cells = node->_module->getTotalMapCells();
  map<LibertyCell*, int>::iterator map_it;
  vector<pair<LibertyCell*, int>> sorted_cells;
  for (map_it = total_map_cells.begin(); map_it != total_map_cells.end();
       map_it++) {
    sorted_cells.push_back(
        pair<LibertyCell*, int>(map_it->first, map_it->second));
  }
  sort(sorted_cells.begin(), sorted_cells.end(), cmpPair);

  file << endl;
  file << endl;
  file << string(120, '*') << endl;
  file << left << setw(40) << "Cell name" << setw(20) << "# Pins" << setw(20)
       << "Number" << setw(20) << "Area" << setw(20) << "Total Area" << endl;

  vector<pair<LibertyCell*, int>>::iterator vec_it;
  for (vec_it = sorted_cells.begin(); vec_it != sorted_cells.end(); vec_it++) {
    LibertyCell* liberty_cell = vec_it->first;

    int term_count = 0;
    LibertyCellPortIterator port_iter(liberty_cell);
    while (port_iter.hasNext()) {
      LibertyPort* port = port_iter.next();
      term_count++;
    }
    float area = liberty_cell->area();
    const char* cell_name = liberty_cell->name();
    int cell_count = vec_it->second;

    file << left << setw(40) << cell_name << setw(20) << term_count << setw(20)
         << cell_count << setw(20) << numberToString(area);
    file << setw(20) << numberToString(area * cell_count) << endl;
  }
}

void printLogicalInfo(std::ofstream& file, odb::dbModule* mod, int level, ModuleData data)
{
  file << "\"module_name\" : \"" <<mod->getName() << "\"," << std::endl;
  file << "\"level\" : " << level << "," << std::endl;
  file << "\"pins\" : " << mod->getNumPins() << "," << std::endl;
  file << "\"total_insts\" : " << data.getTotalStdCells() << "," << std::endl;
  file << "\"total_macros\" : " << data.total_macros << "," << std::endl;
  file << "\"total_area\" : "
        << numberToString(data.getTotalArea()) << ","
        << std::endl;
  file << "\"local_insts\" : " << data.std_cells << "," << std::endl;
  file << "\"local_macros\" : " << data.macros << "," << std::endl;
}
void designBrowserKernel::reportDesignFileUtil(odb::dbModule* mod,
                                               int level,
                                               ofstream& file)
{
  level = level + 1;
  file << "{" << endl;
  file << "\"local_instance_name\" : \"" <<  mod->getModInst()->getName()
       << "\"," << endl;
  file << "\"instance_name\" : \"" << mod->getModInst()->getHierarchalName() << "\","
       << endl;
  file << "\"module\" : {" << endl;
  printLogicalInfo(file, mod, level, module_info_[mod->getId()]);
  file << "\"module_instances\" : [" << endl;
  size_t i = 0;
  uint childrenSz = mod->getChildren().size();
  if (childrenSz > 0) {
    uint cnt = 0;
    for(auto child : mod->getChildren())
    {
      reportDesignFileUtil(child->getMaster(), level, file);
      if(++cnt < childrenSz)
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
  Node* node = findNode(_root, name);
  node->_module->printAreaInfo(file);

  if (detailed) {
    if (node->_module->getDetailedFlag() == true) {
      ;
    } else {
      node->_module->specifyDetailedFlag();
      addTotalMapCellsUtil(node, node->_module);
      specifyTotalCombCellPinsUtil(node);
      node->_module->calculateAverageCombFanouts();
    }

    if (string(key) == "cell_count") {
      reportLogicAreaKeyCellCountUtil(node, file);
    } else {
      reportLogicAreaKeyTermCountUtil(node, file);
    }
    file << "The average pins of combinational cell is  "
         << node->_module->getTotalCombCellPins() * 1.0
                / node->_module->getTotalCombCells()
         << endl;
    file << "The number of combinational nets:   "
         << node->_module->getTotalCombNets() << endl;
    file << "The average fanout of combinational nets:  "
         << node->_module->getAverageCombFanouts() << endl;
  }
}

void designBrowserKernel::reportNet(string name, ofstream& file)
{
  Node* node = findNode(_root, name);
  node->_module->printNetInfo(file);
}

void designBrowserKernel::reportMacro(string name, ofstream& file)
{
  Node* node = findNode(_root, name);
  reportMacroUtil(node, file);
}

void designBrowserKernel::reportDesignFile(ofstream& file)
{
  file << "{" << endl;
  file << "\"logical_hierarchy\" : {" << endl;
  auto mod = _db->getChip()->getBlock()->getTopModule();
  printLogicalInfo(file, mod, 0, module_info_[mod->getId()]);
  file << "\"module_instances\" : [" << endl;
  uint childrenSz = mod->getChildren().size();
  if(childrenSz > 0)
  {
    uint cnt = 0;
    for(auto child : mod->getChildren())
    {
      reportDesignFileUtil(child->getMaster(), 0, file);
      if(++cnt < childrenSz)
        file << "," << endl;
    }
  }

  file << "]" << endl;
  file << "}" << endl;
  file << "}" << endl;
}

designBrowserKernel* makeDesignBrowser(utl::Logger* logger)
{
  return new designBrowserKernel(logger);
}

void deleteDesignBrowser(designBrowserKernel* browser)
{
  delete browser;
}

void dbLinkDesignBrowser(designBrowserKernel* browser,
                         dbVerilogNetwork* network,
                         dbDatabase* db)
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

void dbReportLogicNet(designBrowserKernel* browser,
                      const char* name,
                      const char* file_name)
{
  ofstream output_file;
  output_file.open(file_name);
  browser->reportNet(string(name), output_file);
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
