#ifndef CTEST_CLIENT_H
#define CTEST_CLIENT_H

#include "cserviceclient.h"

class CTestClient : public CServiceClient {
public:

  //void readCb();
  //void connectedCb();

  // 注册回调函数
  static void regMsgCb() {
    regCb(cmsg::MSG_DIR_RES, (msgCbFunc)&dirResCb);
  }

  // 获取目录
  bool getDir(std::string path);

  // 检查自动重连，连接失败时立刻返回
  bool autoReconnect(int timeoutMs);

  // getDir 目录请求的响应
  void dirResCb(cmsg::CMsgHead *head, CMsg *msg);

private:

};


#endif // !CTEST_CLIENT_H
