#ifndef CREGISTER_HANDLE_H
#define CREGISTER_HANDLE_H
#include "cservicehandle.h"

// 处理注册中心的客户端，对应一个连接
class CRegisterHandle : public CServiceHandle {
public:
  // 心跳
  void heartRes(cmsg::CMsgHead *head, CMsg *msg) {};

  // 接收服务的注册请求
  void registerReq(cmsg::CMsgHead *head, CMsg *msg);

  // 接收服务的发现请求
  void getServiceReq(cmsg::CMsgHead *head, CMsg *msg);

  static void regMsgCallback() {
    regCb(cmsg::MSG_HEART_REQ, (msgCbFunc)&CRegisterHandle::heartRes);
    regCb(cmsg::MSG_REGISTER_REQ, (msgCbFunc)&CRegisterHandle::registerReq);
    regCb(cmsg::MSG_GET_SERVICE_REQ, (msgCbFunc)&CRegisterHandle::getServiceReq);
  }

};


#endif

