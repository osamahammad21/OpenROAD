#pragma once
#include <sqlite3.h>

#include <string>

#include "utl/Logger.h"
class sqliteDB
{
 public:
  sqliteDB(utl::Logger* logger,
           const std::string& db_path,
           const std::string& design_name);
  void insertIterationEntry(int iteration,
                            int drvs,
                            int tot_workers,
                            int active_workers,
                            int violating_gcells,
                            int violating_nets,
                            const std::string& flow_type) const;
  void insertWorkerEntry(int iteration,
                         int worker_id,
                         int drv_cost_mult,
                         int marker_cost_mult,
                         int init_drvs,
                         int end_drvs,
                         bool chosen,
                         const std::string& horizontal_offset,
                         const std::string& vertical_offset) const;
  void insertDesignEntry(int area, int gcell_cnt, int net_cnt) const;
  ~sqliteDB();

 private:
  utl::Logger* logger_{nullptr};
  sqlite3* db_{nullptr};
  std::string design_name_;

  void executeSqlQuery(const std::string& query) const;
};