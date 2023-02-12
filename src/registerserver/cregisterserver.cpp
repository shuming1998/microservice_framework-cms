#include "cregisterserver.h"
#include "cregisterhandle.h"

CServiceHandle *CRegisterServer::createServiceHandle() {
  return new CRegisterHandle();
}

void CRegisterServer::mainFunc(int argc, char *argv[]) {
  // 注册消息回调函数
  CRegisterHandle::regMsgCallback();

  int port = REGISTER_PORT;
  if (argc > 1) {
    port = atoi(argv[1]);
  }
  // 设置服务器监听端口
  setServerPort(port);
}

void CRegisterServer::wait() {
  CThreadPool::wait();
}