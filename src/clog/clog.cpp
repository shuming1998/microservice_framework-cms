#include "cservice.h"
#include "cloghandle.h"
#include "clogdao.h"
#include <iostream>

class CLogServer :public CService {
public:
  CServiceHandle *createServiceHandle() override {
    return new CLogHandle();
  }

};

int main() {
  std::cout << "CLog Server " << std::endl;
  CLogDAO::get()->init();
  CLogDAO::get()->install();

  CLogServer server;
  server.setServerPort(LOG_PORT);
  server.start();


  CThreadPool::wait();
  return 0;
}

