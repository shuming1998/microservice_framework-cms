#ifndef CMSG_EVENT_H
#define CMSG_EVENT_H
#include "cmsg.h"
#include "ccometask.h"
#include "cmsgtype.pb.h"
#include "cmsgcom.pb.h"

// 不再调用 bufferevent 接口。直接调用 CComeTask 封装
class CCOME_API CMsgEvent : public CComeTask {
public:
  // 接收消息，分发消息
  virtual void readCb();
  // 消息回调函数，默认发送到用于注册的函数，由路由重载
  virtual void readCb(cmsg::CMsgHead *head, CMsg *msg);

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
  // @return false: bev未设置等原因
  virtual bool sendMsg(cmsg::CMsgType type, const google::protobuf::Message *message);
  virtual bool sendMsg(cmsg::CMsgHead *head, const google::protobuf::Message *message);
  virtual bool sendMsg(cmsg::CMsgHead *head, CMsg *msg);

  // 清理缓存，用于接收下一次消息
  void clear();

  void close();

  typedef void(CMsgEvent::*msgCbFunc)(cmsg::CMsgHead *head, CMsg *msg);

  // 添加消息处理的回调函数，根据消息类型分发   同一个类型只能有一个回调函数
  static void regCb(cmsg::CMsgType type, msgCbFunc func);

private:
  CMsg head_;   //消息头
  CMsg msg_;    // 消息内容

  cmsg::CMsgHead *pbHead_ = nullptr; // pb 消息头
};

#endif // !CMSG_EVENT_H

