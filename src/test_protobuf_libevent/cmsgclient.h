#ifndef  CMSG_CLIENT_H
#define  CMSG_CLIENT_H
#include <string>

class CMsgClient {
public:
  // 延时启动测试线程，发送数据给服务端
  void startThread();

  // 线程主函数
  void mainFunc();

  void setServerPort(int port) { this->serverPort_ = port; }
  void setServerIp(std::string ip) { this->serverIp_ = ip; }

private:
  int serverPort_ = 0;
  std::string serverIp_ = "";

};


#endif // ! CMSG_CLIENT_H

