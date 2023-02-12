#include "cdirtask.h"
#include <iostream>

void CDirTask::connetedCb() {
  CMsg msg;
  msg.type_ = MSG_GET_DIR;
  msg.size_ = serverRoot_.size() + 1;
  msg.data_ = (char *)serverRoot_.c_str();
  writeMsg(&msg);
}

bool CDirTask::readCb(const CMsg *msg) {
  // 接收到服务端发送的目录
  switch (msg->type_) {
    case MSG_DIR_LIST:
      std::cout << "MSG_DIR_LIST\n";
      dirCb_(msg->data_);
      break;


    default:
      break;
  }
  return true;
}