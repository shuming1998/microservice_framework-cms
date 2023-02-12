#ifndef CDOWNLOAD_TASK_H
#define CDOWNLOAD_TASK_H


#include "ccometask.h"
#include <fstream>

typedef void(*downloadCbFunc)();

class CDownloadTask : public CComeTask {
public:
  // 接收到消息的回调
  bool readCb(const CMsg *msg) override;

  // 接收数据
  void readCb(void *data, int size) override;

  // 连接成功的消息回调
  void connetedCb() override;

  downloadCbFunc downloadCb_ = nullptr;

  void setFilePath(std::string path) { this->filePath_ = path; }
  void setLocalDir(std::string dir) { this->localDir_ = dir; }
  
private:
  int fileSize_ = 0;            // 文件大小
  int recvSize_ = 0;            // 已下载文件大小
  std::string filePath_;   // 需要下载的文件相对路径
  std::string localDir_;   // 本地存储路径
  std::ofstream ofs_;           // 用于写入文件
};

#endif // !CDOWNLOAD_TASK_H

