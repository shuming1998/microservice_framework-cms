#include <event2/event.h>
#include <iostream>
#include <string.h>
#ifdef _WIN32

#else
#include <signal.h>

#endif // _WIN32

void listenCb(evutil_socket_t sock, short what, void *arg) {
  // 首先处理客户端的连接事件
  std::cout << "listenCb\n" << '\n';
  if (!(what & EV_READ)) {
    std::cout << "not read!\n";
    return;
  }
  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  socklen_t size = sizeof(addr);
  evutil_socket_t client_socket = accept(sock, (sockaddr *)&addr, &size);
  if (client_socket <= 0) {
    std::cerr << "accept error!\n";
    return;
  }

  char ip[16] = { 0 };
  evutil_inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
  std::cout << "client ip is: " << ip << '\n';

  // 然后注册客户端的读写事件
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
  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
    return 1;
  }
#endif

  // 1 创建 libevent 的上下文，默认创建 base 锁
  event_base *base = event_base_new();
  if (base) {
    std::cout << "event_base_new success!\n";
  }

  // 2 创建 socket 绑定端口
  evutil_socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock <= 0) {
    std::cerr << "socket error" << strerror(errno) << '\n';
    return -1;
  }

  // 设置地址复用和非阻塞
  evutil_make_socket_nonblocking(sock);
  evutil_make_listen_socket_reuseable(sock);

  // 绑定端口
  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(server_port);
  int res = ::bind(sock, (sockaddr *)&addr, sizeof(addr));
  if (res != 0) {
    std::cerr << "bind port " << server_port << " error: " << strerror(errno) << '\n';
    return -1;
  }
  std::cout << "bind port success: " << server_port << '\n';

  listen(sock, 10);

  // 3 注册 socket 监听事件回调函数 读事件，持久化监听，默认水平触发，既只要有数据没处理就一直触发(EV_ET 边缘触发)
  event *ev = event_new(base, sock, EV_READ | EV_PERSIST, listenCb, event_self_cbarg());
  // 开始监听事件, 0 是超时时间
  event_add(ev, 0);

  // 事件主循环，用于判断事件是否发生，以及分发事件到回调函数
  event_base_dispatch(base);
  // 如果没有事件注册会退出，所以需要清理资源
  evutil_closesocket(sock);
  event_del(ev);
  event_free(ev);
  event_base_free(base);

  return 0;
}


