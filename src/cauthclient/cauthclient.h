#ifndef CAUTH_CLIENT_H
#define CAUTH_CLIENT_H
#include "cserviceclient.h"
#include <string>

class CAuthClient : public CServiceClient {
public:
  CAuthClient() {}
  ~CAuthClient() {}

  // 发出登录请求
  // @param username 用户名
  // @param password 明文密码，在函数中会经过 md5 base64 编码后发送
  void LoginReq(std::string username, std::string password);

  // 添加用户请求
  void addUserReq(cmsg::CAddUserReq *user);

  // 注册上面的回调函数
  static void regMsgCallback() {
    regCb(cmsg::MSG_LOGIN_RES, (msgCbFunc)&CAuthClient::LoginRes);
    regCb(cmsg::MSG_ADD_USER_RES, (msgCbFunc)&CAuthClient::addUserRes);
  }

private:
  // 发出登录请求的响应
  void LoginRes(cmsg::CMsgHead *head, CMsg *msg);
  // 添加用户请求的响应
  void addUserRes(cmsg::CMsgHead *head, CMsg *msg);
};




#endif

