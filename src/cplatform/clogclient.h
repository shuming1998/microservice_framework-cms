#ifndef CLOG_CLIENT_H  
#define CLOG_CLIENT_H
#ifdef _WIN32
#ifdef CCOME_EXPORTS
#define CCOME_API __declspec(dllexport)
#else
#define CCOME_API __declspec(dllimport)
#endif
#else
#define CCOME_API
#endif // _WIN32

#include "cserviceclient.h"
#include <fstream>
#include <string>
#include <list>

namespace cms {
  CCOME_API void cLog(cmsg::CLogLevel level, std::string msg, const char *filename, int line);
}

#define LOG_DEBUG(msg) cms::cLog(cmsg::CLOG_DEBUG, msg, __FILE__, __LINE__);
#define LOG_INFO(msg) cms::cLog(cmsg::CLOG_INFO, msg, __FILE__, __LINE__);
#define LOG_ERROR(msg) cms::cLog(cmsg::CLOG_ERROR, msg, __FILE__, __LINE__);
#define LOG_FATAL(msg) cms::cLog(cmsg::CLOG_FATAL, msg, __FILE__, __LINE__);

class CCOME_API CLogClient : public CServiceClient {
public:
  ~CLogClient() {}
  static CLogClient *get() {
    static CLogClient client;
    return &client;
  }

  void addLog(const cmsg::CAddLogReq *req);

  void setLogLevel(cmsg::CLogLevel logLevel) { logLevel_ = logLevel; }

  void setLocalFile(std::string localFile) {
    logOfs_.open(localFile);
  }

  void timerCb() override;

  // 启动日志客户端
  bool startLog();

  void setServicePort(int port) { servicePort_ = port; }
  void setServiceName(std::string serviceName) { serviceName_ = serviceName; }

private:
  CLogClient() {}
  cmsg::CLogLevel logLevel_ = cmsg::CLOG_INFO;
  std::ofstream logOfs_;                        // 负责写入日志文件
  int servicePort_ = 0;
  std::string serviceName_ = "";
  std::list<cmsg::CAddLogReq> logs_;            // 日志消息队列
  std::mutex logsMtx_;                          // 日志消息队列锁
};

#endif

