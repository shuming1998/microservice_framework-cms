#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#ifdef _WIN32

#else
#include <signal.h>

#endif // _WIN32

void readCb(struct bufferevent* bev, void* ctx) {
    std::cout << "readCb:\n" << std::flush;
    char buf[1024] = { 0 };
    int len = bufferevent_read(bev, buf, sizeof(buf) - 1);
    std::cout << buf << '\n';

    // 将要发送的消息插入 buffer 链表
    bufferevent_write(bev, "ok", 3);
}

void writeCb(struct bufferevent* bev, void* ctx) {
    std::cout << "[W]\n";
}

void eventCb(struct bufferevent* bev, short what, void* ctx) {
    std::cout << "[E]\n" << '\n';
    // 读超时
    if (what & BEV_EVENT_TIMEOUT && what & BEV_EVENT_READING) {
        std::cout << "BEV_EVENT_READING BEV_EVENT_TIMEOUT\n";
        // 需要先判断缓冲区是否有内容为读出

        // 清理空间，关闭监听
        bufferevent_free(bev);

        // 写超时
    }
    else if (what & BEV_EVENT_TIMEOUT && what & BEV_EVENT_WRITING) {
        std::cout << "BEV_EVENT_WRITING BEV_EVENT_TIMEOUT\n";
        // 缓冲回滚

        // 清理空间，关闭监听
        bufferevent_free(bev);

        // 异常错误
    }
    else if (what & BEV_EVENT_ERROR) {
        std::cout << "BEV_EVENT_ERROR\n";
        // 清理空间，关闭监听
        bufferevent_free(bev);

        // 连接断开
    }
    else if (what & BEV_EVENT_EOF) {
        std::cout << "BEV_EVENT_EOF\n";
        // 考虑缓冲的处理
        // 清理空间，关闭监听
        bufferevent_free(bev);
    }
}

void listenCb(struct evconnlistener* evc, evutil_socket_t client_socket, struct sockaddr* clientAddr, int socklen, void* args) {
    char ip[16] = { 0 };

    sockaddr_in* addr = (sockaddr_in*)clientAddr;
    evutil_inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
    std::cout << "client ip is: " << ip << '\n';

    // 创建 bufferevent 对象 bev（read 和 write)，通过 bev 进行网络通信
    event_base* base = (event_base*)args;
    // BEV_OPT_CLOSE_ON_FREE 关闭 BEV 时关闭 socket
    bufferevent* bev = bufferevent_socket_new(base, client_socket, BEV_OPT_CLOSE_ON_FREE);
    if (!bev) {
        std::cerr << "bufferevent_socket_new failed!\n";
        return;
    }

    // bufferevent_socket_new 中已经创建了读和写的 event，然后添加监控事件，设置内部权限参数
    bufferevent_enable(bev, EV_READ | EV_WRITE);
    // 设置超时时间: 秒，微秒(1/1000000)   读超时和写超时
    // 客户端连接后 10 秒内不发信息就会超时
    timeval tl = { 10, 0 };
    bufferevent_set_timeouts(bev, &tl, 0);
    // 设置回调函数
    bufferevent_setcb(bev, readCb, writeCb, eventCb, base);
}


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

    // 创建 libevent 的上下文，默认创建 base 锁
    event_base* base = event_base_new();
    if (base) {
        std::cout << "event_base_new success!\n";
    }

    // 监听连接
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(server_port);
    auto evc = evconnlistener_new_bind(base, listenCb, base, LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, 10, (sockaddr*)&addr, sizeof(addr));


    // 事件主循环，用于判断事件是否发生，以及分发事件到回调函数
    event_base_dispatch(base);
    // 如果没有事件注册会退出，所以需要清理资源
    evconnlistener_free(evc);
    event_base_free(base);

    return 0;
}


