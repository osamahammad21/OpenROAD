#pragma once

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <vector>

#include "db_sta/dbReadVerilog.hh"
#include "opendb/db.h"
#include "sta/Liberty.hh"

namespace utl {
class Logger;
}

namespace ord {

//******************************************************
//**** Utility Function:  Convert number to string
//*****************************************************
std::string numberToString(double a);

template <class K, class V>
bool cmpPair(std::pair<K, V>& a, std::pair<K, V>& b);

//*****************************************************
//***********  struct ModuleData
//*****************************************************
struct ModuleData
{
  int total_comb_cells = 0;
  int total_seq_cells = 0;
  int total_buf_inv_cells = 0;
  int total_macros = 0;
  int total_comb_cell_pins = 0;
  int macros = 0;
  int std_cells = 0;
  int num_pins = 0;
  double total_macro_area = 0.0;
  double std_cell_area = 0.0;
  double macro_area = 0.0;
  double total_comb_cell_area = 0.0;
  double total_seq_cell_area = 0.0;
  double total_buf_inv_cell_area = 0.0;
  std::map<odb::dbMaster*, uint> macros_map;

  bool valid = false;

  int getTotalStdCells() const { return total_comb_cells + total_seq_cells; }
  double getTotalStdCellArea() const
  {
    return total_seq_cell_area + total_comb_cell_area;
  }
  double getTotalArea() const
  {
    return total_macro_area + getTotalStdCellArea();
  }
  void appendData(const ModuleData& data)
  {
    total_comb_cells += data.total_comb_cells;
    total_seq_cells += data.total_seq_cells;
    total_buf_inv_cells += data.total_buf_inv_cells;
    total_macros += data.total_macros;
    total_comb_cell_pins += data.total_comb_cell_pins;
    total_macro_area += data.total_macro_area;
    total_comb_cell_area += data.total_comb_cell_area;
    total_seq_cell_area += data.total_seq_cell_area;
    total_buf_inv_cell_area += data.total_buf_inv_cell_area;
    for (auto& [id, cnt] : data.macros_map)
      macros_map[id] += cnt;
  }
};

//*****************************************************
//***********  Class designBrowserKernel
//*****************************************************
class designBrowserKernel
{
 protected:
  dbVerilogNetwork* _network = nullptr;
  odb::dbDatabase* _db = nullptr;
  utl::Logger* logger_;

  int _level;
  std::map<uint, ModuleData> module_info_;

  void updateModuleInfo(odb::dbModule* mod);

  //////////////////////////////////////////////////////////////////////////////
  // Note:  dbuToMeters, metersToDbu, area are copied from Resizer.cc file
  double dbuToMicrons(int dist) const;

  int micronsToDbu(double dist) const;

  double area(odb::dbMaster* master);
  //////////////////////////////////////////////////////////////////////////////
  void createTree();

  void linkDesignBrowserKernelUtil(sta::Instance* inst);

  void createDBModule(sta::Instance* inst, odb::dbModule* parent = nullptr);

  void traverseDepthUtil(odb::dbModule* mod, int value, std::ofstream& file);

  void reportLogicAreaKeyTermCountUtil(odb::dbModule* mod, std::ofstream& file);

  void reportLogicAreaKeyCellCountUtil(odb::dbModule* mod, std::ofstream& file);

  void reportMacroUtil(odb::dbModule* mod, std::ofstream& file);

  void reportDesignFileUtil(odb::dbModule* mod, int level, std::ofstream& file);

 public:
  designBrowserKernel(utl::Logger* loggerIn) : logger_(loggerIn) {}

  void init(dbVerilogNetwork* network, odb::dbDatabase* db)
  {
    _network = network;
    _db = db;
  }

  void specifyLevel(int level) { _level = level; }

  void linkDesignBrowserKernel(dbVerilogNetwork* network, odb::dbDatabase* db);

  void traverseDesign(std::string name, std::ofstream& file);

  void reportLogicArea(std::string name,
                       std::ofstream& file,
                       bool detailed,
                       const char* key);

  void reportMacro(std::string name, std::ofstream& file);

  void reportDesignFile(std::ofstream& file);

  // Accessor
  dbVerilogNetwork* getNetwork() { return _network; }
};

//***************************************************************************************************
//******** Interface functions
//***************************************************************************************************

designBrowserKernel* makeDesignBrowser(utl::Logger* logger);

void deleteDesignBrowser(designBrowserKernel* browser);

void dbLinkDesignBrowser(designBrowserKernel* browser,
                         dbVerilogNetwork* network,
                         odb::dbDatabase* db);

void dbDesignBrowser(designBrowserKernel* browser,
                     const char* name,
                     const char* file_name,
                     int level = 3);

void dbReportLogicArea(designBrowserKernel* browser,
                       const char* name,
                       const char* file_name,
                       bool detailed,
                       const char* key);

void dbReportMacro(designBrowserKernel* browser,
                   const char* name,
                   const char* file_name);

void dbReportDesignFile(designBrowserKernel* browser, const char* file_name);

}  // namespace ord
