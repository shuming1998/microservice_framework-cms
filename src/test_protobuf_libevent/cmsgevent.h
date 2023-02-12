#ifndef CMSG_EVENT_H
#define CMSG_EVENT_H
#include "cmsg.h"
#include "cmsgtype.pb.h"

class CMsgEvent {
public:

  // 接收数据包
  // 1. 正确接收消息，调用消息处理函数
  // 2. 消息接收不完整，等待下一次
  // 3. 消息接收出错，退出并清理空间
  // return true: 1/2  false: 3 
  bool recvMsg();

  CMsg *getMsg();

  // 发送消息，包含头部(自动创建)
  // @param type 消息类型
  // @param message 消息内容
  void sendMsg(cmsg::CMsgType type, const google::protobuf::Message *message);

  void setBev(struct bufferevent *bev) { this->bev_ = bev; }

  // 清理缓存，用于接收下一次消息
  void clear();

private:
  struct bufferevent *bev_ = nullptr;
  CMsg head_;   //消息头
  CMsg msg_;    // 消息内容

};

#endif // !CMSG_EVENT_H

