#include "clogclient.h"
#include "ccometask.h"
#include "ctools.h"
#include "csslctx.h"
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>
#include <iostream>
#include <string.h>
#include <thread>

CComeTask::CComeTask() {
  mtx_ = new std::mutex();
}

CComeTask::~CComeTask() {
  delete mtx_;
}

// 回调的全局函数，在其中再调用成员函数
static void sReadCb(struct bufferevent *bev, void *ctx) {
  auto task = (CComeTask *)ctx;
  task->readCb();
}
static void sWriteCb(struct bufferevent *bev, void *ctx) {
  auto task = (CComeTask *)ctx;
  task->writeCb();
}
static void sEventCb(struct bufferevent *bev, short what, void *ctx) {
  auto task = (CComeTask *)ctx;
  task->eventCb(what);
}
static void sTimerCb(evutil_socket_t sock, short what, void *ctx) {
  auto task = (CComeTask *)ctx;
  task->timerCb();
}
static void sAutoConnectTimerCb(evutil_socket_t sock, short what, void *ctx) {
  auto task = (CComeTask *)ctx;
  task->autoConnectTimerCb();
}

void CComeTask::setTimer(int ms) {
  if (!getBase()) {
    LOG_ERROR("setTimer failed: base_ not set!");
    return;
  }

  timerEvent_ = event_new(getBase(), -1, EV_PERSIST, sTimerCb, this);
  if (!timerEvent_) {
    LOG_ERROR("set timer failed: event_new failed!");
    return;
  }

  int sec = ms / 1000;          // 秒
  int us = (ms % 1000) * 1000;  // 微秒
  timeval tv = { sec, us };
  event_add(timerEvent_, &tv);
  LOG_DEBUG("setTimer success!");
}

// 设置自动重连的定时器
void CComeTask::setAutoConnectTimer(int ms) {
  if (!getBase()) {
    LOG_ERROR("setAutoConnectTimer failed: base_ not set!");
    return;
  }

  // 如果之前设置过自动重连，就先清理上一次的
  if (autoConnectTimerEvent_) {
    event_free(autoConnectTimerEvent_);
    autoConnectTimerEvent_ = nullptr;
  }

  autoConnectTimerEvent_ = event_new(getBase(), -1, EV_PERSIST, sAutoConnectTimerCb, this);
  if (!autoConnectTimerEvent_) {
    LOG_ERROR("setAutoConnectTimer failed: event_new failed!");
    return;
  }

  int sec = ms / 1000;          // 秒
  int us = (ms % 1000) * 1000;  // 微秒
  timeval tv = { sec, us };
  event_add(autoConnectTimerEvent_, &tv);
}

// 自动重连定时器的回调函数
void CComeTask::autoConnectTimerCb() {
  std::cout << "CComeTask:autoConnectTimerCb\n";
  // 如果正在连接，则等待，否则开始连接
  if (isConnected()) {
    return;
  }
  if (!isConnecting()) {
    connect();
  }
}

int CComeTask::readMsg(void *data, int size) {
  if (!bev_) {
    LOG_ERROR("bev not set");
    return 0;
  }
  int res = bufferevent_read(bev_, data, size);
  return res;
}

void CComeTask::beginWrite() {
  if (!bev_) {
    return;
  }
  // 激活写入回调函数
  bufferevent_trigger(bev_, EV_WRITE, 0);
}

void CComeTask::eventCb(short what) {
  std::cout << "eventCb " << what << '\n';
  if (what & BEV_EVENT_CONNECTED) {
    std::cout << "BEV_EVENT_CONNECTED\n";
    std::stringstream ss;
    ss << "connect server " << serverIp_ << ':' << serverPort_ << " success!";
    LOG_INFO(ss.str().c_str());
    // 通知连接成功
    isConnected_ = true;
    isConnecting_ = false;
    // 打印 ssl 信息
    auto ssl = bufferevent_openssl_get_ssl(bev_);
    if (ssl) {
      CSSL cssl;
      cssl.setSSL(ssl);
      // 打印证书
      cssl.printCert();
      // 打印加密算法
      cssl.printCipher();
    }
    connetedCb();
  }

  // 退出时要处理缓冲
  if (what & BEV_EVENT_ERROR) {
    std::cout << "BEV_EVENT_ERROR";
    auto ssl = bufferevent_openssl_get_ssl(bev_);
    if (ssl) {
      CSSL cssl;
      cssl.setSSL(ssl);
      // 打印证书
      cssl.printCert();
      // 打印加密算法
      cssl.printCipher();
    }
    std::cout << "BEV_EVENT_ERROR\n";
    int sock = bufferevent_getfd(bev_);
    int err = evutil_socket_geterror(sock);
    LOG_DEBUG(evutil_socket_error_to_string(err));
    closeBev();
  }

  if (what & BEV_EVENT_TIMEOUT) {
    std::cout << "BEV_EVENT_TIMEOUT\n";
    closeBev();
  }

  if (what & BEV_EVENT_EOF) {
    std::cout << "BEV_EVENT_EOF\n";
    closeBev();
  }
}

bool CComeTask::writeMsg(const void *data, int size) {
  std::lock_guard<std::mutex> guard(*mtx_);
  if (!bev_ || !data || size <= 0) {
    return false;
  }
  int res = bufferevent_write(bev_, data, size);
  if (res != 0) {
    std::cerr << "CComeTask::writeMsg(const void *data, int size) error!\n";
    return false;
  }
  return true;
}

