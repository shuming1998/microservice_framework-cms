#include <iostream>
#include "cregisterserver.h"

int main(int argc, char *argv[]) {

  std::cout << "Register server\n";
  CRegisterServer server;
  // 初始化，传递端口号等参数
  server.mainFunc(argc, argv);
  // 启动服务线程，开始监听端口
  server.start();
  // 阻塞等待线程池退出
  server.wait();

  return 0;
}