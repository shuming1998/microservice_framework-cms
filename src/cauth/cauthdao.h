#ifndef CAUTH_DAO_H
#define CAUTH_DAO_H

namespace cmsg {
  class CAddUserReq;
  class CLoginReq;
  class CLoginRes;
}

class CAuthDAO {
public:
  ~CAuthDAO() {}
  static CAuthDAO *get() {
    static CAuthDAO dao;
    return &dao;
  }

  // 初始化数据库，准备用户名密码
  bool init();

  // 安装表
  bool install();

  // 添加用户
  bool addUser(cmsg::CAddUserReq *user);

  // 登录数据库
  // @param userReq 用户登录信息，此时密码已加密
  // @param userRes 返回用户 token
  // @param timeoutSec token 超时时间
  bool login(const cmsg::CLoginReq *userReq, cmsg::CLoginRes *userRes, int timeoutSec);

private:
  CAuthDAO() {}

};







#endif // ! CAUTH_DAO_H

