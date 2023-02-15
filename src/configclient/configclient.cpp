#include "cconfigclient.h"
#include "cregisterclient.h"
#include "cmsgcom.pb.h"
#include "ctools.h"
#include <iostream>
#include <thread>
#include <string>

#define REG CRegisterClient::get()
#define CONF CConfigClient::get()


void configTimer() {
  static std::string configIp = "";
  static int configPort = 0;

  // 读取配置项 //
  std::cout << "==========root = " << CONF->getString("root") << "==========\n";

  if (configPort <= 0) {
    // 从注册中心获取配置中心的 ip
    auto configs = REG->getServices(CONFIG_NAME, 1);
    std::cout << configs.DebugString() << '\n';
    if (configs.service_size() <= 0) {
      return;
    }
    auto config = configs.service()[0];
    if (config.ip().empty() || config.port() <= 0) {
      return;
    }
    configIp = config.ip();
    configPort = config.port();
    CONF->setServerIp(configIp.c_str());
    CONF->setServerPort(configPort);
    CONF->connect();
  }
}

int main(int argc, char *argv[]) {
  int clientPort = 4000;
  // 设置注册中心的 ip 和 端口
  REG->setServerIp("127.0.0.1");
  REG->setServerPort(REGISTER_PORT);
  // 把自己注册到注册中心
  REG->registerServer("test_config", clientPort, 0);
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  // 初始化配置中心
  cmsg::CDirConfig tmpConf;
  //CONF->startGetConfig(config.ip().c_str(), config.port(), 0, clientPort, &tmpConf);
  CONF->startGetConfig(0, clientPort, &tmpConf, configTimer);
  std::this_thread::sleep_for(std::chrono::milliseconds(200));

  // 存储配置项 //
  std::string proto;
  // 通过反射生成 msg
  auto msg = CONF->loadProto("cmsgcom.proto", "CDirConfig", proto);
  // 通过反射设置值
  auto ref = msg->GetReflection();
  auto field = msg->GetDescriptor()->FindFieldByName("root");
  ref->SetString(msg, field, "test_new_root");
  std::cout << msg->DebugString();

  // 存储配置
  cmsg::CConfig conf;
  conf.set_servicename("test_config");
  conf.set_serviceport(clientPort);
  conf.set_proto(proto);
  conf.set_privatepb(msg->SerializePartialAsString());
  CONF->uploadConfig(&conf);

  // 读取配置项 //
  //std::cout << "=====root = " << CONF->getString("root") << "=====\n";

  // 读取配置列表(管理工具使用) //
  for (int i = 0; i < 1; ++i) {
    // 获取配置列表
    auto confs = CONF->downloadAllConfig(1, 1000, 10);
    std::cout << "=====================================" << '\n';
    std::cout << confs.DebugString() << '\n';
    if (confs.config_size() <= 0) {
      std::this_thread::sleep_for(std::chrono::seconds(2));
      continue;
    }

    // 取得单个配置信息(第一个配置项)
    std::string ip = confs.config()[0].serviceip();
    int port = confs.config()[0].serviceport();
    CONF->downloadConfig(ip.c_str(), port);
    cmsg::CConfig saveConf;
    CONF->getConfig(ip.c_str(), port, &saveConf);
    std::cout << "11111111111111111111111111111111111" << '\n';
    std::cout << saveConf.DebugString() << '\n';
    std::this_thread::sleep_for(std::chrono::seconds(2));
  }

  //CConfigClient::get()->regMsgCallback();
  //CConfigClient::get()->setServerIp("127.0.0.1");
  //CConfigClient::get()->setServerPort(CONFIG_PORT);
  //CConfigClient::get()->startConnect();
  //if (!CConfigClient::get()->waitforConnected(10)) {
  //  LOG_DEBUG("连接配置中心失败!");
  //} else {
  //  LOG_DEBUG("连接配置中心成功!");
  //}

  //cmsg::CConfig conf;
  //conf.set_servicename("test_client_name");
  //conf.set_serviceip("127.0.0.1");
  //conf.set_serviceport(20030);
  //conf.set_proto("message");
  //conf.set_privatepb("test pb");
  //CConfigClient::get()->uploadConfig(&conf);
  //CConfigClient::get()->downloadConfig("127.0.0.1", 20030);

  //// 从配置中心客户端获取配置
  //std::this_thread::sleep_for(std::chrono::milliseconds(300));
  //cmsg::CConfig tmpConfig;
  //CConfigClient::get()->getConfig("127.0.0.1", 20030, &tmpConfig);
  //std::cout << "==============tmpConfig==============\n";
  //std::cout << tmpConfig.DebugString() << '\n';

  //// 从配置中心获取所有配置
  //auto configs = CConfigClient::get()->downloadAllConfig(1, 2, 10);
  //std::cout << configs.DebugString() << '\n';
  CConfigClient::get()->wait();
  return 0;
}