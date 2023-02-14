#include "cregisterhandle.h"
#include "cmsgcom.pb.h"
#include "ctools.h"

// 注册服务列表的缓存，使用指针避免在进入 main 函数之前初始化对象不好排查错误
static cmsg::CServiceMap *serviceMap = nullptr;
// 用于互斥访问 
static std::mutex serviceMapMtx;

void CRegisterHandle::registerReq(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("CRegisterHandle::registerReq");

  // 回应的消息
  cmsg::CMessageRes res;
  // 解析请求
  cmsg::CRegisterReq req;
  std::string error;
  if (!req.ParseFromArray(msg->data_, msg->size_)) {
    error = "CRegisterReq ParseFromArray failed!";
    LOG_DEBUG(error.c_str());
    res.set_return_(cmsg::CMessageRes::ERROR);
    res.set_msg(error);
    sendMsg(cmsg::MSG_REGISTER_RES, &res);
    return;
  }

  // 接收到用户的服务名称、IP、端口号
  std::string serviceName = req.name();
  if (serviceName.empty()) {
    error = "serviceName is empty!";
    LOG_DEBUG(error.c_str());
    res.set_return_(cmsg::CMessageRes::ERROR);
    res.set_msg(error);
    sendMsg(cmsg::MSG_REGISTER_RES, &res);
    return;
  }

  std::string serviceIp = req.ip();
  if (serviceIp.empty()) {
    // 如果服务未设置 ip ，直接设置为客户端的 ip
    LOG_DEBUG("serviceIp is empty: use client ip!");
    serviceIp = this->clientIp();
  }

  int servicePort = req.port();
  if (servicePort <= 0 || servicePort >= 65536) {
    std::stringstream ss;
    ss << "invalid servicePort： " << servicePort << " out of range: 1 ~ 65535!";
    LOG_DEBUG(ss.str().c_str());
    res.set_return_(cmsg::CMessageRes::ERROR);
    res.set_msg(ss.str());
    sendMsg(cmsg::MSG_REGISTER_RES, &res);
    return;
  }

  // 已正确接收用户注册信息
  std::stringstream ss;
  ss << "registerReq: " << serviceName << "->" << serviceIp << ":" << servicePort;
  LOG_INFO(ss.str().c_str());

  // 存储用户注册信息，如果已经注册就更新信息(或者跳过)
  {
    std::lock_guard<std::mutex> guard(serviceMapMtx);
    if (!serviceMap) {
      serviceMap = new cmsg::CServiceMap();
    }

    // 获取 map 的指针
    auto pbMap = serviceMap->mutable_servicemap();
    // 判断 map 中是否已经注册同类型微服务
    // 集群微服务，获取该类型微服务的列表
    auto serviceList = pbMap->find(serviceName);
    if (serviceList == pbMap->end()) {
      // 该微服务未注册过
      (*pbMap)[serviceName] = cmsg::CServiceMap::CServiceList();
      serviceList = pbMap->find(serviceName);

    } // lock_guard
    // 已经注册过，查找是否有相同 ip 和 port 的微服务
    // 获取 pb service 列表的指针
    auto pbServiceRted = serviceList->second.mutable_service();
    // 遍历 pb service 列表中的 service
    for (auto service : (*pbServiceRted)) {
      if (service.ip() == serviceIp && service.port() == servicePort) {
        std::stringstream ss;
        ss << "service [" << serviceName.c_str() << "->" << serviceIp << ':' << servicePort << "] already registed!";
        LOG_DEBUG(ss.str().c_str());
        res.set_return_(cmsg::CMessageRes::ERROR);
        res.set_msg(ss.str());
        sendMsg(cmsg::MSG_REGISTER_RES, &res);
        return;
      }
    }
    // 微服务没有注册，添加新的微服务
    auto pService = serviceList->second.add_service();
    pService->set_ip(serviceIp);
    pService->set_port(servicePort);
    pService->set_name(serviceName);
    std::stringstream ss;
    ss << "service [" << serviceName.c_str() << "->" << serviceIp << ':' << servicePort << "] registe success!";
    LOG_DEBUG(ss.str().c_str());
  } // lock_guard

  res.set_return_(cmsg::CMessageRes::OK);
  res.set_msg("OK");
  sendMsg(cmsg::MSG_REGISTER_RES, &res);
}

void CRegisterHandle::getServiceReq(cmsg::CMsgHead *head, CMsg *msg) {
  // 暂时只发送全部服务
  LOG_DEBUG("CRegisterHandle::getServiceReq");

  cmsg::CGetServiceReq req;

  // 用于错误处理
  cmsg::CServiceMap res;
  res.mutable_res()->set_return_(cmsg::CMessageRes_CReturn_ERROR);

  if (!req.ParseFromArray(msg->data_, msg->size_)) {
    std::stringstream ss;
    ss << "req.ParseFromArray failed!";
    LOG_DEBUG(ss.str().c_str());
    res.mutable_res()->set_msg(ss.str().c_str());
    sendMsg(cmsg::MSG_GET_SERVICE_RES, &res);
    return;
  }

  // 返回单种还是全部
  serviceMap->set_type(req.type());

  std::string serviceName = req.name();
  std::stringstream ss;
  ss << "CRegisterHandle::getServiceReq service name: " << serviceName;
  LOG_DEBUG(ss.str().c_str());

  // 发送全部微服务数据
  {
    std::lock_guard<std::mutex> guard(serviceMapMtx);
    serviceMap->mutable_res()->set_return_(cmsg::CMessageRes_CReturn_OK);
    sendMsg(cmsg::MSG_GET_SERVICE_RES, serviceMap);
  }
}