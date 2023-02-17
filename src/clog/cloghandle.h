#ifndef CLOG_HANDLE_H
#define CLOG_HANDLE_H
#include "cservicehandle.h"

class CLogHandle : public CServiceHandle {
public:
  CLogHandle() { regMsgCallback(); }
  ~CLogHandle() {}

  void addLogReq(cmsg::CMsgHead *head, CMsg *msg);

  static void regMsgCallback() {
    regCb(cmsg::MSG_ADD_LOG_REQ, (msgCbFunc)&CLogHandle::addLogReq);
  }

};

#endif // !CLOG_HANDLE_H

