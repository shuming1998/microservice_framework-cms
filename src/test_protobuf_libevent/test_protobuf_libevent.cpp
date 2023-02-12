#include "cmsgserver.h"
#include "cmsgclient.h"
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>  // 和 protobuf 头文件有冲突，protobuf 头文件要在 windows.h 之前
#else
#include <signal.h>
#endif // _WIN32
 

int main(int argc, char* argv[]) {
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
    CMsgClient client;
    client.setServerPort(server_port);
    client.startThread();

    CMsgServer server;
    server.init(server_port);
    return 0;
}


