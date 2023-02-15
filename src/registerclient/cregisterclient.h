#ifndef CREGISTER_CLIENT_H
#define CREGISTER_CLIENT_H
#include "cserviceclient.h"

// 注册中心客户端在 windows 中直接引用文件
class CRegisterClient : public CServiceClient {
public:
  static CRegisterClient *get() {
    static CRegisterClient *client = nullptr;
    if (!client) {
      client = new CRegisterClient();
    }
    return client;
  }

  // 连接成功的消息回调，由业务类重载
  virtual void connetedCb() override;

  // 向注册中心注册服务，此函数需要第一个调用，建立连接
  // @param serviceName 微服务名称
  // @param port 微服务端口
  // @param ip 微服务ip，如果传递 null,则采用客户端连接地址
  void registerServer(const char *serviceName, int port, const char *ip);

  // 发出获取微服务信息列表的请求
  // @param serviceName 微服务名称，如果为 Null 或者 "all"，则请求获取所有微服务
  void getServiceReq(const char *serviceName);

  // 获取所有的服务列表，复制原数据，每次都清理上次的复制数据
  // 此函数和操作 cmsg::CServiceMap 数据的线程在一个线程中
  // 从 pbServiceMap copy 了一份服务列表供用户访问，避免了多线程访问问题
  cmsg::CServiceMap *getAllService();

  // 获取指定服务名称的微服务列表 (阻塞)
  // @param serviceName 服务名称
  // @param timeoutSec 超时时间
  // @return 服务列表
  // @brief ①等待连接成功 ②发送获取微服务的消息 ③等待微服务列表消息反馈(有可能拿到上一次的配置)
  cmsg::CServiceMap::CServiceList getServices(const char *serviceName, int timeoutSec);


  // 接收注册服务的响应消息
  void registerRes(cmsg::CMsgHead *head, CMsg *msg);

  // 接收获取服务列表的响应消息
  void getServiceRes(cmsg::CMsgHead *head, CMsg *msg);

  //定时器，用于发送心跳
  virtual void timerCb();

  // 注册上面的回调函数
  static void regMsgCallback() {
    regCb(cmsg::MSG_REGISTER_RES, (msgCbFunc)&CRegisterClient::registerRes);
    regCb(cmsg::MSG_GET_SERVICE_RES, (msgCbFunc)&CRegisterClient::getServiceRes);
  }

private:
  CRegisterClient() {}
  // 读取本地缓存，线程不安全，在调用之前需要在外部对 pbServiceMap 加锁
  bool loadLocalFile();

  char serviceName_[32] = { 0 };
  char serviceIp_[16] = { 0 };
  int servicePort_ = 0;
};

#endif // ! CREGISTER_CLIENT_H

