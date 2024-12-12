#include "sqliteDB.h"
void sqliteDB::executeSqlQuery(const std::string& query) const
{
  char* errmsg = nullptr;
  int rc = sqlite3_exec(db_, query.c_str(), nullptr, nullptr, &errmsg);
  if (rc != SQLITE_OK) {
    logger_->error(
        utl::DRT, 8000, "Faliled to perform query with msg {}", errmsg);
    sqlite3_free(errmsg);
  }
}
sqliteDB::sqliteDB(utl::Logger* logger,
                   const std::string& db_path,
                   const std::string& design_name)
    : logger_(logger), design_name_(design_name)
{
  sqlite3_open_v2(
      db_path.c_str(),
      &db_,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
      nullptr);

  // Enable WAL mode for better concurrent write performance
  sqlite3_exec(db_, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
  const std::string iterations_table
      = "CREATE TABLE IF NOT EXISTS Iterations ("
        "design_name TEXT,"
        "iteration INTEGER,"
        "drvs INTEGER,"
        "total_workers INTEGER,"
        "active_workers INTEGER,"
        "violating_gcells INTEGER,"
        "violating_nets INTEGER,"
        "flow_type TEXT,"
        "PRIMARY KEY (design_name, iteration));";
  executeSqlQuery(iterations_table);

  const std::string workers_table
      = "CREATE TABLE IF NOT EXISTS Workers ("
        "id INTEGER PRIMARY KEY,"
        "design_name TEXT,"
        "iteration INTEGER,"
        "worker_id INTEGER,"
        "drv_cost_mult INTEGER,"
        "marker_cost_mult INTEGER,"
        "init_drvs INTEGER,"
        "end_drvs INTEGER,"
        "horizontal_offset TEXT,"
        "vertical_offset TEXT,"
        "chosen INTEGER);";
  executeSqlQuery(workers_table);
  const std::string designs_table
      = "CREATE TABLE IF NOT EXISTS Designs ("
        "design_name TEXT,"
        "area INTEGER,"
        "nets INTEGER,"
        "gcells INTEGER,"
        "PRIMARY KEY (design_name));";
  executeSqlQuery(designs_table);
  executeSqlQuery(fmt::format(
      "DELETE FROM Iterations WHERE design_name = '{}';", design_name_));
  executeSqlQuery(fmt::format("DELETE FROM Workers WHERE design_name = '{}';",
                              design_name_));
  executeSqlQuery(fmt::format("DELETE FROM Designs WHERE design_name = '{}';",
                              design_name_));
}
void sqliteDB::insertIterationEntry(const int iteration,
                                    const int drvs,
                                    const int tot_workers,
                                    const int active_workers,
                                    const int violating_gcells,
                                    const int violating_nets,
                                    const std::string& flow_type) const
{
  const std::string sql
      = "INSERT INTO Iterations (design_name, iteration, drvs, total_workers, "
        "active_workers, violating_gcells, violating_nets, flow_type) VALUES "
        "(?, ?,?,?,?,?,?,?);";
  sqlite3_stmt* stmt;
  sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
  sqlite3_bind_text(stmt, 1, design_name_.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_int(stmt, 2, iteration);
  sqlite3_bind_int(stmt, 3, drvs);
  sqlite3_bind_int(stmt, 4, tot_workers);
  sqlite3_bind_int(stmt, 5, active_workers);
  sqlite3_bind_int(stmt, 6, violating_gcells);
  sqlite3_bind_int(stmt, 7, violating_nets);
  sqlite3_bind_text(stmt, 8, flow_type.c_str(), -1, SQLITE_STATIC);
  if (sqlite3_step(stmt) != SQLITE_DONE) {
    logger_->error(utl::DRT,
                   8002,
                   "Failed to insert data with msg {}",
                   sqlite3_errmsg(db_));
  }
  sqlite3_finalize(stmt);
}
void sqliteDB::insertWorkerEntry(const int iteration,
                                 const int worker_id,
                                 const int drv_cost_mult,
                                 const int marker_cost_mult,
                                 const int init_drvs,
                                 const int end_drvs,
                                 const bool chosen,
                                 const std::string& horizontal_offset,
                                 const std::string& vertical_offset) const
{
  const std::string sql
      = "INSERT INTO Workers (design_name, iteration, worker_id, "
        "drv_cost_mult, "
        "marker_cost_mult, init_drvs, end_drvs, chosen, horizontal_offset, "
        "vertical_offset) VALUES "
        "(?, ?,?,?,?,?,?,?,?,?);";
  sqlite3_stmt* stmt;
  sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
  sqlite3_bind_text(stmt, 1, design_name_.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_int(stmt, 2, iteration);
  sqlite3_bind_int(stmt, 3, worker_id);
  sqlite3_bind_int(stmt, 4, drv_cost_mult);
  sqlite3_bind_int(stmt, 5, marker_cost_mult);
  sqlite3_bind_int(stmt, 6, init_drvs);
  sqlite3_bind_int(stmt, 7, end_drvs);
  sqlite3_bind_int(stmt, 8, chosen ? 1 : 0);
  sqlite3_bind_text(stmt, 9, horizontal_offset.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 10, vertical_offset.c_str(), -1, SQLITE_STATIC);
  if (sqlite3_step(stmt) != SQLITE_DONE) {
    logger_->error(utl::DRT,
                   8003,
                   "Failed to insert data with msg {}",
                   sqlite3_errmsg(db_));
  }
  sqlite3_finalize(stmt);
}
void sqliteDB::insertDesignEntry(const int area,
                                 const int gcell_cnt,
                                 const int net_cnt) const
{
  const std::string sql
      = "INSERT INTO Designs (design_name, area, gcells, nets) VALUES "
        "(?, ?,?,?);";
  sqlite3_stmt* stmt;
  sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
  sqlite3_bind_text(stmt, 1, design_name_.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_int(stmt, 2, area);
  sqlite3_bind_int(stmt, 3, gcell_cnt);
  sqlite3_bind_int(stmt, 4, net_cnt);
  if (sqlite3_step(stmt) != SQLITE_DONE) {
    logger_->error(utl::DRT,
                   8004,
                   "Failed to insert data with msg {}",
                   sqlite3_errmsg(db_));
  }
  sqlite3_finalize(stmt);
}

sqliteDB::~sqliteDB()
{
  sqlite3_close(db_);
}
