#include "cauthclient.h"
#include <thread>
#include <iostream>

int main() {
  CAuthClient::regMsgCallback();
  CAuthClient client;
  client.setServerIp("127.0.0.1");
  client.setServerPort(AUTH_PORT);
  client.startConnect();
  while (!client.isConnected()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // 添加用户
  cmsg::CAddUserReq addUser;
  addUser.set_username("root");
  addUser.set_password("123456");
  addUser.set_rolename("root");
  client.addUserReq(&addUser);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // 发起登录请求
  client.LoginReq("root", "123456");

  // 存储登录的 token 信息

  CThreadPool::wait();
  return 0;
}

