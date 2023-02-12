#include "cmsgclient.h"
#include "cmsgevent.h"
#include "cclientevent.h"
#include "cmsgcom.pb.h"
#include "cmsgtype.pb.h"
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <iostream>
#include <thread>
#include <chrono>

static void readCb(struct bufferevent* bev, void* ctx) {
  std::cout << "client readCb:\n" << std::flush;

  auto ev = (CClientEvent *)ctx;
  // 判断消息接受是否成功，失败需要释放资源
  if (!ev->recvMsg()) {
    delete ev;
    bufferevent_free(bev);
    return;
  }

  auto msg = ev->getMsg();
  if (!msg) {
    return;
  }
  ev->callFunc(msg->type_, msg->data_, msg->size_);
  ev->clear();
}

static void eventCb(struct bufferevent* bev, short what, void* ctx) {
  std::cout << "server eventCb\n" << '\n';
  auto ev = (CMsgEvent *)ctx;
  // 读超时
  if (what & BEV_EVENT_TIMEOUT || what & BEV_EVENT_ERROR || what & BEV_EVENT_EOF) {
    std::cout << "BEV_EVENT_READING || BEV_EVENT_ERROR || BEV_EVENT_EOF\n";
    // 需要先判断缓冲区是否有内容为读出
    delete ev;
    // 清理空间，关闭监听
    bufferevent_free(bev);
    return;
  }

  if (what & BEV_EVENT_CONNECTED) {
    std::cout << "BEV_EVENT_CONNECTED\n";
    // bufferevent_write(bev, "ok", 3);
    // 发送请求登录消息
    cmsg::CLoginReq req;
    req.set_username("root");
    req.set_password("123456");
    ev->sendMsg(cmsg::MSG_LOGIN_REQ, &req);
  }
}

void CMsgClient::startThread() {
  std::thread t(&CMsgClient::mainFunc, this);
  t.detach();
}

void CMsgClient::mainFunc() {
  // 注册消息回调函数
  CClientEvent::init();
  if (serverPort_ <= 0) {
    std::cerr << "client error: please set server port!\n";
  }
  std::this_thread::sleep_for(std::chrono::microseconds(200));  // 等待服务端启动
  std::cout << "CMsgClient::mainFunc() start!\n";

  event_base *base = event_base_new();
  // 连接服务端
  bufferevent *bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
  
  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(this->serverPort_);
  if (this->serverIp_.empty()) {
    serverIp_ = "127.0.0.1";
  }
  evutil_inet_pton(AF_INET, this->serverIp_.c_str(), &addr.sin_addr.s_addr);

  // 设置回调函数
  // bufferevent_socket_new 中已经创建了读和写的 event，然后添加监控事件，设置内部权限参数
  bufferevent_enable(bev, EV_READ | EV_WRITE);
  // 设置超时时间: 秒，微秒(1/1000000)   读超时和写超时
  // 客户端连接后 10 秒内不发信息就会超时
  timeval tl = { 30, 0 };
  bufferevent_set_timeouts(bev, &tl, 0);

  auto ev = new CClientEvent();
  ev->setBev(bev);

  // 设置回调函数
  bufferevent_setcb(bev, readCb, 0, eventCb, ev);

  int res = bufferevent_socket_connect(bev, (sockaddr *)&addr, sizeof(addr));
  if (res != 0) {
    std::cerr << "bufferevent_socket_connect error!\n";
    return;
  }

  // 事件主循环，用于判断事件是否发生，以及分发事件到回调函数
  event_base_dispatch(base);
  event_base_free(base);
}
