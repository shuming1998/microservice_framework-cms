#include "CThreadPool.h"
#include "CFtpServerCmd.h"
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <iostream>
#include <string.h>
#ifdef _WIN32

#else
#include <signal.h>

#endif // _WIN32

void listenCb(struct evconnlistener *evc, evutil_socket_t client_socket, struct sockaddr *clientAddr, int socklen, void *arg) {
  std::cout << "listen cb\n";
  CTask *task = new CFtpServerCmd();
  task->sock_ = client_socket;
  CThreadPool::get()->dispatch(task);
}


int main(int argc, char *argv[]) {
  int server_port = 10080;
  if (argc > 1) {
    server_port = atoi(argv[1]);
  }

#ifdef _WIN32
  // 初始化 socket 库
  WSADATA wsa;
  WSAStartup(MAKEWORD(2, 2), &wsa);
#else
  // 忽略管道信号，发送数据给已关闭的 socket
  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
    return 1;
  }
#endif

  // ① 初始化线程池
  CThreadPool::get()->init(8);

  // 创建 libevent 的上下文，默认创建 base 锁
  event_base *base = event_base_new();
  if (base) {
    std::cout << "event_base_new success!\n";
  }

  // 监听连接
  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(server_port);
  auto evc = evconnlistener_new_bind(base, listenCb, base, LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, 10, (sockaddr *)&addr, sizeof(addr));

  // 事件主循环，用于判断事件是否发生，以及分发事件到回调函数
  event_base_dispatch(base);
  // 如果没有事件注册会退出，所以需要清理资源
  evconnlistener_free(evc);
  event_base_free(base);

  return 0;
}


