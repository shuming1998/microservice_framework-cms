#include "cconfigclient.h"
#include "cmsgcom.pb.h"
#include "ctools.h"
#include "clogclient.h"
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>
#include <map>
#include <thread>
#include <fstream>

#define PB_ROOT "root/"

// 用来显示解析的语法错误
class ParseError : public google::protobuf::compiler::MultiFileErrorCollector {
public:
  virtual void AddError(const std::string& filename, int line, int column,
    const std::string& message) override {
    std::stringstream ss;
    ss << '[' << filename << "|" << line << "|" << column << "]: " << message;
    LOG_DEBUG(ss.str().c_str());
  }
};
static ParseError psError;

// 用于存储从服务端下载的配置信息，业务获取配置时从此处查询 key: ip:port
static std::map<std::string, cmsg::CConfig> configMap;
static std::mutex configMapMtx;

// 用来存储当前微服务配置
static google::protobuf::Message *curServiceConfig = nullptr;
static std::mutex curServiceConfigMtx;

// 用来存储获取的全部配置列表
static cmsg::CConfigList *allConfigs = nullptr;
static std::mutex allConfigsMtx;

CConfigClient::CConfigClient() {
  // 管理文件加载路径
  sourceTree_ = new google::protobuf::compiler::DiskSourceTree();
  sourceTree_->MapPath("", "");
  // 使用绝对路径时，不加 root 会失败
  sourceTree_->MapPath(PB_ROOT, "");
}

void CConfigClient::setCurServiceMessage(google::protobuf::Message *msg) {
  std::lock_guard<std::mutex> guard(curServiceConfigMtx);
  curServiceConfig = msg;
}

google::protobuf::Message *CConfigClient::loadProto(std::string fileName, std::string className, std::string &outProtoCode) {
  // 1 加载 proto 文件
  // 需要清理空间
  if (importer_) {
    delete importer_;
  }
  importer_ = new google::protobuf::compiler::Importer(sourceTree_, &psError);
  if (!importer_) {
    return nullptr;
  }

  std::string path = PB_ROOT;
  path += fileName;
  // 返回 proto 文件描述符
  auto fileDesc = importer_->Import(path);
  if (!fileDesc) {
    return nullptr;
  }
  LOG_DEBUG(fileDesc->DebugString());
  std::stringstream ss;
  ss << fileName << "proto 文件加载成功";
  LOG_DEBUG(ss.str().c_str());

  // 获取类型描述符
  // 如果 className 为空，则使用第一个类型
  const google::protobuf::Descriptor *messageDesc = nullptr;
  if (className.empty()) {
    if (fileDesc->message_type_count() <= 0) {
      LOG_DEBUG("proto 文件中没有 message!");
      return nullptr;
    }
    // 取第一个类型
    messageDesc = fileDesc->message_type(0);
  } else {
    // 包含命名空间的类名
    std::string classNamePack = "";
    // className 不为空则查找类型，涉及到命名空间的问题，判断用户是否传入了命名空间
    // 如果用户没有传入命名空间
    if (className.find('.') == className.npos) {
      // 判断一下 proto 文件是否有命名空间
      if (fileDesc->package().empty()) {
        classNamePack = className;
      } else {
        classNamePack = fileDesc->package();
        classNamePack += ".";
        classNamePack += className;
      }
    } else {
      // 用户传入了命名空间，直接赋值
      classNamePack = className;
    }
    messageDesc = importer_->pool()->FindMessageTypeByName(classNamePack);
    if (!messageDesc) {
      std::string log = "proto 文件中没有指定的 message: ";
      log += classNamePack;
      LOG_DEBUG(log.c_str());
      return nullptr;
    }
  }

  LOG_DEBUG(messageDesc->DebugString().c_str());


  // 动态创建消息类型的工厂，不能销毁，否则由它创建的 message 对象也会销毁
  static google::protobuf::DynamicMessageFactory factory;
  // 先创建一个类型原型
  auto messageProto = factory.GetPrototype(messageDesc);
  // 反射生成 message 对象
  delete message_;
  message_ = messageProto->New();
  // 要传入数据库的 proto 文件内容格式：

  /*
  syntax="proto3";
  package xmsg;
  message CDirConfig {
    string root = 1;

  }
  */

  // syntax="proto3";
  outProtoCode = "syntax=\"";
  outProtoCode += fileDesc->SyntaxName(fileDesc->syntax());
  outProtoCode += "\";\n";

  //  package 命名空间;
  outProtoCode += "package ";
  outProtoCode += fileDesc->package();
  outProtoCode += ";\n";

  // 存枚举定义，暂时不支持import 多 proto 文件
  // 同一个类型只生成一次代码
  std::map<std::string, const google::protobuf::EnumDescriptor *> enumDesc;
  for (int i = 0; i < messageDesc->field_count(); ++i) {
    auto field = messageDesc->field(i);
    if (field->type() == google::protobuf::FieldDescriptor::TYPE_ENUM) {
      // 已经添加过该枚举类型
      if (enumDesc.find(field->enum_type()->name()) != enumDesc.end()) {
        continue;
      }
      // 找到枚举的类型
      enumDesc[field->enum_type()->name()] = field->enum_type();
      outProtoCode += field->enum_type()->DebugString();
      outProtoCode += "\n";
    }
  }

  // message 类名
  outProtoCode += messageDesc->DebugString();
  return message_;
}

