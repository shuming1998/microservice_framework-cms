#ifndef CFILE_SERVER_TASK_H
#define CFILE_SERVER_TASK_H

#include "ccometask.h"
#include <fstream>
#include <mutex>

class CFileServerTask : public CComeTask {
public:
  // 接收到消息的回调
  bool readCb(const CMsg *msg) override;

  // 不接收消息时调用，用于接收客户端发送的文件
  void readCb(void *data, int size) override;

  // 写入文件回调
  void writeCb() override;

  static void setCurDir(std::string dir) {
    std::lock_guard<std::mutex> guard(curDirMtx_);
    curDir_ = dir;
  }

  static std::string getCurDir() {
    std::lock_guard<std::mutex> guard(curDirMtx_);
    return std::string(curDir_);
  }

private:
  // 处理获取目录的消息，返回目录列表
  void getDir(const CMsg *msg);

  // 处理客户端上传请求
  void uploadFile(const CMsg *msg);

  // 处理客户端下载请求
  void downloadFile(const CMsg *msg);


  int fileSize_ = 0;            // 文件大小
  int recvSize_ = 0;            // 客户端已上传文件大小
  std::string filePath_;        // 文件路径
  std::ofstream ofs_;           // 用于写入文件
  std::ifstream ifs_;           // 用于读取文件

  static std::mutex curDirMtx_;
  static std::string curDir_;   // 当前路径
  
};

#endif // !CFILE_SERVER_TASK_H

