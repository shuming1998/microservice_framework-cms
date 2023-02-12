#include "crouterhandle.h"
#include "cserviceproxy.h"
#include "ctools.h"

void CRouterHandle::readCb(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("CRouterHandle::readCb");
  // 转发消息
  CServiceProxy::get()->sendMsg(head, msg, this);

}

void CRouterHandle::close() {
  CMsgEvent::close();
  CServiceProxy::get()->delEvent(this);
}