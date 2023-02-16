#include "cregisterclient.h"
#include "clogclient.h"
#include "cmsgcom.pb.h"
#include "ctools.h"
#include <fstream>
#include <thread>

// 缓存的服务列表
static cmsg::CServiceMap *pbServiceMap = nullptr;
// 本地缓存的服务列表
static cmsg::CServiceMap *pbClientMap = nullptr;
static std::mutex pbServiceMapMtx;

void CRegisterClient::timerCb() {
  static long long count = 0;
  count++;
  cmsg::CMsgHeart req;
  req.set_count(count);
  sendMsg(cmsg::MSG_HEART_REQ, &req);
}

// 连接成功的消息回调
void CRegisterClient::connetedCb() {
  // 发送注册消息
  LOG_DEBUG("CRegisterClient::connetedCb()");
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

  // 设置自动重连
  setAutoConnect(true);

  // 设置心跳定时器时间
  setTimerMs(3000);

  // 添加默认的 IP 和端口
  if (getServerIp()[0] == '\0') {
    setServerIp("127.0.0.1");
  }
  if (getServerPort() <= 0) {
    setServerPort(REGISTER_PORT);
  }

  // 把任务加入到线程池中
  startConnect();
  loadLocalFile();
}

void CRegisterClient::getServiceReq(const char *serviceName) {
  LOG_DEBUG("CRegisterClient::getServiceReq");
  cmsg::CGetServiceReq req;
  if (serviceName) {
    req.set_type(cmsg::CServiceType::ONE);
    req.set_name(serviceName);
  } else {
    req.set_type(cmsg::CServiceType::ALL);
  }
  sendMsg(cmsg::MSG_GET_SERVICE_REQ, &req);
}

cmsg::CServiceMap *CRegisterClient::getAllService() {
  std::lock_guard<std::mutex> guard(pbServiceMapMtx);
  loadLocalFile();
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
    LOG_DEBUG("wait for connect timeout!");
    // 在连接等待超时的时候，读取本地缓存
    // 只有第一次读取缓存
    std::lock_guard<std::mutex> guard(pbServiceMapMtx);
    if (!pbServiceMap) {
      loadLocalFile();
    }
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
  LOG_DEBUG("CRegisterClient::registerRes");
  cmsg::CMessageRes res;
  if (!res.ParseFromArray(msg->data_, msg->size_)) {
    LOG_DEBUG("CRegisterClient::registerRes failed: CMessageRes ParseFromArray error!");
    return;
  }

  if (res.return_() == cmsg::CMessageRes::OK) {
    LOG_DEBUG("registe service success!");
      return;
  }

  std::stringstream ss;
  ss << "registe service failed: " << res.msg();
  LOG_DEBUG(ss.str().c_str());
}

void CRegisterClient::getServiceRes(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("CRegisterClient::getServiceRes");
  std::lock_guard<std::mutex> guard(pbServiceMapMtx);
  // 是否替换全部缓存
  bool isAll = false;
  cmsg::CServiceMap *cacheServiceMap;
  cmsg::CServiceMap tmpMap;
  cacheServiceMap = &tmpMap;
  // 将服务存储到本地
  if (!pbServiceMap) {
    pbServiceMap = new cmsg::CServiceMap();
    cacheServiceMap = pbServiceMap;
    isAll = true;
  }
  if (!cacheServiceMap->ParseFromArray(msg->data_, msg->size_)) {
    LOG_DEBUG("cmsg::CServiceMap ParseFromArray failed!");
    return;
  }

  // 区分是获取一种还是全部，判断如何刷新缓存
  if (cacheServiceMap->type() == cmsg::CServiceType::ALL) {
    isAll = true;
  }

  // 内存缓存刷新
  if (cacheServiceMap == pbServiceMap) {
    // 内存缓存已经刷新
  } else {
    if (isAll) {
      pbServiceMap->CopyFrom(*cacheServiceMap);
    } else {
      // 将刚读取的数据 cacheMap 存储 pbServiceMap 内存缓存中
      auto cacheMap = cacheServiceMap->mutable_servicemap();
      if (!cacheMap || cacheMap->empty()) {
        return;
      }
      // 只取第一个
      auto one = cacheMap->begin();
      auto sMap = pbServiceMap->mutable_servicemap();
      // 修改缓存
      (*sMap)[one->first] = one->second;
    }
  }

  // 磁盘缓存刷新，后期应考虑刷新频率，判断刷新策略(是否与上一次有区别)
  // 生成名称
  std::stringstream ss;
  ss << "register_" << serviceName_ << serviceIp_ << servicePort_ << ".cache";
  LOG_DEBUG("Save local file!");
  if (!pbServiceMap) {
    return;
  }
  std::ofstream ofs;
  ofs.open(ss.str(), std::ios::binary);
  if (!ofs.is_open()) {
    LOG_DEBUG("save file failed!");
    return;
  }
  pbServiceMap->SerializePartialToOstream(&ofs);
  ofs.close();
}

bool CRegisterClient::loadLocalFile() {
  LOG_DEBUG("Load local register data");
  if (!pbServiceMap) {
    pbServiceMap = new cmsg::CServiceMap();
  }
  std::stringstream ss;
  ss << "register_" << serviceName_ << serviceIp_ << servicePort_ << ".cache";
  std::ifstream ifs;
  ifs.open(ss.str(), std::ios::binary);
  if (!ifs.is_open()) {
    std::stringstream log;
    log << "ParseFromIstream failed: [";
    log << ss.str();
    log << ']';
    LOG_DEBUG(log.str().c_str());
    return false;
  }

  pbServiceMap->ParseFromIstream(&ifs);
  ifs.close();
  return true;
}
