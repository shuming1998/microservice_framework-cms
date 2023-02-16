#include "cmsgevent.h"
#include "clogclient.h"
#include "cmsgcom.pb.h"
#include "ctools.h"
#include <string>
#include <iostream>
#include <sstream>
#include <map>

// 同一个类型只能有一个回调函数
static std::map<cmsg::CMsgType, CMsgEvent::msgCbFunc> msgCb;

void CMsgEvent::regCb(cmsg::CMsgType type, msgCbFunc func) {
  if (msgCb.find(type) != msgCb.end()) {
    std::stringstream ss;
    ss << "regCb error: " << type << " has set cb";
    LOG_ERROR(ss.str().c_str());
    return;
  }

  msgCb[type] = func;
}

void CMsgEvent::close() {
  clear();
  CComeTask::closeBev();
}

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

void CMsgEvent::readCb(cmsg::CMsgHead *head, CMsg *msg) {
  // 收到消息，根据消息类型调用消息的回调函数
  auto funcPt = msgCb.find(head->msg_type());
  if (funcPt == msgCb.end()) {
    clear();
    LOG_DEBUG("msg cbfunc not set!");
    return;
  }

  auto func = funcPt->second;
  (this->*func)(head, msg);
}

// 循环接收多条消息
void CMsgEvent::readCb() {
  // TODO：如果线程退出是否会出错?
  while (1) {
    if (!recvMsg()) {
      clear();
      return;
    }
    if (!pbHead_) {
      return;
    }
    auto msg = getMsg();
    if (!msg) {
      return;
    }
    // 在这里会调用注册的回调函数，所以 router 那边只需要注册，不需要调用

    // 避免死循环
    if (pbHead_->msg_type() != cmsg::MSG_ADD_LOG_REQ) {
      std::string ss;
      ss = "CMsgEvent::readCb(): ";
      ss += pbHead_->service_name();
      LOG_DEBUG(ss.c_str());
    }

    readCb(pbHead_, msg);
    clear();
  }
}

bool CMsgEvent::recvMsg() {
  // 解包
  // 1 消息头大小
  if (!head_.size_) {
    int len = readMsg(&head_.size_, sizeof(head_.size_));
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
    int len = readMsg(head_.data_ + head_.recvSize_, head_.size_ - head_.recvSize_);
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
    if (!pbHead_) {
      pbHead_ = new cmsg::CMsgHead();
    }
    if (!pbHead_->ParseFromArray(head_.data_, head_.size_)) {
      std::cerr << "pbHead.ParseFromArray failed!\n";
      return false;
    }

    // 处理空包数据
    if (pbHead_->msg_size() == 0) {
      LOG_DEBUG("recv msg which size = 0!");
      msg_.type_ = pbHead_->msg_type();
      msg_.size_ = 0;
      return true;
    } else {  // 正常处理消息
      // 鉴权
      // 获取消息内容大小分配空间
      bool is = msg_.alloc(pbHead_->msg_size());
      if (!is) {
        std::cerr << "msg_.alloc failed!\n";
        return false;
      }
    }
    // 设置消息类型
    msg_.type_ = pbHead_->msg_type();
  }

  // 3 开始接收消息内容
  if (!msg_.recved()) {
    int len = readMsg(msg_.data_ + msg_.recvSize_, msg_.size_ - msg_.recvSize_);
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

bool CMsgEvent::sendMsg(cmsg::CMsgHead *head, CMsg *msg) {
  if (!head || !msg) {
    return false;
  }
  head->set_msg_size(msg->size_);
  // 消息头序列化
  std::string headStr = head->SerializeAsString();
  int headSize = headStr.size();

  // 1 发送消息头大小 暂不考虑字节序问题
  int res = writeMsg(&headSize, sizeof(headSize));
  if (!res) {
    return false;
  }

  // 2 发送消息头(pb 序列化) CMsgHead(其中要设置即将发送的消息内容大小)
  res = writeMsg(headStr.data(), headStr.size());
  if (!res) {
    return false;
  }

  res = writeMsg(msg->data_, msg->size_);
  if (!res) {
    return false;
  }

  return true;
}

bool CMsgEvent::sendMsg(cmsg::CMsgHead *head, const google::protobuf::Message *message) {
  if (!message || !head) {
    return false;
  }

  // 封包
  // 消息内容序列化
  std::string msgStr = message->SerializeAsString();
  int msgSize = msgStr.size();
  CMsg msg;
  msg.data_ = (char *)msgStr.data();
  msg.size_ = msgSize;
  return sendMsg(head, &msg);

  //head->set_msg_size(msgSize);

  //// 消息头序列化
  //std::string headStr = head->SerializeAsString();
  //int headSize = headStr.size();

  //// 1 发送消息头大小 暂不考虑字节序问题
  //int res = writeMsg(&headSize, sizeof(headSize));
  //if (!res) {
  //  return false;
  //}

  //// 2 发送消息头(pb 序列化) CMsgHead(其中要设置即将发送的消息内容大小)
  //res = writeMsg(headStr.data(), headStr.size());
  //if (!res) {
  //  return false;
  //}

  //// 3 发送消息内容(pb 序列化) 业务 proto
  //res = writeMsg(msgStr.data(), msgStr.size());
  //if (!res) {
  //  return false;
  //}

  //return true;
}

bool CMsgEvent::sendMsg(cmsg::CMsgType type, const google::protobuf::Message *message) {
  if (!message) {
    return false;
  }
  cmsg::CMsgHead head;
  head.set_msg_type(type);

  return sendMsg(&head, message);
}