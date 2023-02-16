#include "clogclient.h"
#include "ctools.h"
#include <string>

#define LOG_LIST_MAX 100

namespace cms {
  CCOME_API void cLog(cmsg::CLogLevel level, std::string msg, const char *filename, int line) {
    cmsg::CAddLogReq req;
    req.set_log_level(level);
    req.set_log_txt(msg);
    req.set_filename(filename);
    req.set_line(line);
    CLogClient::get()->addLog(&req);
  }
}

void CLogClient::timerCb() {
  for (;;) {
    cmsg::CAddLogReq log;
    {
      std::lock_guard<std::mutex> guard(logsMtx_);
      if (logs_.empty()) {
        return;
      }
      log = logs_.front();
      logs_.pop_front();
    }
    sendMsg(cmsg::MSG_ADD_LOG_REQ, &log);
  }
}

bool CLogClient::startLog() {
  if (strlen(getServerIp()) == 0) {
    setServerIp("127.0.0.1");
  }
  if (getServerPort() <= 0) {
    setServerPort(LOG_PORT);
  }

  setAutoConnect(true);
  setTimerMs(100);
  startConnect();
  return true;
}

void CLogClient::addLog(const cmsg::CAddLogReq *req) {
  if (!req) {
    return;
  }
  if (req->log_level() < logLevel_) {
    return;
  }

  std::string levelStr = "DEBUG";
  switch (req->log_level()) {
    case cmsg::CLOG_DEBUG:
      levelStr = "DEBUG";
      break;
    case cmsg::CLOG_INFO:
      levelStr = "INFO";
      break;
    case cmsg::CLOG_ERROR:
      levelStr = "ERROR";
      break;
    case cmsg::CLOG_FATAL:
      levelStr = "FATAL";
      break;
    default:
      break;
  }

  std::string logTime = cgetTime(0, "%F %T");
  std::stringstream logText;
  logText << "==================================================\n";
  logText << logTime << " [" << levelStr << "] " << req->filename() << ":" << req->line() << '\n';
  logText << req->log_txt() << '\n';

  std::cout << logText.str().c_str() << '\n';

  // 日志写入到本地文件
  if (logOfs_.is_open()) {
    logOfs_.write(logText.str().c_str(), logText.str().size());
  }

  cmsg::CAddLogReq tmp = *req;
  if (tmp.log_time() <= 0) {
    tmp.set_log_time(time(0));
  }
  tmp.set_service_port(servicePort_);
  tmp.set_service_name(serviceName_);

  // 使用消息队列存储日志消息
  logsMtx_.lock();
  if (logs_.size() > LOG_LIST_MAX) {
    std::cout << "pop front form log list\n";
    logs_.pop_front();
  }
  logs_.push_back(*req);
  logsMtx_.unlock();
}