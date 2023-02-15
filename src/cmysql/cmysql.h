#ifndef CMYSQL_H
#define CMYSQL_H

#include "cdata.h"
#include <vector>

struct MYSQL;
struct MYSQL_RES;
namespace cmysql {

class CMYSQL_API CMysql {
public:
  // 初始化 MYSQL API
  bool init();

  // 清理占用的所有资源
  void close();

  // 建立数据库连接(不考虑线程安全)
  // @param flag 设置支持多条语句
  bool connect(const char *host, const char *user, const char *password, const char *db, unsigned short port = 3306, /*const char *unixSock = 0,*/ unsigned long flag = 0);

  // 执行 sql 语句. 如果 sqlLen = 0，则用 strlen 获取字符串长度
  bool query(const char *sql, unsigned long sqlLen = 0);

  // Mysql 参数设定(超时时间、自动重连，在连接之前调用)
  bool option(COption opt, const void *arg);

  // 设置连接超时时间
  bool setConnectTimeout(int sec);

  // 设置超时重连
  bool setReconnect(bool is = true);

  // 获取结果集
  bool storeResult();   // 返回全部结果
  bool useResult();     // 开始接收结果，通过 fetch 获取结果

  // 释放结果集占用的空间
  void freeResult();

  // 获取一行数据
  std::vector<CData> fetchRow();

  // 生成 insert sql 语句
  std::string getInsertSql(MData kv, std::string table);

  // 插入非二进制数据
  bool insert(MData kv, std::string table);

  // 插入二进制数据
  bool insertBin(MData kv, std::string table);

  // 获取 update 数据的 sql 语句，用户要包含 where
  std::string getUpdateSql(MData kv, std::string table, std::string where);

  // 执行 update，返回更新数量，失败返回 -1
  int update(MData kv, std::string table, std::string where);

  // update 二进制数据
  int updateBin(MData kv, std::string table, std::string where);

  // 事务接口
  bool startTransaction();
  bool stopTransaction();
  bool commit();
  bool rollback();

  // 简易接口，返回 select 的数据结果，每次调用时清理上次的结果集
  CRows getResult(const char *sql);

protected:
  MYSQL *mysql_ = nullptr;
  MYSQL_RES *result_ = nullptr;

};

}// namespace cmysql

#endif
