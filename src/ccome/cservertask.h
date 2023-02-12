#ifndef CSERVER_TASK_H
#define CSERVER_TASK_H
#include "ctask.h"
#include <functional>

typedef void(*listenCbFunc)(int sock, struct sockaddr*addr, int socklen, void *arg);

class CCOME_API CServerTask : public CTask {
public:
  //using listenCbFunc = std::function<void(int sock, struct sockaddr *addr, int sockLen, void *arg)>;
  listenCbFunc listenCbFunc_ = nullptr;
  bool init() override;
  void setServerPort(int port) { this->serverPort_ = port; }

private:
  int serverPort_;

};

#endif // !CSERVER_TASK_H

