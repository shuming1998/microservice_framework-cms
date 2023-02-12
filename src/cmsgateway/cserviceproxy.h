#ifndef CSERVICE_PROXY
#define CSERVICE_PROXY

#include "cserviceproxyclient.h"
#include <map>
#include <string>
#include <vector>

class CServiceProxy {
public:
  static CServiceProxy *get() {
    static CServiceProxy cs;
    return &cs;
  }
  // 初始化微服务列表(从注册中心获取(也是一个微服务))，建立连接
  bool init();

  // 负载均衡找到客户端连接，进行数据发送
  bool sendMsg(cmsg::CMsgHead *head, CMsg *msg, CMsgEvent *ev);

  // 清理消息回调(客户端连接提前断开，防止微服务调用已断开连接的客户端的消息回调)
  void delEvent(CMsgEvent *ev);

  // 开启自动重连的线程
  void start();

  // 停止线程
  void stop();

  // 
  void mainFunc();

private:
  bool isExit_ = false;

  std::map<std::string, std::vector<CServiceProxyClient *> > clientMap_;  // 存放与各个微服务的连接对象的 map
  std::mutex clientMapMtx_;                                               // clientMap_ 互斥锁
  std::map<std::string, int> clientMapLastIdx_;                           // 记录上一次轮询的索引
  std::map<CMsgEvent *, CServiceProxyClient *> clientCbs_;                // 用于清理 callback 缓存
  std::mutex clientCbsMtx_;                                               // clientCbs_ 互斥锁
};


#endif

