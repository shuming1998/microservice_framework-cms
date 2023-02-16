#include "clogdao.h"
#include "cmysql.h"
#include "clogclient.h"
#include "cmsgcom.pb.h"
#include "ctools.h"
#include <mutex>

static cmysql::CMysql mysql;
static std::mutex logMtx;

bool CLogDAO::init() {
  std::lock_guard<std::mutex> guard(logMtx);
  if (!mysql.init()) {
    LOG_DEBUG("mysql init failed!");
    return false;
  }
  mysql.setReconnect(true);
  mysql.setConnectTimeout(3);

  if (!mysql.inputDBConfig()) {
    LOG_DEBUG("mysql.inputDBConfig() failed!");
    return false;
  }
  return true;
}


bool CLogDAO::install() {
  // 创建日志表
  std::string sql = "";
  sql = "CREATE TABLE IF NOT EXISTS `cms_log` (\
        `id` INT AUTO_INCREMENT, \
        `service_name` VARCHAR(16), \
        `service_ip` VARCHAR(16), \
        `service_port` INT, \
        `log_txt` VARCHAR(4096), \
        `log_time` INT, \
        `log_level` INT, \
        PRIMARY KEY(`id`));";
  std::lock_guard<std::mutex> guard(logMtx);
  if (!mysql.query(sql.c_str())) {
    LOG_DEBUG("create table cms_log failed!");
    return false;
  }
  return true;
}


bool CLogDAO::addLog(const cmsg::CAddLogReq *req) {
  if (!req) {
    return false;
  }
  cmysql::MData data;
  data["service_name"] = req->service_name().c_str();
  data["service_ip"] = req->service_ip().c_str();
  int servicePort = req->service_port();
  data["service_port"] = &servicePort;
  data["log_txt"] = req->log_txt().c_str();
  int logTime = req->log_time();
  data["log_time"] = &logTime;
  int logLevel = req->log_level();
  data["log_level"] = &logLevel;

  std::lock_guard<std::mutex> guard(logMtx);
  return mysql.insertBin(data, "cms_log");
}