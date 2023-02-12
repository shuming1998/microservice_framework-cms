#ifndef CONFIG_DAO_H
#define CONFIG_DAO_H
#include "cmsgcom.pb.h"

namespace cmysql {
  class CMysql;
}

class ConfigDAO {
public:
  virtual ~ConfigDAO() {}

  static ConfigDAO *get() {
    static ConfigDAO dao;
    return &dao;
  }

  // 初始化数据库
  bool init(const char *ip, const char *user, const char *password, const char *db, int port = 3306);

  // 安装数据库的表
  bool install();

  // 保存配置，如果已有就更新
  bool uploadConfig(cmsg::CConfig *conf);

  // 读取配置
  cmsg::CConfig downloadConfig(const char *ip, int port);

  // 读取分页的配置列表
  cmsg::CConfigList downloadAllConfig(int page, int pageCount);

  // 删除指定微服务配置
  bool deleteConfig(const char *ip, int port);

private:
  ConfigDAO() {}

  // mysql 数据库上下文
  cmysql::CMysql *mysql_ = nullptr;

};

#endif

