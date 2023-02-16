#include "clogclient.h"
#include "configdao.h"
#include "cmysql.h"
#include "ctools.h"
#include "cmsg.h"

#define CONFIG_TABLE "cms_service_config"

// 确保安装表、获取配置、写入/更新配置时 数据库的线程安全
static std::mutex mysql_Mtx;

bool ConfigDAO::init(const char *ip, const char *user, const char *password, const char *db, int port) {
  std::lock_guard<std::mutex> guard(mysql_Mtx);
  if (!mysql_) {
    mysql_ = new cmysql::CMysql();
  }

  if (!mysql_->init()) {
    LOG_DEBUG("mysql_->init() failed!");
    return false;
  }

  // 设置 mysql 自动重连
  mysql_->setReconnect(true);
  // 设置 mysql 最长等待连接时间
  mysql_->setConnectTimeout(3);
  if (!mysql_->connect(ip, user, password, db, port)) {
    LOG_DEBUG("mysql_->connect failed!");
    return false;
  }

  LOG_DEBUG("mysql_->connect success!");

  return true;
}

bool ConfigDAO::install() {
  LOG_DEBUG("ConfigDAO::install()!");
  std::lock_guard<std::mutex> guard(mysql_Mtx);
  if (!mysql_) {
    LOG_ERROR("mysql_ not init!");
    return false;
  }

  // 如果表不存在，则创建表
  std::string createTbSql = "CREATE TABLE IF NOT EXISTS `cms_service_config` ( \
                            `id` INT AUTO_INCREMENT, \
                            `service_name` VARCHAR(16), \
                            `service_port` INT, \
                            `service_ip` VARCHAR(16), \
                            `private_pb` VARCHAR(4096), \
                            `proto` VARCHAR(4096), \
                            PRIMARY KEY(`id`)); ";
  if (!mysql_->query(createTbSql.c_str())) {
    LOG_INFO("CREATE TABLE cms_service_config failed!\n");
    return false;
  }

  LOG_INFO("CREATE TABLE cms_service_config sucess!\n");
  return true;
}

bool ConfigDAO::uploadConfig(cmsg::CConfig *conf) {
  LOG_DEBUG("ConfigDAO::uploadConfig!");

  std::lock_guard<std::mutex> guard(mysql_Mtx);
  if (!mysql_) {
    LOG_ERROR("mysql_ not init!");
    return false;
  }

  if (!conf || conf->serviceip().empty()) {
    LOG_ERROR("ConfigDAO::saveConfig failed!");
    return false;
  }

  // 准备数据
  std::string tb = CONFIG_TABLE;
  cmysql::MData data;
  data["service_name"] = cmysql::CData(conf->servicename().c_str());
  int port = conf->serviceport();
  data["service_port"] = cmysql::CData(&port);
  data["service_ip"] = cmysql::CData(conf->serviceip().c_str());
  // 再序列化一次，把整个 CConfig 存入到 private_pb
  std::string privatePb;
  conf->SerializeToString(&privatePb);
  data["private_pb"].data = privatePb.c_str();
  data["private_pb"].size = privatePb.size();
  data["proto"].data = conf->proto().c_str();
  data["proto"].size = conf->proto().size();

  // 如果已经有此条数据，则修改数据
  std::stringstream ss;
  ss << " where service_ip='";
  ss << conf->serviceip() << "' and service_port=" << conf->serviceport();
  std::string where = ss.str();
  std::string sql = "select id from ";
  sql += tb;
  sql += where;

  LOG_DEBUG(sql);
  auto rows = mysql_->getResult(sql.c_str());
  bool res;
  if (rows.size() > 0) {
    int affetcRows  =  mysql_->updateBin(data, tb, where);
    if (affetcRows >= 0) {
      LOG_DEBUG("配置更新成功!");
      return true;
    }
    
    LOG_DEBUG("配置更新失败!");
    return false;
  }

  // 插入数据
  res = mysql_->insertBin(data, tb);
  if (res) {
    LOG_DEBUG("配置插入成功!");
  } else {
    LOG_DEBUG("配置插入失败!");
  }
  return res;
}

cmsg::CConfig ConfigDAO::downloadConfig(const char *ip, int port) {
  cmsg::CConfig conf;
  LOG_DEBUG("ConfigDAO::loadConfig!");

  std::lock_guard<std::mutex> guard(mysql_Mtx);
  if (!mysql_) {
    LOG_ERROR("mysql_ not init!");
    return conf;
  }

  if (!ip || port <= 0 || port >= 65536 || strlen(ip) == 0) {
    LOG_ERROR("ConfigDAO::loadConfig failed: ip or port is invalid!");
    return conf;
  }

  std::string tb = CONFIG_TABLE;
  std::stringstream ss;
  // 只需要获取配置项
  ss << "select private_pb from " << tb;
  ss << " where service_ip='" << ip << "' and service_port=" << port;
  auto rows = mysql_->getResult(ss.str().c_str());
  conf.set_serviceip(ip);
  conf.set_serviceport(port);
  if (rows.size() == 0) {
    LOG_DEBUG("download config failed!");
    return conf;
  }

  // 只取第一条记录
  auto row = rows[0];
  if (!conf.ParseFromArray(row[0].data, row[0].size)) {
    LOG_DEBUG("download config failed：ParseFromArray failed!");
    return conf;
  }

  LOG_DEBUG("download config success!");
  LOG_DEBUG(conf.DebugString());
  return conf;
}

cmsg::CConfigList ConfigDAO::downloadAllConfig(int page, int pageCount) {
  cmsg::CConfigList configs;
  LOG_DEBUG("downloadAllConfig");
  std::lock_guard<std::mutex> guard(mysql_Mtx);
  if (!mysql_) {
    LOG_ERROR("mysql_ not init!");
    return configs;
  }
  if (page <= 0 || pageCount <= 0) {
    LOG_ERROR("downloadAllConfig error: page or pageCount invalid!");
    return configs;
  }

  // 读取分页的配置列表 page 从 1 开始 每页数量 pageCount 
  std::string tableName = CONFIG_TABLE;
  std::stringstream ss;
  ss << "select `service_name`,`service_ip`,`service_port` from " << tableName;
  // 分页处理 select * from table limit 0,10（10,10、20,10、.....）
  ss << " order by id desc";
  ss << " limit " << (page - 1) * pageCount << "," << pageCount;
  LOG_DEBUG(ss.str().c_str());
  auto rows = mysql_->getResult(ss.str().c_str());
  for (auto row : rows) {
    // 遍历结果集插入到 proto 类型中
    auto pConf = configs.add_config();
    pConf->set_servicename(row[0].data);
    pConf->set_serviceip(row[1].data);
    pConf->set_serviceport(atoi(row[2].data));
  }

  return configs;
}

bool ConfigDAO::deleteConfig(const char *ip, int port) {
  LOG_DEBUG("deleteConfig");
  std::lock_guard<std::mutex> guard(mysql_Mtx);
  if (!mysql_) {
    LOG_ERROR("mysql_ not init!");
    return false;
  }

  if (!ip || port <= 0 || port >= 65536 || strlen(ip) == 0) {
    LOG_ERROR("ConfigDAO::deleteConfig failed: ip or port is invalid!");
    return false;
  }

  std::string tb = CONFIG_TABLE;
  std::stringstream ss;
  ss << "delete from " << tb;
  ss << " where service_ip='" << ip << "' and service_port=" << port;

  return mysql_->query(ss.str().c_str());
}