#include "CFtpServerCmd.h"
#include <iostream>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <string.h>

// event 事件回调函数
void eventCb(bufferevent *bev, short flag, void *arg) {
  // 如果客户端网络断开或者死机，有可能无法收到 BEV_EVENT_EOF 数据
  if ((flag & BEV_EVENT_EOF) | (flag & BEV_EVENT_ERROR) | (flag & BEV_EVENT_TIMEOUT)) {
    std::cout << "BEV_EVENT_EOF | BEV_EVENT_ERROR\n";
    bufferevent_free(bev);
    delete (CFtpServerCmd *)arg;
  }
}

// 在子线程 CThread event 事件分发中调用
static void readCb(bufferevent *bev, void *arg) {
  char buf[1024] = { 0 };
  for (;;) {
    int len = bufferevent_read(bev, buf, sizeof(buf) - 1);
    if (len <= 0) {
      break;
    }
    buf[len] = '\0';
    std::cout << buf;

    // 测试
    if (strstr(buf, "quit")) {
      bufferevent_free(bev);
      delete (CFtpServerCmd *)arg;
      break;
    }
  }
}

// 初始化任务 运行在子线程中
bool CFtpServerCmd::init() {
  std::cout << "CFtpServerCmd::init()\n";

  // 利用 bufferevent 监听 socket
  // 创建 bufferevent
  bufferevent *bev = bufferevent_socket_new(base_, sock_, BEV_OPT_CLOSE_ON_FREE);
  bufferevent_setcb(bev, readCb, 0, eventCb, this);
  bufferevent_enable(bev, EV_READ | EV_WRITE);

  // 添加超时机制
  timeval readTv = { 10, 0 }; // 读事件的超时, 10 秒
  bufferevent_set_timeouts(bev, &readTv, 0);

  return true;
} 