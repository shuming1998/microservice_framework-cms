#ifndef CCONFIG_CLIENT_H
#define CCONFIG_CLIENT_H
#include "cserviceclient.h"
#include "cmsgcom.pb.h"

namespace google {
  namespace protobuf {
    namespace compiler {
      class Importer;
      class DiskSourceTree;
    }
  }
}

typedef void(*uploadConfigResCbFunc)(bool isOk, const char *msg);

class CConfigClient : public CServiceClient {
public:
  static CConfigClient *get() {
    static CConfigClient cc;
    return &cc;
  }

  // 连接配置中心，开启定时器获取配置
  // 封装：regMsgCallback、setServerIp、setServerPort、startConnect、超时等待连接
  bool startGetConfig(const char *serverIp, int serverPort, 
                      const char *localIp, int localPort,
                      google::protobuf::Message *configMsg, int timeoutSec = 10);

  // 定时器调用的回调函数
  void timerCb();

  // 供业务调用，通过此接口获取配置
  bool getConfig(const char *ip, int port, cmsg::CConfig *outConf);

  // 获取下载的本地参数
  int getInt(const char *key);
  std::string getString(const char *key);
  bool getBool(const char *key);   // isSSL

  // 上传配置请求
  void uploadConfig(cmsg::CConfig *conf);
  // 上传配置请求的响应
  void uploadConfigRes(cmsg::CMsgHead *head, CMsg *msg);

  // 下载配置请求
  // @param ip 如果为 NULL，则取客户端连接到配置中心的地址
  void downloadConfig(const char *ip, int port);
  // 下载配置请求的响应
  void downloadConfigRes(cmsg::CMsgHead *head, CMsg *msg);

  // 下载全部配置列表请求 1 断开连接时自动重连 2 阻塞等待结果返回
  cmsg::CConfigList downloadAllConfig(int page, int pageCount, int timeoutSec);
  // 下载全部配置请求的响应
  void downloadAllConfigRes(cmsg::CMsgHead *head, CMsg *msg);

  // 删除指定微服务配置
  void deleteConfig(const char *ip, int port);
  // 删除指定微服务配置的响应
  void deleteConfigRes(cmsg::CMsgHead *head, CMsg *msg);

  // 设置当前的配置对象(互斥)
  void setCurServiceMessage(google::protobuf::Message *msg);

  // 载入 proto 文件,非线程安全
  // @param fileName 文件路径
  // @param className 文件类型
  // @param outProtoCode 读取到的代码，包含命名空间和版本
  // @return 返回动态创建的消息，失败返回 nullptr，第二次调用会释放上一次的空间
  google::protobuf::Message *loadProto(std::string fileName, std::string className, std::string &outProtoCode);

  // 注册回调函数
  static void regMsgCallback() {
    regCb(cmsg::MSG_UPLOAD_CONFIG_RES, (msgCbFunc)&CConfigClient::uploadConfigRes);
    regCb(cmsg::MSG_DOWNLOAD_CONFIG_RES, (msgCbFunc)&CConfigClient::downloadConfigRes);
    regCb(cmsg::MSG_DOWNLOAD_ALL_CONFIG_RES, (msgCbFunc)&CConfigClient::downloadAllConfigRes);
    regCb(cmsg::MSG_DELETE_CONFIG_RES, (msgCbFunc)&CConfigClient::deleteConfigRes);
  }

  // 等待线程退出
  void wait();

  // 上传配置成功后在上传配置的响应中调用，用于刷新上传成功后的界面
  uploadConfigResCbFunc uploadConfigResCb_ = nullptr;

private:
  CConfigClient();

  char localIp_[16] = { 0 };  // 本地微服务的 IP
  int localPort_ = 0;          // 本地微服务的端口号

  // 动态解析 proto 文件
  google::protobuf::compiler::Importer *importer_ = nullptr;
  // 解析文件的管理对象
  google::protobuf::compiler::DiskSourceTree *sourceTree_ = nullptr;
  // 反射生成的 message 对象，根据 proto 文件动态创建
  google::protobuf::Message *message_ = nullptr;
};

#endif