#include "cserviceproxy.h"
#include "cmsgcom.pb.h"
#include "cregisterclient.h"
#include "ctools.h"
#include "clogclient.h"
#include <thread>

// 开启自动重连的线程
void CServiceProxy::start() {
  std::thread t(&CServiceProxy::mainFunc, this);
  t.detach();
}

// 停止线程
void CServiceProxy::stop() {
   
}

// 自动重连
void CServiceProxy::mainFunc() {
  // 从注册中心获取微服务的列表


  while (!isExit_) {
    // 从注册中心获取微服务的列表更新
    // 发送请求到注册中心
    CRegisterClient::get()->getServiceReq(0);

    auto serviceMap = CRegisterClient::get()->getAllService();
    if (!serviceMap) {
      LOG_DEBUG("CRegisterClient::get()->getAllService(): service is null!");
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }

    auto pbMap = serviceMap->servicemap();
    if (pbMap.empty()) {
      LOG_DEBUG("servicemap is empty!");
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // 遍历所有的微服务名称的列表
    for (auto pbServiceMap : pbMap) {
      // 遍历每个列表中的微服务
      std::string serviceName = pbServiceMap.first;
      // 不连接自己
      if (serviceName == API_GATEWAY_NAME) {
        continue;
      }
      for (auto pbService : pbServiceMap.second.service()) {
        std::lock_guard<std::mutex> guard(clientMapMtx_);
        // 如果该微服务名称不存在，新建一个微服务列表
        if (clientMap_.find(serviceName) == clientMap_.end()) {
          clientMap_[serviceName] = std::vector<CServiceProxyClient *>();
        }

        bool isFind = false;
        // 查看该服务名称对应的列表中是否已经有该 ip:port 注册的微服务
        for (auto connect : clientMap_[serviceName]) {
          if (pbService.ip() == connect->getServerIp() && pbService.port() == connect->getServerPort()) {
            isFind = true;
            break;
          }
        }

        if (isFind) {
          continue;
        }

        // 如果当前的微服务还没加入列表，就创建一个该微服务的对象，建立连接，加入微服务列表
        auto proxy = new CServiceProxyClient();
        proxy->setServerIp(pbService.ip().c_str());
        proxy->setServerPort(pbService.port());
        proxy->setAutoDelete(false);
        proxy->startConnect();
        clientMap_[serviceName].push_back(proxy);
        clientMapLastIdx_[serviceName] = 0;
      }
    }

    // 定时全部重新获取
    for (auto m : clientMap_) {
      for (auto c : m.second) {
        if (c->isConnected()) {
          continue;
        }
        if (!c->isConnecting()) {
          LOG_DEBUG("start connect service");
          c->connect();
        }
      }
    }
    std::this_thread::sleep_for(std::chrono::seconds(3));
  }
}

bool CServiceProxy::init() {
  // 1 从注册中心获取微服务列表

  //// 测试数据
  //cmsg::CServiceMap serviceMap;
  //// 获取 serviceMap 的指针
  //auto sMap = serviceMap.mutable_servicemap();
  //cmsg::CServiceMap::CServiceList list;
  //{
  //  auto service = list.add_service();
  //  service->set_ip("127.0.0.1");
  //  service->set_port(20011);
  //  service->set_name("dir");
  //}

  //{
  //  auto service = list.add_service();
  //  service->set_ip("127.0.0.1");
  //  service->set_port(20012);
  //  service->set_name("dir");
  //}

  //{
  //  auto service = list.add_service();
  //  service->set_ip("127.0.0.1");
  //  service->set_port(20013);
  //  service->set_name("dir");
  //}

  //(*sMap)["dir"] = list;

  //std::cout << serviceMap.DebugString() << '\n';

  //// 2 与微服务建立连接
  //// 遍历 serviceMap 数据
  //for (auto m : (*sMap)) {
  //  clientMap_[m.first] = std::vector<CServiceProxyClient *>();
  //  for (auto s : m.second.service()) {
  //    auto proxy = new CServiceProxyClient();
  //    proxy->setServerIp(s.ip().c_str());
  //    proxy->setServerPort(s.port());
  //    proxy->startConnect();
  //    clientMap_[m.first].push_back(proxy);
  //    clientMapLastIdx_[m.first] = 0;
  //  }
  //}




  return true;
}

void CServiceProxy::delEvent(CMsgEvent *ev) {
  if (!ev) {
    return;
  }
  std::lock_guard<std::mutex> guard(clientCbsMtx_);
  auto it = clientCbs_.find(ev);
  if (it == clientCbs_.end()) {
    LOG_DEBUG("can't find ev in clientCbs_!");
  }
  it->second->delEvent(ev);
}

bool CServiceProxy::sendMsg(cmsg::CMsgHead *head, CMsg *msg, CMsgEvent *ev) {
  if (!head || !msg) {
    return false;
  }
  std::string serviceName = head->service_name();
  // 1 负载均衡找到客户端连接
  std::lock_guard<std::mutex> guard(clientMapMtx_);
  auto clientList = clientMap_.find(serviceName);
  if (clientList == clientMap_.end()) {
    std::stringstream ss;
    ss << serviceName << " not find int clientMap_!";
    LOG_DEBUG(ss.str().c_str());
    return false;
  }

  // 轮询找到可用的微服务连接
  int curIdx = clientMapLastIdx_[serviceName];
  int listSize = clientList->second.size();
  for (int i = 0; i < listSize; ++i) {
    ++curIdx;
    curIdx = curIdx % listSize;
    clientMapLastIdx_[serviceName] = curIdx;
    auto client = clientList->second[curIdx];
    if (client->isConnected()) {
      
    {
      // 后续用于退出时清理回调函数缓存
      std::lock_guard<std::mutex> guard(clientCbsMtx_);
      clientCbs_[ev] = client;
    }

      // 转发消息
      return client->sendMsg(head, msg, ev);
    }
  }

  LOG_DEBUG("can't find proxy");

  return false;
}