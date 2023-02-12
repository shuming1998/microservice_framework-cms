#include "cclientevent.h"
#include "cmsgcom.pb.h"
#include <iostream>
#include <thread>

std::map<cmsg::CMsgType, CClientEvent::msgCbFunc> CClientEvent::cbs_;

// 初始化回调函数
void CClientEvent::init() {
  regCb(cmsg::MSG_LOGIN_RES, &CClientEvent::loginRes);
}


// 接收登录反馈消息
// @param data 消息数据
// @param size 消息大小
void CClientEvent::loginRes(const char *data, int size) {
  std::cout << "loginRes " << size << '\n';
  // 解封装 反序列化
  cmsg::CLoginRes res;
  res.ParseFromArray(data, size);
  std::cout << res.res() << " recv server token: " << res.token() << '\n';

  // 序列化
  cmsg::CLoginReq req;
  char buf[1024] = { 0 };
  static int count = 0;
  ++count;
  sprintf(buf, "root_%d", count);
  req.set_username(buf);
  req.set_password("123456");
  sendMsg(cmsg::MSG_LOGIN_REQ, &req);
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
}