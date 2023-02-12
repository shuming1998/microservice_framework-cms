#include "cuploadtask.h"
#include <iostream>

// 接收到消息后处理消息的回调
bool CUploadTask::readCb(const CMsg *msg) {
  switch (msg->type_) {
    case MSG_UPLOAD_ACCEPT :
      // 开始写文件
      beginWrite();
      break;
    case MSG_UPLOAD_COMPLETE:
      // 刷新界面
      if (uploadCb_) {
        uploadCb_();
      }
      break;
    default:
      break;
  }
  return true;
}

// 连接成功的消息回调
void CUploadTask::connetedCb() {
  if (filePath_.empty()) {
    std::cerr << "CUploadTask::connetedCb() filePath_ is empty!\n";
    return;
  }
  // 1 打开本地文件
  // 跳转到结尾获取文件大小
  ifs_.open(filePath_.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
  if (!ifs_.is_open()) {
    std::cerr << "open file" << filePath_ << "failed!\n";
  }

  // 2 获取文件名称，大小  filename,size
  fileSize_ = ifs_.tellg();
  ifs_.seekg(0, std::ios::beg);
  std::cerr << "open file" << filePath_ << "success!\n";
  std::string fileName = "";
  int pos = filePath_.find_last_of('/');
  if (pos < 0) {
    pos = 0;
  }
  fileName = filePath_.substr(pos, filePath_.size() - pos);
  char data[1024];
  sprintf(data, "%s,%d", fileName.c_str(), fileSize_);

  // 3 发送上传文件请求
  CMsg msg;
  msg.type_ = MSG_UPLOAD_INFO;
  msg.data_ = data;
  msg.size_ = strlen(data) + 1;
  writeMsg(&msg);
}

// 写入文件回调
void CUploadTask::writeCb() {
  if (!ifs_.is_open()) {
    return;
  }
  ifs_.read(readBuf_, sizeof(readBuf_));
  int len = ifs_.gcount();
  if (len <= 0) {
    ifs_.close();
    return;
  }
  writeMsg(readBuf_, len);
  if (ifs_.eof()) {
    ifs_.close();
  }
}