#include "cservice.h"
#include "cdirservicehandle.h"
#include "crouterserver.h"
#include "cserviceproxy.h"
#include "cregisterclient.h"
#include <iostream>

int main(int argc, char *argv[]) {
  std::cout << "Run cmsgateway like this: ./[executable file name] [gateway port] [register ip] [register port]\n";

  int serverPort = API_GATEWAY_PORT;
  if (argc > 1) {
    serverPort = atoi(argv[1]);
  }

  std::cout << "server port is " << serverPort << '\n';
  std::string registerIp = "127.0.0.1";
  int registerPort = REGISTER_PORT;
  if (argc > 2) {
    registerIp = argv[2];
  }
  if (argc > 3) {
    registerPort = atoi(argv[3]);
  }


  // 设置注册中心的 IP 和 端口
  CRegisterClient::get()->setServerIp(registerIp.c_str());
  CRegisterClient::get()->setServerPort(registerPort);
  // 注册到注册中心
  CRegisterClient::get()->registerServer(API_GATEWAY_NAME, serverPort, 0);
  // 等待注册中心连接
  CRegisterClient::get()->waitforConnected(3);
  CRegisterClient::get()->getServiceReq(0);

  // 初始化 CServiceProxy，建立连接加入到线程池中
  CServiceProxy::get()->init();
  // 开启自动重连
  CServiceProxy::get()->start();

  CRouterServer service;
  service.setServerPort(serverPort);
  service.start();
  CThreadPool::wait();

  return 0;
}