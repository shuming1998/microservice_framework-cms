#include "cmysql.h"
#include <mysql.h>
#include <iostream>
#include <string.h>

namespace cmysql {


bool CMysql::init() {
  this->close();
  std::cout << "CMysql::init()\n";
  // 新创建一个 MYSQL 对象
  mysql_ = mysql_init(0);
  if (!mysql_) {
    std::cerr << "mysql_init failed!\n";
    return false;
  }

  return true;
}


void CMysql::close() {
  std::cout << "CMysql::close()\n";
  freeResult();

  if (mysql_) {
    mysql_close((MYSQL *)mysql_);
    mysql_ = nullptr;
  }
}

bool CMysql::connect(const char *host,
                     const char *user,
                     const char *password,
                     const char *db,
                     unsigned short port,
                     //const char *unixSock,
                     unsigned long flag) {
  if (!mysql_ && !init()) {
    std::cerr << "CMysql::connect failed: mysql not init!\n";
    return false;
  }

  if (!mysql_real_connect((MYSQL *)mysql_, host, user, password, db, port, 0, flag)) {
    std::cerr << "mysql_real_connect failed: " << mysql_error(mysql_) << '\n';
    return false;
  }

  std::cout << "CMysql::connect success!\n";
  return true;
}

bool CMysql::query(const char *sql, unsigned long sqlLen) {
  if (!mysql_) {
    std::cerr << "CMysql::query failed: mysql_ not init!\n";
    return false;
  }
  if (!sql) {
    std::cerr << "sql is nullptr!\n";
    return false;
  }

  if (sqlLen <= 0) {
    sqlLen = (unsigned long)strlen(sql);
  }
  if (sqlLen <= 0) {
    std::cerr << "sql is empty or error format!\n";
    return false;
  }

  // 执行 sql 语句
  int res = mysql_real_query((MYSQL *)mysql_, sql, sqlLen);
  if (res != 0) {
    std::cerr << "mysql_real_query failed: " << mysql_errno((MYSQL *)mysql_) << '\n';
    return false;
  }

  return true;
}


bool CMysql::option(COption opt, const void *arg) {
  if (!mysql_) {
    std::cerr << "CMysql::option set failed: mysql_ not init!\n";
    return false;
  }

  int res = mysql_options((MYSQL *)mysql_, (mysql_option)opt, arg);
  if (res != 0) {
    std::cerr << "mysql_options set failed: " << mysql_error((MYSQL *)mysql_) << '\n';
    return false;
  }

  return true;
}

bool CMysql::setConnectTimeout(int sec) {
  return option(CMYSQL_OPT_CONNECT_TIMEOUT, &sec);
}

bool CMysql::setReconnect(bool is) {
  return option(CMYSQL_OPT_RECONNECT, &is);
}

bool CMysql::storeResult() {
  if (!mysql_) {
    std::cerr << "CMysql::storeResult() failed: mysql_ not init!\n";
    return false;
  }

  freeResult();

  result_ = mysql_store_result((MYSQL *)mysql_);
  if (!result_) {
    std::cerr << "mysql_store_result failed: " << mysql_error((MYSQL *)mysql_) << '\n';
    return false;
  }

  return true;
}

bool CMysql::useResult() {
  if (!mysql_) {
    std::cerr << "CMysql::useResult() failed: mysql_ not init!\n";
    return false;
  }

  freeResult();

  result_ = mysql_use_result((MYSQL *)mysql_);
  if (!result_) {
    std::cerr << "mysql_use_result failed: " << mysql_error((MYSQL *)mysql_) << '\n';
    return false;
  }

  return true;
}

void CMysql::freeResult() {
  if (result_) {
    mysql_free_result((MYSQL_RES *)result_);
    result_ = nullptr;
  }
}

std::vector<CData> CMysql::fetchRow() {
  std::vector<CData> res;
  if (!result_) {
    return res;
  }

  MYSQL_ROW row = mysql_fetch_row(result_);
  if (!row) {
    return res;
  }

  // 获取列数
  int cols = mysql_num_fields(result_);
  unsigned long *lens = mysql_fetch_lengths(result_);
  for (int i = 0; i < cols; ++i) {
    CData data;
    data.data = row[i];
    data.size = lens[i];
    // 获取列的信息
    auto field = mysql_fetch_field_direct(result_, i);
    data.type = (CFieldType)field->type;
    res.push_back(data);
  }

  return res;
}

std::string CMysql::getInsertSql(MData kv, std::string table) {
  std::string insertSql = "";
  if (kv.empty() || table.empty()) {
    return "";
  }

  insertSql = "insert into `";
  insertSql += table;
  insertSql += "`";

  // 迭代遍历 map
  std::string keys = "";
  std::string values = "";
  for (auto it = kv.begin(); it != kv.end(); ++it) {
    keys += "`";
    keys += it->first;
    keys += "`,";


    values += "'";
    values += it->second.data;
    values += "',";
  }
  // 去除结尾多余的逗号
  //keys.pop_back();
  //values.pop_back();
  keys[keys.size() - 1] = ' ';
  values[values.size() - 1] = ' ';

  insertSql += "(";
  insertSql += keys;
  insertSql += ")values(";
  insertSql += values;
  insertSql += ")";
  return insertSql;
}

bool CMysql::insert(MData kv, std::string table) {
  if (!mysql_) {
    std::cerr << "CMysql::insert failed: mysql_ not init!\n";
    return false;
  }

  std::string sql = getInsertSql(kv, table);
  if (sql.empty()) {
    return false;
  }

  if (!query(sql.c_str())) {
    return false;
  }

  int rows = mysql_affected_rows((MYSQL *)mysql_);
  if (rows <= 0) {
    return false;
  }

  return true;
}

bool CMysql::insertBin(MData kv, std::string table) {
  std::string sql = "";
  if (kv.empty() || table.empty() || !mysql_) {
    return false;
  }

  sql = "insert into `";
  sql += table;
  sql += "`";

  // 迭代遍历 map
  std::string keys = "";
  std::string values = "";
  // 绑定字段
  MYSQL_BIND bind[256] = { 0 };
  int i = 0;
  for (auto it = kv.begin(); it != kv.end(); ++it) {
    keys += "`";
    keys += it->first;
    keys += "`,";

    values += "?,";
    bind[i].buffer = (char *)it->second.data;
    bind[i].buffer_length = it->second.size;
    bind[i].buffer_type = (enum_field_types)it->second.type;
    i++;
  }
  // 去除结尾多余的逗号
  //keys.pop_back();
  //values.pop_back();
  keys[keys.size() - 1] = ' ';
  values[values.size() - 1] = ' ';

  sql += "(";
  sql += keys;
  sql += ")values(";
  sql += values;
  sql += ")";

  // 预处理 sql 语句
  MYSQL_STMT *stmt = mysql_stmt_init((MYSQL *)mysql_);
  if (!stmt) {
    std::cerr << "mysql_stmt_init failed: " << mysql_stmt_error(stmt) << '\n';
    return false;
  }
  if (mysql_stmt_prepare(stmt, sql.c_str(), sql.length()) != 0) {
    //std::cout << "======" << mysql_stmt_prepare(stmt, insertSql.c_str(), insertSql.length()) << '\n';
    std::cerr << "mysql_stmt_prepare failed: " << mysql_stmt_error(stmt) << '\n';
    mysql_stmt_close(stmt);
    return false;
  }

  if (mysql_stmt_bind_param(stmt, bind) != 0) {
    std::cerr << "mysql_stmt_bind_param failed: " << mysql_stmt_error(stmt) << '\n';
    mysql_stmt_close(stmt);
    return false;
  }

  // 执行 sql 语句
  if (mysql_stmt_execute(stmt) != 0) {
    std::cerr << "mysql_stmt_execute failed: " << mysql_stmt_error(stmt) << '\n';
    mysql_stmt_close(stmt);
    return false;
  }

  // 清理 stmt
  mysql_stmt_close(stmt);

  return true;
}

std::string CMysql::getUpdateSql(MData kv, std::string table, std::string where) {
  // update db set xx='xx',xx=xx where xx=xx
  std::string updateSql = "";
  if (kv.empty() || table.empty()) {
    return "";
  }
  
  updateSql = "update `";
  updateSql += table;
  updateSql += "` set ";
  for (auto it = kv.begin(); it != kv.end(); ++it) {
    updateSql += "`";
    updateSql += it->first;
    updateSql += "`='";
    updateSql += it->second.data;
    updateSql += "',";
  }
  // 去除结尾多余的逗号
  updateSql[updateSql.size() - 1] = ' ';
  updateSql += " ";
  updateSql += where;


  return updateSql;
}

int CMysql::update(MData kv, std::string table, std::string where) {
  if (!mysql_) {
    return -1;
  }

  std::string sql = getUpdateSql(kv, table, where);
  if (sql.empty()) {
    return -1;
  }
  if (!query(sql.c_str())) {
    return -1;
  }

  return mysql_affected_rows(mysql_);
}

int CMysql::updateBin(MData kv, std::string table, std::string where) {
  if (!mysql_ || kv.empty() || table.empty()) {
    return -1;
  }

  std::string sql = "";
  sql = "update `";
  sql += table;
  sql += "` set ";
  MYSQL_BIND bind[256] = { 0 };
  int i = 0;
  for (auto it = kv.begin(); it != kv.end(); ++it) {
    sql += "`";
    sql += it->first;
    sql += "`=?,";
    bind[i].buffer = (char *)it->second.data;
    bind[i].buffer_length = it->second.size;
    bind[i].buffer_type = (enum_field_types)it->second.type;
    ++i;
  }

  // 去除结尾多余的逗号
  sql[sql.size() - 1] = ' ';
  sql += " ";
  sql += where;

  // 预处理 sql 语句的上下文
  MYSQL_STMT *stmt = mysql_stmt_init((MYSQL *)mysql_);
  if (!stmt) {
    std::cerr << "mysql_stmt_init failed: " << mysql_stmt_error(stmt) << '\n';
    return -1;
  }

  if (mysql_stmt_prepare(stmt, sql.c_str(), sql.length()) != 0) {
    std::cerr << "mysql_stmt_prepare failed: " << mysql_stmt_error(stmt) << '\n';
    mysql_stmt_close(stmt);
    return -1;
  }

  if (mysql_stmt_bind_param(stmt, bind) != 0) {
    std::cerr << "mysql_stmt_bind_param failed: " << mysql_stmt_error(stmt) << '\n';
    mysql_stmt_close(stmt);
    return -1;
  }

  // 执行 sql 语句
  if (mysql_stmt_execute(stmt) != 0) {
    std::cerr << "mysql_stmt_execute failed: " << mysql_stmt_error(stmt) << '\n';
    mysql_stmt_close(stmt);
    return -1;
  }

  // 清理 stmt 之前获取行数
  int rows = mysql_stmt_affected_rows(stmt);
  // 清理 stmt
  mysql_stmt_close(stmt);
  return rows;
}

bool CMysql::startTransaction() {
  return query("set autocommit=0");
}

bool CMysql::stopTransaction() {
  return query("set autocommit=1");
}

bool CMysql::commit() {
  return query("commit");
}

bool CMysql::rollback() {
  return query("rollback");
}

CRows CMysql::getResult(const char *sql) {
  freeResult();
  CRows rows;
  if (!query(sql)) {
    return rows;
  }
  if (!storeResult()) {
    return rows;
  }
  for (;;) {
    auto row = fetchRow();
    if (row.empty()) {
      break;
    }
    rows.push_back(row);
  }

  return rows;
}

} // namespace cmysql