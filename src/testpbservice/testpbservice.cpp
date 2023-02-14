#include "cservice.h"
#include "cregisterclient.h"
#include "cdirservicehandle.h"
#include "cconfigclient.h"
#include "ctools.h"
#include <thread>
#include <iostream>

class CTestService : public CService {
public:
  CServiceHandle *createServiceHandle() {
    return new CDirServiceHandle();
  }
};

int main(int argc, char *argv[]) {
  std::cout << "Run test_pbservice like this: ./[executable file name] [server port] [register ip] [register port]\n";
  int serverPort = 30001;
  if (argc > 1) {
    serverPort = atoi(argv[1]);
  }
  std::cout << "server port is " << serverPort << '\n';

  std::string registerIp = "127.0.0.1";
  int registerPort = REGISTER_PORT;
  if (argc > 2) {
    registerIp = argv[2];
  }
  if (argc > 3) {
    registerPort = atoi(argv[3]);
  }


  // 设置注册中心的 IP 和 端口
  CRegisterClient::get()->setServerIp(registerIp.c_str());
  CRegisterClient::get()->setServerPort(registerPort);
  // 注册到注册中心
  CRegisterClient::get()->registerServer("dir", serverPort, 0);

  // 找到配置中心的 IP 和 端口
  // 等待配置获取成功
  auto configs = CRegisterClient::get()->getServices(CONFIG_NAME, 10);
  std::cout << "=======================================\n";
  std::cout << configs.DebugString() << '\n';
  if (configs.service_size() <= 0) {
    std::cout << "find config service failed!\n";
  } else {
    // 只取第一个配置中心
    auto conf = configs.service()[0];
    // 连接配置中心(封装到 startGetConfig 中)
    //CConfigClient::get()->regMsgCallback();
    //CConfigClient::get()->setServerIp(conf.ip().c_str());
    //CConfigClient::get()->setServerPort(conf.port());
    //CConfigClient::get()->startConnect();
    //if (!CConfigClient::get()->waitforConnected(10)) {
    //  LOG_DEBUG("连接配置中心失败!");
    //} else {
    //  LOG_DEBUG("连接配置中心成功!");
    //}

    static cmsg::CDirConfig dirConf1;
    if (CConfigClient::get()->startGetConfig(conf.ip().c_str(), 
          conf.port(), "127.0.0.1", serverPort, &dirConf1)) {
      std::cout << dirConf1.DebugString() << '\n';
    }

    // 写入测试的配置
    cmsg::CConfig uploadConf;
    uploadConf.set_servicename("dir");
    uploadConf.set_serviceip("127.0.0.1");
    uploadConf.set_serviceport(serverPort);
    // 类型描述
    cmsg::CDirConfig dirConf; // 用来测试的类型
    dirConf.set_root("./root");
    uploadConf.set_proto(dirConf.GetDescriptor()->DebugString());
    std::string dirConfPb = dirConf.SerializeAsString();
    uploadConf.set_privatepb(dirConfPb);
    CConfigClient::get()->uploadConfig(&uploadConf);

    CConfigClient::get()->downloadConfig("127.0.0.1", serverPort);
    // 从配置中心客户端获取配置
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    cmsg::CConfig tmpConfig;
    CConfigClient::get()->getConfig("127.0.0.1", serverPort, &tmpConfig);
    std::cout << "==============tmpConfig==============\n";
    std::cout << tmpConfig.DebugString() << '\n';

    // 反序列化下载的配置信息
    cmsg::CDirConfig downloadConf;\
    // 每个微服务单独的配置
    if (downloadConf.ParseFromString(tmpConfig.privatepb())) {
      std::cout << "downloadConf = " << downloadConf.DebugString() << '\n';
    }
 
  }


  // 注册消息回调函数
  CDirServiceHandle::regMsgCb();

  CTestService service;
  service.setServerPort(serverPort);
  service.start();
  CThreadPool::wait();

  return 0;
}