bool CConfigClient::startGetConfig(const char *localIp, int localPort,
                    google::protobuf::Message *configMsg,
                    configTimerCbFunc func) {
  regMsgCallback();

  if (localIp) {
    strncpy(localIp_, localIp, 16);
  }
  localPort_ = localPort;

  // 设置当前配置类型
  setCurServiceMessage(configMsg);
  // 设置回调函数
  this->configTimerCb_ = func;
  // 设置定时器时间间隔
  this->setTimerMs(3000);

  // 读取本地缓存
  std::stringstream ss;
  ss << localPort_ << "_conf.cache";
  std::ifstream ifs;
  ifs.open(ss.str(), std::ios::binary);
  if (!ifs.is_open()) {
    LOG_DEBUG("open local config failed!");
  } else {
    if (configMsg) {
      curServiceConfig->ParseFromIstream(&ifs);
    }
    ifs.close();
  }

  // 连接配置中心，任务加入线程池
  startConnect();
  return true;
}

bool CConfigClient::startGetConfig(const char *serverIp, int serverPort, 
                                   const char *localIp, int localPort,
                                   google::protobuf::Message *configMsg, 
                                   int timeoutSec) {

  regMsgCallback();
  setServerIp(serverIp);
  setServerPort(serverPort);
  if (localIp) {
    strncpy(localIp_, localIp, 16);
  }
  localPort_ = localPort;
  // 设置当前配置类型
  setCurServiceMessage(configMsg);

  // 连接配置中心任务加入到线程池
  startConnect();

  if (!waitforConnected(timeoutSec)) {
    LOG_DEBUG("连接配置中心失败!");
    return false;
  }

  LOG_DEBUG("连接配置中心成功!");
  // 连接成功后，启动定时器(3000毫秒)
  setTimer(3000);
  
  return true;
}

bool CConfigClient::init() {
  CServiceClient::init();
  // 启动成功之后，先调用一次定时器，确保消息及时获取
  timerCb();
  return true;
}

void CConfigClient::timerCb() {
  if (configTimerCb_) {
    configTimerCb_();
  }
  // 发出获取配置的请求
  if (localPort_ > 0) {
    downloadConfig(localIp_, localPort_);
  }
}

