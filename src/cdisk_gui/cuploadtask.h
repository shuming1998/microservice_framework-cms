#ifndef CUPLOAD_H
#define CUPLOAD_H

#include "ccometask.h"
#include <fstream>

typedef void(*uploadCbFunc)();

class CUploadTask : public CComeTask {
public:
  // 接收到消息后处理消息的回调
  bool readCb(const CMsg *msg) override;

  // 连接成功的消息回调
  void connetedCb() override;

  // 写入文件回调
  void writeCb() override;

  //  上传成功后的回调函数
  uploadCbFunc uploadCb_ = nullptr;

  void setFilePath(std::string path) { this->filePath_ = path; }

private:
  std::string filePath_ = ""; // 需要上传的文件路径
  std::ifstream ifs_;         // 读取文件
  int fileSize_ = 0;          // 文件大小
};

#endif // !CUPLOAD_H

