#ifndef CSERVICE_PROXY_CLIENT_H
#define CSERVICE_PROXY_CLIENT_H

#include "cserviceclient.h"
#include <map>

class CServiceProxyClient : public CServiceClient {
public:
  virtual void readCb(cmsg::CMsgHead *head, CMsg *msg);

  // 发送数据，添加标识至 callbackTasks_
  virtual bool sendMsg(cmsg::CMsgHead *head, CMsg *msg, CMsgEvent *ev);

  // 注册一个事件
  void regEvent(CMsgEvent *ev);

  // 清理一个事件
  void delEvent(CMsgEvent *ev);

private: 
  // 消息转发的对象，一个 proxy 对应多个 CMsgEvent
  // 用指针的值作为 key，要兼容 64 位
  std::map<long long, CMsgEvent *> callbackTasks_;    

  std::mutex callbackTasksMtx_;
};


#endif

