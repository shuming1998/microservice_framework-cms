#ifndef CDIRSERVICE_HANDLE_H
#define CDIRSERVICE_HANDLE_H
#include "cservicehandle.h"

class CDirServiceHandle : public CServiceHandle {
public:
  CDirServiceHandle();
  ~CDirServiceHandle();

  // 处理用户的目录请求的回调函数
  void dirReqCb(cmsg::CMsgHead *head, CMsg *msg);

  // 用于注册回调函数
  static void regMsgCb();

private:


};

#endif // !CDIRSERVICE_HANDLE_H

