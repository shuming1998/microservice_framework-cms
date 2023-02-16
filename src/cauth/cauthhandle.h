#ifndef CAUTH_HANDLE_H
#define CAUTH_HANDLE_H
#include "cservicehandle.h"

class CAuthHandle : public CServiceHandle {
public:
  CAuthHandle() {}
  ~CAuthHandle() {}

  // 接收登录请求
  void loginReq(cmsg::CMsgHead *head, CMsg *msg);

  // 接收添加用户请求
  void addUserReq(cmsg::CMsgHead *head, CMsg *msg);

  static void regMsgCallback() {
    regCb(cmsg::MSG_LOGIN_REQ, (msgCbFunc)&CAuthHandle::loginReq);
    regCb(cmsg::MSG_ADD_USER_REQ, (msgCbFunc)&CAuthHandle::addUserReq);
  }

private:

};





#endif // !CAUTH_HANDLE_H