int CConfigClient::getInt(const char *key) {
  std::lock_guard<std::mutex> guard(curServiceConfigMtx);
  if (!curServiceConfig) {
    return 0;
  }
  // 先获取字段类型
  auto field = curServiceConfig->GetDescriptor()->FindFieldByName(key);
  if (!field) {
    return 0;
  }

  // 在通过反射器获取参数值
  return curServiceConfig->GetReflection()->GetInt32(*curServiceConfig, field);
}

std::string CConfigClient::getString(const char *key) {
  std::lock_guard<std::mutex> guard(curServiceConfigMtx);
  if (!curServiceConfig) {
    return std::string("");
  }

  // 先获取字段类型
  auto field = curServiceConfig->GetDescriptor()->FindFieldByName(key);
  if (!field) {
    return std::string("");
  }
  
  // 再通过反射器获取参数值
  return curServiceConfig->GetReflection()->GetString(*curServiceConfig, field);
}

bool CConfigClient::getBool(const char *key) {
  std::lock_guard<std::mutex> guard(curServiceConfigMtx);
  if (!curServiceConfig) {
    return false;
  }

  // 先获取字段类型
  auto field = curServiceConfig->GetDescriptor()->FindFieldByName(key);
  if (!field) {
    return false;
  }

  // 再通过反射器获取参数值
  return curServiceConfig->GetReflection()->GetBool(*curServiceConfig, field);
}

bool CConfigClient::getConfig(const char *ip, int port, cmsg::CConfig *outConf, int timeoutMs) {
  // 10 毫秒判断一次
  int count = timeoutMs / 10;
  std::stringstream key;
  key << ip << ':' << port;

  for (int i = 0; i < count; ++i) {
    std::lock_guard<std::mutex> guard(configMapMtx);
    // 查找配置
    auto confIt = configMap.find(key.str());
    if (confIt == configMap.end()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      continue;
    }

    // 复制配置
    outConf->CopyFrom(confIt->second);
    return true;
  }

  LOG_DEBUG("config not find!");
  return false;
}

void CConfigClient::wait() {
  CThreadPool::wait();
}

void CConfigClient::uploadConfig(cmsg::CConfig *conf) {
  LOG_DEBUG("CConfigClient::uploadConfig");
  sendMsg(cmsg::MSG_UPLOAD_CONFIG_REQ, conf);
}

void CConfigClient::uploadConfigRes(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("CConfigClient::uploadConfigRes");
  cmsg::CMessageRes res;
  if (!res.ParseFromArray(msg->data_, msg->size_)) {
    LOG_DEBUG("ParseFromArray failed!");
    if (uploadConfigResCb_) {
      uploadConfigResCb_(false, "ParseFromArray failed!");
    }
    return;
  }

  if (res.return_() == cmsg::CMessageRes_CReturn_OK) {
    LOG_DEBUG("uploadConfig failed!");
    if (uploadConfigResCb_) {
      uploadConfigResCb_(true, "uploadConfig success!");
    }
    return;
  }

  // 上传失败
  std::stringstream ss;
  ss << "uploadConfig failed：" << res.msg();
  if (uploadConfigResCb_) {
    uploadConfigResCb_(false, ss.str().c_str());
  }
  LOG_DEBUG(ss.str().c_str());
}


void CConfigClient::downloadConfig(const char *ip, int port) {
  LOG_DEBUG("下载配置信息");
  if (port <= 0 || port >= 65536) {
    LOG_DEBUG("downloadConfig failed: port out of range!");
    return;
  }

  // 生成请求下载消息
  cmsg::CDownloadconfigReq req;
  if (ip) {
    req.set_serviceip(ip);
  }
  req.set_serviceport(port);
  // 发送消息到服务端
  sendMsg(cmsg::MSG_DOWNLOAD_CONFIG_REQ, &req);
}

