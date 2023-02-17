#include "cloghandle.h"
#include "cmsgcom.pb.h"
#include "clogdao.h"
#include <iostream>

void CLogHandle::addLogReq(cmsg::CMsgHead *head, CMsg *msg) {
  cmsg::CAddLogReq req;
  if (!req.ParseFromArray(msg->data_, msg->size_)) {
    std::cerr << "CLogHandle::addLogReq error: ParseFromArray failed!\n";
  }
  if (req.service_ip().empty()) {
    req.set_service_ip(clientIp());
  }

  CLogDAO::get()->addLog(&req);
} 