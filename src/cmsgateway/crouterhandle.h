#ifndef CROUTER_HANDLE_H
#define CROUTER_HANDLE_H

#include "cservicehandle.h"


class CRouterHandle : public CServiceHandle {
public:
  virtual void readCb(cmsg::CMsgHead *head, CMsg *msg);

  // 连接断开/超时/出错 时调用
  virtual void close();

private:

};

#endif

