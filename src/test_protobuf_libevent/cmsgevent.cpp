#include "cmsgevent.h"
#include "cmsgcom.pb.h"
#include <event2/bufferevent.h>
#include <string>
#include <iostream>

void CMsgEvent::clear() {
  head_.clear();
  msg_.clear();
}

CMsg *CMsgEvent::getMsg() {
  if (msg_.recved()) {
    return &msg_;
  }
  return nullptr;
}

bool CMsgEvent::recvMsg() {
  if (!bev_) {
    std::cerr << "CMsgEvent::recvMsg() failed：bev not set" << '\n';
    return false;
  }
  // 解包
  // 1 消息头大小
  if (!head_.size_) {
    int len = bufferevent_read(bev_, &head_.size_, sizeof(head_.size_));
    if (len <= 0 || head_.size_ <= 0) {
      return false;
    }
    if (!head_.alloc(head_.size_)) {
      std::cerr << "head_.alloc failed!\n";
      return false;
    }
  }
  
  // 2 开始接收消息头(鉴权，消息大小)
  if (!head_.recved()) {
    // 第二次进来后从上次的位置开始读
    int len = bufferevent_read(bev_, head_.data_ + head_.recvSize_, head_.size_ - head_.recvSize_);
    if (len <= 0) {
      return true;
    }
    head_.recvSize_ += len;
    // 如果数据还是没有接收完成，继续下一次接收
    if (!head_.recved()) {
      return true;
    }
    // 完整的头部数据接收完成
    // 反序列化
    cmsg::CMsgHead pbHead;
    if (!pbHead.ParseFromArray(head_.data_, head_.size_)) {
      std::cerr << "pbHead.ParseFromArray failed!\n";
      return false;
    }

    // 鉴权
    // 获取消息内容大小分配空间
    if (!msg_.alloc(pbHead.msg_size())) {
      std::cerr << "msg_.alloc failed!\n";
      return false;
    }
    // 设置消息类型
    msg_.type_ = pbHead.msg_type();
  }
  
  // 3 开始接收消息内容
  if (!msg_.recved()) {
    int len = bufferevent_read(bev_, msg_.data_ + msg_.recvSize_, msg_.size_ - msg_.recvSize_);
    if (len <= 0) {
      return true;
    }
    msg_.recvSize_ += len;
  }
  if (msg_.recved()) {
    std::cout <<"msg_ recved()" << '\n';
  }
  return true;
}

void CMsgEvent::sendMsg(cmsg::CMsgType type, const google::protobuf::Message *message) {
  if (!bev_ || !message) {
    return;
  }

  // 消息内容
  std::string msgStr = message->SerializeAsString();
  int msgSize = msgStr.size();
  cmsg::CMsgHead head;
  head.set_msg_type(type);
  head.set_msg_size(msgSize);

  // 消息头
  std::string headStr = head.SerializeAsString();
  int headSize = headStr.size();

  // 1 发送消息头大小 暂不考虑字节序问题
  bufferevent_write(bev_, &headSize, sizeof(headSize));

  // 2 发送消息头 CMsgHead(其中要设置即将发送的消息内容大小)
  bufferevent_write(bev_, headStr.data(), headStr.size());

  // 3 发送消息内容 
  bufferevent_write(bev_, msgStr.data(), msgStr.size());
}