#include "cserverevent.h"
#include "cmsgcom.pb.h"
#include <iostream>

std::map<cmsg::CMsgType, CServerEvent::msgCbFunc> CServerEvent::cbs_;

// 初始化回调函数
void CServerEvent::init() {
  regCb(cmsg::MSG_LOGIN_REQ, &CServerEvent::loginReq);
}

// 接收登录请求消息
// @param data 消息数据
// @param size 消息大小
void CServerEvent::loginReq(const char *data, int size) {

  // 反序列化消息
  cmsg::CLoginReq req;
  req.ParseFromArray(data, size);
  std::cout << "recv user name = " << req.username() << '\n';
  std::cout << "recv password = " << req.password() << '\n';

  // 返回消息
  cmsg::CLoginRes res;
  res.set_res(cmsg::CLoginRes::OK);
  std::string token = req.username();
  token += "sign";
  res.set_token(token);
  sendMsg(cmsg::MSG_LOGIN_RES, &res);
}