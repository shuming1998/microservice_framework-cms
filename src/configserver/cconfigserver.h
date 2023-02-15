#ifndef CCONFIG_SERVER_H
#define CCONFIG_SERVER_H
#include "cservice.h"



class CConfigServer : public CService {
public:
  CServiceHandle *createServiceHandle();

  // 根据参数初始化服务，需要先调用
  void mainFunc(int argc, char *argv[]);

  // 等待线程退出
  void wait();
};



#endif

