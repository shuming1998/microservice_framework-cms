#ifndef CSERVICE_H
#define CSERVICE_H

#include "ctask.h"
#include "cthreadpool.h"
#include "cservicehandle.h"

class CCOME_API CService : public CTask {
public:
  CService();
  ~CService();
  
  // 每个连接进入后，调用此函数创建处理对象，加入到线程池，需要重载
  virtual CServiceHandle *createServiceHandle() = 0;

  // 服务初始化，由线程池调用
  bool init();

  // 开始服务运行，接收连接任务，加入到线程池
  bool start();

  void listenCb(int clientSock, struct sockaddr *addr, int socklen);

  // 设置服务器监听端口
  void setServerPort(int port) { serverPort_ = port; }

  // 设置 SSL 通信上下文，如果使用了，就使用 SSL 加密通信
  void setSslCtx(CSSLCtx *ctx) { this->sslCtx_ = ctx; }
  CSSLCtx *sslCtx() { return this->sslCtx_; }

private:
  CSSLCtx *sslCtx_ = nullptr;                     // ssl 通信的上下文

  int threadCount_ = 10;                          // 处理用户数据的线程池开辟的线程数
  int serverPort_ = 0;                            // 服务器监听端口

  CThreadPool *threadPoolForListen_ = nullptr;    // 用于接收用户连接的线程池，开辟一个线程即可
  CThreadPool *threadPoolForClient_ = nullptr;    // 用于处理用户数据的线程池

};

#endif // ! CSERVICE_H