bool CComeTask::connect() {
  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(serverPort_);
  evutil_inet_pton(AF_INET, serverIp_, &addr.sin_addr.s_addr);

  std::lock_guard<std::mutex> guard(*mtx_);
  isConnected_ = false;
  isConnecting_ = false;
  if (!bev_) {
    initBev(-1);
  }
  if (!bev_) {
    LOG_ERROR("CComTask::Connect failed! bev is null!");
    return false;
  }

  // 建立 socket 连接
  int res = bufferevent_socket_connect(bev_, (sockaddr *)&addr, sizeof(addr));
  if (res != 0) {
    return false;
  }

  isConnecting_ = true;
  return true;
}

bool CComeTask::initBev(int sock) {
  // 用 bufferevent 建立连接，comeSock = -1 时自动创建 socket
  // 创建 bufferevent 上下文
  // 区分是否加密通信，以及区分服务器和客户端
  if (!sslCtx()) {    // 非加密通信
    bev_ = bufferevent_socket_new(getBase(), sock, BEV_OPT_CLOSE_ON_FREE);
    if (!bev_) {
      LOG_ERROR("bufferevent_socket_new() failed!");
      return false;
    }
  } else {            // 加密通信
    auto cssl = sslCtx()->newCSSL(sock);
    // 客户端，sock 传入的是 -1
    if (sock < 0) {
      // bufferevent_free 会同时关闭 socket 和 ssl
      bev_ = bufferevent_openssl_socket_new(getBase(),
                                            sock,
                                            cssl.ssl(),
                                            BUFFEREVENT_SSL_CONNECTING,
                                            BEV_OPT_CLOSE_ON_FREE);
    } else {
      // 服务端
      bev_ = bufferevent_openssl_socket_new(getBase(),
                                            sock,
                                            cssl.ssl(),
                                            BUFFEREVENT_SSL_ACCEPTING,
                                            BEV_OPT_CLOSE_ON_FREE);
    }

    if (!bev_) {
      LOG_ERROR("bufferevent_openssl_socket_new() failed!");
      return false;
    }
  }


  // 设定读超时时间
  if (readTimeoutMs_ > 0) {
    // 秒，微秒
    timeval readTv = { readTimeoutMs_ / 1000, (readTimeoutMs_ % 1000) * 1000 };
    bufferevent_set_timeouts(bev_, &readTv, 0);
  }

  //定时器设定
  if (timerMs_ > 0)
  {
    setTimer(timerMs_);
  }

  // 设置回调函数
  bufferevent_setcb(bev_, sReadCb, sWriteCb, sEventCb, this);
  // 设置权限
  bufferevent_enable(bev_, EV_READ | EV_WRITE); 
  // 设置超时时间
  //timeval tv = { 10, 0 };
  //bufferevent_set_timeouts(bev_, &tv, &tv);
  return true;
}

bool CComeTask::init() {
  // comeSock 用于区分是服务端还是客户端
  int comeSock = getSock();
  if (comeSock <= 0) {
    comeSock = -1;
  }

  {
    std::lock_guard<std::mutex> guard(*mtx_);
    initBev(comeSock);
  }

  // 连接服务器
  if (serverIp_[0] == '\0') {
    return true;
  }

  // 断开三秒自动重连
  setAutoConnectTimer(3000);

  // 客户端
  return connect();
}

void CComeTask::clearTimers() {
  if (autoConnectTimerEvent_) {
    event_free(autoConnectTimerEvent_);
    autoConnectTimerEvent_ = nullptr;
  }
  if (timerEvent_) {
    event_free(timerEvent_);
    timerEvent_ = nullptr;
  }
}

void CComeTask::closeBev() {
  {
    std::lock_guard<std::mutex> guard(*mtx_);
    isConnected_ = false;
    isConnecting_ = false;
    if (bev_) {
      // 包含释放 socket 和 ssl
      bufferevent_free(bev_);
    }
    bev_ = nullptr;
    if (msg_.data_) {
      delete msg_.data_;
    }
    memset(&msg_, 0, sizeof(msg_));
  }

  // 清理连接对象空间、清理定时器，如果断开重连，需要单独处理
  if (autoDelete_) {
    clearTimers();
    delete this;
  }
}

bool CComeTask::waitforConnected(int timeoutSec) {
  // 每 10 毫秒监听一次
  int milsec = timeoutSec * 100;
  for (int i = 0; i < milsec; ++i) {
    if (isConnected()) {
      return true;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return isConnected();
}

bool CComeTask::autoConnect(int timeoutSec) {
  // 如果正在连接，则等待，否则开始连接
  if (isConnected()) {
    return true;
  }
  if (!isConnecting()) {
    connect();
  }
  return waitforConnected(timeoutSec);
}

void CComeTask::setServerIp(const char *ip) { 
  strncpy(this->serverIp_, ip, sizeof(serverIp_));
}

void CComeTask::setClientIp(const char *ip) {
  if (!ip) {
    return;
  }
  strncpy(clientIp_, ip, sizeof(clientIp_));
}

void CComeTask::setLocalIp(const char *ip) {
  strncpy(this->localIp_, ip, sizeof(localIp_));
}