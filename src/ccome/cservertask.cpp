#include "cservertask.h"
#include <event2/event.h>
#include <event2/listener.h>
#include <iostream>
#include <string.h>

static void sListenCb(struct evconnlistener *evc,
                      evutil_socket_t client_socket,
                      struct sockaddr *clientAddr,
                      int socklen,
                      void *arg) {
  std::cout << "sListenCb\n";
  auto task = (CServerTask *)arg;
  if (!task->listenCbFunc_) {
    std::cerr << "listenCbFunc not set!\n";
  } else {
    task->listenCbFunc_(client_socket, clientAddr, socklen, arg);
  }
}

bool CServerTask::init() {
  if (serverPort_ <= 0) { 
    std::cerr << "CServerTask::init() failed! serverPort is not set!\n";
  }

  // 监听端口
  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(serverPort_);

  // 设置回调函数
  auto evc = evconnlistener_new_bind(getBase(), sListenCb, this, LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, 10, (sockaddr*)&addr, sizeof(addr));

  if (!evc) {
    std::cout << "listen port " << serverPort_ << "failed!\n";
    return false;
  }

  return true;
}