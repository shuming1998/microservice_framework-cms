#ifndef CCOME_TASK_H
#define CCOME_TASK_H
#include "cmsg.h"
#include "ctask.h"
#include <string>

class CCOME_API CComeTask : public CTask {
public:
  bool init() override;
  virtual void closeBev();
  void setServerIp(std::string ip) { this->serverIp_ = ip; }
  void setServerPort(int port) { this->serverPort_ = port; }

  // 事件回调函数
  virtual void eventCb(short what);

  // 读取数据回调
  virtual void readCb();

  // 写入数据回调
  virtual void writeCb() {}

  // 接收到消息后处理消息的回调，由业务类重载
  // 返回 true 正常，返回 false 退出当前的消息处理，不处理下一条消息
  virtual bool readCb(const CMsg *msg) = 0;

  // 发送消息
  virtual bool writeMsg(const CMsg *msg);

  // 写入文件
  virtual bool writeMsg(const void *data, int size);

  // 连接成功的消息回调
  virtual void connetedCb() {}

  // 关闭消息接收时，数据将发送到此函数，由业务模块重载
  virtual void readCb(void *data, int size) {}

  // 激活写入回调
  virtual void beginWrite();

  void setIsRecvMsg(bool isRecv) { this->isRecvMsg_ = isRecv; }


protected:
  char readBuf_[4096] = { 0 };         // 读缓冲区

private:
  std::string serverIp_ = "";         // 服务器 ip 地址
  int serverPort_ = 0;                // 服务器端口号
  struct bufferevent *bev_ = nullptr;

  CMsg msg_;                          // 消息缓存
  bool isRecvMsg_ = true;             // 是否接收消息

};


#endif // !CCOME_TASK_H

