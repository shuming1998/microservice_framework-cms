#ifndef CCOME_TASK_H
#define CCOME_TASK_H
#include "cmsg.h"
#include "ctask.h"

class CSSLCtx;
class CCOME_API CComeTask : public CTask {
public:
  CComeTask();
  virtual ~CComeTask();

  // 开始连接服务器，考虑到要自动重连，所以将连接逻辑单独放到一个函数中
  virtual bool connect();

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

  // 设定定时器，定时调用 timer(任务加入到线程池之后，只能设置一个定时器，在 init 函数中调用) 
  // @param ms 定时调用的毫秒
  virtual void setTimer(int ms);
  // 定时器的回调函数
  virtual void timerCb() {}

  void setServerIp(const char *ip);
  void setServerPort(int port) { this->serverPort_ = port; }
  void setIsRecvMsg(bool isRecv) { this->isRecvMsg_ = isRecv; }

  const char *getServerIp() const { return this->serverIp_; };
  const int getServerPort() const { return this->serverPort_; }

  // 等待连接成功
  // @param timeoutSec 最大等待时间
  bool waitforConnected(int timeoutSec);

  // 建立连接，如果断开会再次重连，直到连接成功或超时
  bool autoConnect(int timeoutSec);

  bool isConnecting() const { return isConnecting_; }
  bool isConnected() const { return isConnected_; }

  void setAutoDelete(bool is) { this->autoDelete_ = is; }


  // 设置 SSL 通信上下文，如果使用了，就使用 SSL 加密通信
  void setSslCtx(CSSLCtx *ctx) { this->sslCtx_ = ctx; }
  CSSLCtx *sslCtx() { return this->sslCtx_; }

protected:
  char readBuf_[4096] = { 0 };        // 读缓冲区

private:
  CSSLCtx *sslCtx_ = nullptr;         // ssl 通信的上下文
  bool autoDelete_ = true;            // 连接断开时是否清理对象
  bool initBev(int sock); 
  char serverIp_[16] = { 0 };         // 服务器 ip 地址
  int serverPort_ = 0;                // 服务器端口号
  struct bufferevent *bev_ = nullptr;

  CMsg msg_;                          // 消息缓存
  bool isRecvMsg_ = true;             // 是否接收消息

  // 客户端的连接状态 1未处理(开始连接) 2连接中(等待连接成功) 3已连接(可以做业务操作) 4连接后失败(等待间隔时间后连接)
  bool isConnecting_ = true;
  bool isConnected_  = false;
  std::mutex *mtx_ = nullptr;
  struct event *timerEvent_ = nullptr;
};


#endif // !CCOME_TASK_H

