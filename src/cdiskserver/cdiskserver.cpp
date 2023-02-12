#include "cthreadpool.h"
#include "cservertask.h"
#include "cfileservertask.h"
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#endif // _WIN32
#include <thread>

static void listenCb(int sock, sockaddr *addr, int sockLen, void *arg) {
  std::cout << "listenCbFunc in main\n";
  auto task = new CFileServerTask();
  task->setSock(sock);
  CThreadPool::get()->dispatch(task);
}

int main(int argc, char *argv[]) {
#ifdef _WIN32
  WSADATA wsa;
  WSAStartup(MAKEWORD(2, 2), &wsa);
#else
  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
    return 1;
  }
#endif // _WIN32

  int serverPort = 10086;
  int threadNum = 7;
  if (argc > 1) {
    serverPort = atoi(argv[1]);
  }
  if (argc > 2) {
    threadNum = atoi(argv[2]);
  }

  // 初始化主线程池
  CThreadPool::get()->init(threadNum);

  // 服务器线程池
  CThreadPool serverPool;
  serverPool.init(1);

  auto task = new CServerTask();
  task->setServerPort(serverPort);
  task->listenCbFunc_ = listenCb;
  serverPool.dispatch(task);

  for (;;) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  return 0;
}