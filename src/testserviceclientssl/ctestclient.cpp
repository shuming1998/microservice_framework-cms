#include "ctestclient.h"
#include "ctools.h"
#include "cmsgcom.pb.h"
#include <thread>
#include <sstream>

//void CTestClient::readCb() {
//  LOG_DEBUG("readCb");
//}
//
//
//void CTestClient::connectedCb() {
//  LOG_DEBUG("connected");
//}

bool CTestClient::getDir(std::string path) {
  LOG_DEBUG("begin CTestClient::getDir");
  if (!autoReconnect(300)) {
    return false;
  }
  std::stringstream ss;
  ss << "CTestClient::getDir: " << path;
  LOG_DEBUG(ss.str().c_str());

  // 发送 pb 的消息给服务
  cmsg::CDirReq req;
  req.set_path(path);
  cmsg::CMsgHead head;
  head.set_msg_type(cmsg::MSG_DIR_REQ);
  head.set_service_name("dir");
  head.set_token("test token");

  return sendMsg(&head, &req);
}

void CTestClient::dirResCb(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("CTestClient::dirResCb");
  // 反序列化消息
  cmsg::CDirRes res;
  if (!res.ParseFromArray(msg->data_, msg->size_)) {
    LOG_DEBUG("res.ParseFromArray failed!");
    return;
  }

  std::cout << "================== recv dir ==================\n";
  for (auto dir : res.dirs()) {
    std::cout << "[" << dir.filename() << "]:" << dir.filesize() << '\n';
  }
}


bool CTestClient::autoReconnect(int timeoutMs) {
  // 1 已连接
  if (isConnected()) {
    return true;
  }

  // 未连接，也不在连接中
  if (!isConnecting()) {
    if (!connect()) {
      return false;
    }
  }

  int time = timeoutMs / 10;
  // 连接中
  for (int i = 0; i < time; ++i) {
    if (isConnected()) {
      return true;
    }
    if (!isConnecting()) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  return isConnected();
}