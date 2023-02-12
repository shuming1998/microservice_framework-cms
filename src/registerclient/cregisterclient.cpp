#include "cregisterclient.h"
#include "cmsgcom.pb.h"
#include "ctools.h"

// 缓存的服务列表
static cmsg::CServiceMap *pbServiceMap = nullptr;
// 本地缓存的服务列表
static cmsg::CServiceMap *pbClientMap = nullptr;
static std::mutex pbServiceMapMtx;

// 连接成功的消息回调
void CRegisterClient::connetedCb() {
  // 发送注册消息
  LOG_DEBUG("注册中心客户连接成功， 开始发送注册请求!");
  cmsg::CRegisterReq req;
  req.set_name(serviceName_);
  req.set_ip(serviceIp_);
  req.set_port(servicePort_);
  sendMsg(cmsg::MSG_REGISTER_REQ, &req);
}

void CRegisterClient::registerServer(const char *serviceName, int port, const char *ip) {
  // 注册消息回调函数
  regMsgCallback();
  // 发送消息到服务器
  // 服务器连接是否成功 ？
  // 注册中心的IP port
  if (serviceName) {
    strcpy(serviceName_, serviceName);
  }
  if (ip) {
    strcpy(serviceIp_, ip);
  }
  servicePort_ = port;

  // 把任务加入到线程池中
  startConnect();
}

void CRegisterClient::getServiceReq(const char *serviceName) {
  LOG_DEBUG("发出获取微服务列表的请求");
  cmsg::CGetServiceReq req;
  if (serviceName) {
    req.set_type(cmsg::CGetServiceReq_CType_ONE);
    req.set_name(serviceName);
  } else {
    req.set_type(cmsg::CGetServiceReq_CType_ALL);
  }
  sendMsg(cmsg::MSG_GET_SERVICE_REQ, &req);
}

cmsg::CServiceMap *CRegisterClient::getAllService() {
  std::lock_guard<std::mutex> guard(pbServiceMapMtx);
  if (!pbServiceMap) {
    return nullptr;
  }
  if (!pbClientMap) {
    pbClientMap = new cmsg::CServiceMap();
  }
  pbClientMap->CopyFrom(*pbServiceMap);
  return pbClientMap;
}

cmsg::CServiceMap::CServiceList CRegisterClient::getServices(const char *serviceName, int timeoutSec) {
  cmsg::CServiceMap::CServiceList serviceList;
  
  // 时间粒度设置为 10 毫秒
  int totalTm = timeoutSec * 100;
  int curTm = 0;
  // 1 等待连接成功
  while (curTm < totalTm) {
    if (isConnected()) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ++curTm;
  }

  if (!isConnected()) {
    LOG_DEBUG("等待连接超时!");
    return serviceList;
  }

  // 2 发送获取微服务的消息 
  getServiceReq(serviceName);

  // 3 等待微服务列表消息反馈(有可能拿到上一次的配置) 
  while (curTm < totalTm) {
    std::lock_guard<std::mutex> guard(pbServiceMapMtx);
    if (!pbServiceMap) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      ++curTm;
      continue;
    }
    auto pMap = pbServiceMap->mutable_servicemap();
    if (!pMap) {
      // 没有找到指定的微服务
      getServiceReq(serviceName);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      curTm += 10;
      continue;
    }
    auto itServiceList = pMap->find(serviceName);
    if (itServiceList == pMap->end()) {
      // 没有找到指定的微服务
      getServiceReq(serviceName);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      curTm += 10;
      continue;
    }
    serviceList.CopyFrom(itServiceList->second);
    return serviceList;
  }
  return serviceList;
}

void CRegisterClient::registerRes(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("接收到注册服务的响应");
  cmsg::CMessageRes res;
  if (!res.ParseFromArray(msg->data_, msg->size_)) {
    LOG_DEBUG("CRegisterClient::registerRes failed: CMessageRes ParseFromArray error!");
    return;
  }

  if (res.return_() == cmsg::CMessageRes::OK) {
    LOG_DEBUG("注册微服务成功");
      return;
  }

  std::stringstream ss;
  ss << "注册微服务失败: " << res.msg();
  LOG_DEBUG(ss.str().c_str());
}

void CRegisterClient::getServiceRes(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("接收获取服务列表的响应");

  std::lock_guard<std::mutex> guard(pbServiceMapMtx);
  // 将服务存储到本地
  if (!pbServiceMap) {
    pbServiceMap = new cmsg::CServiceMap();
  }
  if (!pbServiceMap->ParseFromArray(msg->data_, msg->size_)) {
    LOG_DEBUG("cmsg::CServiceMap ParseFromArray failed!");
    return;
  }
  LOG_DEBUG(pbServiceMap->DebugString());
}