#ifndef CCLIENT_EVENT_H
#define CCLIENT_EVENT_H

#include "cmsgevent.h"
#include "cmsgtype.pb.h"
#include <map>

class CClientEvent : public CMsgEvent {
public:
  typedef void(CClientEvent::*msgCbFunc)(const char *data, int size);

  // 初始化回调函数
  static void init();

  // 接收登录反馈消息
  // @param data 消息数据
  // @param size 消息大小
  void loginRes(const char *data, int size);

  // 注册消息回调函数，只需要注册一次，存在静态 map 中
  // @param type 消息类型
  // @param func 消息回调函数
  static void regCb(cmsg::CMsgType type, msgCbFunc func) {
    cbs_[type] = func;
  }

  // 通过类型和成员函数调用函数
  // @param type 消息类型
  // @param data 消息数据
  // @param size 消息大小
  void callFunc(cmsg::CMsgType type, const char *data, int size) {
    if (cbs_.find(type) != cbs_.end()) {
      (this->*cbs_[type])(data, size);
    }
  }

private:
  static std::map<cmsg::CMsgType, msgCbFunc> cbs_;
};

#endif // ! CCLIENT_EVENT_H