void CConfigClient::downloadConfigRes(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("CConfigClient::downloadConfigRes");
  cmsg::CConfig conf;
  if (!conf.ParseFromArray(msg->data_, msg->size_)) {
    LOG_DEBUG("downloadConfigRes ParseFromArray failed!");
    return;
  }
  LOG_DEBUG(conf.DebugString().c_str());

  std::stringstream key;
  key << conf.serviceip() << ':' << conf.serviceport();

  {
    // 更新配置
    std::lock_guard<std::mutex> guard(configMapMtx);
    configMap[key.str()] = conf;
  }

  // 没有本地配置直接返回
  if (localPort_ <= 0 || !curServiceConfig) {
    return;
  }

  std::stringstream localKey;
  std::string ip = localIp_;
  if (ip.empty()) {
    ip = conf.serviceip();
  }
  localKey << ip << ':' << localPort_;
  // 不是本地的配置项直接返回
  if (key.str() != localKey.str()) {
    return;
  }

  std::lock_guard<std::mutex> guard(curServiceConfigMtx);
  if (!curServiceConfig->ParseFromString(conf.privatepb())) {
    return;
  }

  // 测试 ParseFromString bug
  cmsg::CDirConfig cconfig;
  cconfig.ParseFromString(conf.privatepb());

  // 存储配置到本地文件：[port]_conf.cache
  std::stringstream ss;
  ss << localPort_ << "_conf.cache";
  std::ofstream ofs;
  ofs.open(ss.str(), std::ios::binary);
  if (!ofs.is_open()) {
    LOG_DEBUG("save local config failed!");
    return;
  }
  curServiceConfig->SerializePartialToOstream(&ofs);
  //cconfig.SerializePartialToOstream(&ofs);
  ofs.close();
}

cmsg::CConfigList CConfigClient::downloadAllConfig(int page, int pageCount, int timeoutSec) {
  {
    // 先清理历史数据
    std::lock_guard<std::mutex> guard(allConfigsMtx);
    delete allConfigs;
    allConfigs = nullptr;
  }
  
  cmsg::CConfigList configs;
  // 1 断开连接时自动重连
  if (!autoConnect(timeoutSec)) {
    return configs;
  }

  // 2 发送获取全部配置的请求消息
  cmsg::CDownloadAllConfigReq req;
  req.set_page(page);
  req.set_pagecount(pageCount);
  sendMsg(cmsg::MSG_DOWNLOAD_ALL_CONFIG_REQ, &req);


  // 3 每 10 毫秒监听一次是否受到响应
  int milsec = timeoutSec * 100;
  for (int i = 0; i < milsec; ++i) {
    {
      std::lock_guard<std::mutex> guard(allConfigsMtx);
      if (allConfigs) {
        return *allConfigs;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  return configs;
}

void CConfigClient::downloadAllConfigRes(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("CConfigClient::downloadAllConfigRes");
  std::lock_guard<std::mutex> guard(allConfigsMtx);
  if (!allConfigs) {
    allConfigs = new cmsg::CConfigList();
  }
  allConfigs->ParseFromArray(msg->data_, msg->size_);
}

void CConfigClient::deleteConfig(const char *ip, int port) {
  if (!ip || strlen(ip) == 0 || port < 0 || port >= 65536) {
    LOG_DEBUG("deleteConfig failed: ip or port not invalid!");
    return;
  }

  // 生成删除配置消息
  cmsg::CDownloadconfigReq req;
  if (ip) {
    req.set_serviceip(ip);
  }
  req.set_serviceport(port);
  // 发送消息到服务端
  sendMsg(cmsg::MSG_DELETE_CONFIG_REQ, &req);
}

void CConfigClient::deleteConfigRes(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("CConfigClient::deleteConfigRes");
  cmsg::CMessageRes res;
  if (!res.ParseFromArray(msg->data_, msg->size_)) {
    LOG_DEBUG("deleteConfigRes ParseFromArray failed!");
    return;
  }

  if (res.return_() == cmsg::CMessageRes_CReturn_OK) {
    LOG_DEBUG("deleteConfig success!");
    return;
  }

  LOG_DEBUG("deleteConfig failed!");
}