#ifndef CCOME_TASK_H
#define CCOME_TASK_H
#include "cmsg.h"
#include "ctask.h"

class CSSLCtx;
class CCOME_API CComeTask : public CTask {
public:
  CComeTask();
  virtual ~CComeTask();

  // 开始连接服务器
  // autoConnect 设定是自动重连
  // @return false: bev 环境未处理，true: 连接任务加入成功，但不代表连接成功
  virtual bool connect();

  // 初始化 bufferevent，客户端建立连接
  // 添加到线程池任务列表调用，包括客户端和服务端
  // @return 服务端返回 true，客户端调用 Connect 并返回
  virtual bool init();

  virtual void closeBev();

  // 封装 bufferevent_read
  int readMsg(void *data, int size);

  // 事件回调函数
  virtual void eventCb(short what);

  // 读取数据回调
  virtual void readCb() = 0;

  // 写入数据回调
  virtual void writeCb() {}

  // 接收到消息后处理消息的回调，由业务类重载
  // 返回 true 正常，返回 false 退出当前的消息处理，不处理下一条消息
  /*virtual bool readCb(const CMsg *msg) = 0;*/

  // 发送消息
  //virtual bool writeMsg(const CMsg *msg);
  virtual bool writeMsg(const void *data, int size);

  // 连接成功的消息回调
  // 如果是 ssl 加密通信，服务端也会进到这个函数中
  virtual void connetedCb() {}

  // 关闭消息接收时，数据将发送到此函数，由业务模块重载
  virtual void readCb(void *data, int size) {}

  // 激活写入回调
  virtual void beginWrite();

  // 清理所有定时器
  virtual void clearTimers();
  // 设定定时器，定时调用 timer(任务加入到线程池之后，只能设置一个定时器，在 init 函数中调用)
  // @param ms 定时调用的毫秒
  virtual void setTimer(int ms);
  // 定时器的回调函数
  virtual void timerCb() {}

  // 设置自动重连的定时器
  virtual void setAutoConnectTimer(int ms);
  // 自动重连定时器的回调函数
  virtual void autoConnectTimerCb();


  void setServerIp(const char *ip);
  void setServerPort(int port) { this->serverPort_ = port; }
  void setIsRecvMsg(bool isRecv) { this->isRecvMsg_ = isRecv; }

  const char *getServerIp() const { return this->serverIp_; };
  const int getServerPort() const { return this->serverPort_; }

  //本地IP用于获取配置项
  // 客户端在连接成功后设置 不是服务端的接收连接的客户端IP（client_ip()）
  void setLocalIp(const char *ip);
  const char *localIp() { return localIp_; };

  // 等待连接成功
  // @param timeoutSec 最大等待时间
  bool waitforConnected(int timeoutSec);

  // 建立连接，如果断开会再次重连，直到连接成功或超时
  bool autoConnect(int timeoutSec);

  bool isConnecting() const { return isConnecting_; }
  bool isConnected() const { return isConnected_; }

  // 设置连接断开时是否自动清理对象，包含清理定时器事件
  void setAutoDelete(bool is) { this->autoDelete_ = is; }
  // 设置自动重连，默认不自动重连，要在添加线程池之前设置
  // 一旦设置自动重连，对象就不能自动清理
  void setAutoConnect(bool is) {
    this->autoConnect_ = is;
    if (is) {
      this->autoDelete_ = false;
    }
  }

  // 设置 SSL 通信上下文，如果使用了，就使用 SSL 加密通信
  void setSslCtx(CSSLCtx *ctx) { this->sslCtx_ = ctx; }
  CSSLCtx *sslCtx() { return this->sslCtx_; }

  // 设定读超时，要在加入线程池之前
  void setReadTimeoutMs(int ms) { readTimeoutMs_ = ms; }

  //设定要在加入线程池之前 virtual void timerCb() {}
  void setTimerMs(int ms) { timerMs_ = ms; }

  void setClientIp(const char *ip);
  void setClientPort(int port) { clientPort_ = port; }
  const char *clientIp() const { return clientIp_; }

protected:
  char readBuf_[4096] = { 0 };        // 读缓冲区
  char clientIp_[16] = { 0 };         // 服务端收到连接，存放客户端的 IP
  int clientPort_ = 0;                // 服务端收到连接，存放客户端的 Port
  char localIp_[16] = { 0 };          // 本地 IP 用于获取配置项

private:
  bool initBev(int sock);


  int readTimeoutMs_ = 0;             // 读超时时间，超时设定要在 bufferevent 创建好之后，即 init 之后
  CSSLCtx *sslCtx_ = nullptr;         // ssl 通信的上下文
  bool autoDelete_ = true;            // 连接断开时是否清理对象
  bool autoConnect_ = false;          // 是否自动重连
  // 自动重连定时器事件，close 时不清理
  struct event *autoConnectTimerEvent_ = nullptr;

  char serverIp_[16] = { 0 };         // 服务器 ip 地址
  int serverPort_ = 0;                // 服务器端口号
  struct bufferevent *bev_ = nullptr;

  CMsg msg_;                          // 消息缓存
  bool isRecvMsg_ = true;             // 是否接收消息

  // 客户端的连接状态 1未处理(开始连接) 2连接中(等待连接成功) 3已连接(可以做业务操作) 4连接后失败(等待间隔时间后连接)
  bool isConnecting_ = true;
  bool isConnected_  = false;
  std::mutex *mtx_ = nullptr;
  // 定时器事件，close 时不清理
  struct event *timerEvent_ = nullptr;
  int timerMs_ = 0;                   // timerCb 定时调用时间
};


#endif // !CCOME_TASK_H

