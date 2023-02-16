#include "cservice.h"
#include "cauthhandle.h"
#include "cauthdao.h"
#include <iostream>

class CAuthService : public CService {
public:
  CServiceHandle *createServiceHandle() override {
    return new CAuthHandle();
  }

private:

};


int main(int argc, char *argv[]) {
  CAuthDAO::get()->init();
  CAuthDAO::get()->install();

  int serverPort = AUTH_PORT;
  CAuthHandle::regMsgCallback();
  CAuthService service;
  service.setServerPort(serverPort);
  service.start();


  CThreadPool::wait();
  return 0;
}


