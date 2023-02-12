#include "cdownloadtask.h"
#include <iostream>

void CDownloadTask::connetedCb() {
  if (filePath_.empty()) {
    closeBev();
    return;
  }
  // 发送下载文件请求
  std::string data = filePath_;
  // 1 获取文件名
  int pos = filePath_.find_last_of('/');
  if (pos < 0) {
    pos = 0;
  }
  std::string fileName = filePath_.substr(pos, filePath_.size() - pos);
  std::string path = localDir_ + fileName;

  // 2 打开文件，准备写入
  ofs_.open(path, std::ios::out | std::ios::binary);
  if (!ofs_.is_open()) {
    std::cout << "Open file [" << path << "] failed!\n";
    return;
  }
  std::cout << "Open file [" << path << "] success!\n";
  // 3 发送下载请求消息
  CMsg msg;
  msg.type_ = MSG_DOWNLOAD_INFO;
  msg.data_ = (char *)data.c_str();
  msg.size_ = data.size() + 1;
  writeMsg(&msg);
}

bool CDownloadTask::readCb(const CMsg *msg) {
  switch (msg->type_) {
  case MSG_DOWNLOAD_ACCEPT:
    // 开始接收服务端文件 期间不再接收消息
    setIsRecvMsg(false);
    if (msg->data_) {
      fileSize_ = atoi(msg->data_);
    } else {
      closeBev();
      return false;
    }
    break;
  default:
    break;
  }
  return true;
}

void CDownloadTask::readCb(void *data, int size) {
  if (!data || size <= 0 || !ofs_.is_open()) {
    return;
  }
  ofs_.write((char *)data, size);
  recvSize_ += size;
  if (recvSize_ == fileSize_) {
    // 客户端下载成功
    if (downloadCb_) {
      downloadCb_();
    }
    std::cout << "finish writing file\n";
    ofs_.close();
    CMsg resMsg;
    resMsg.type_ = MSG_DOWNLOAD_COMPLETE;
    resMsg.size_ = 3;
    resMsg.data_ = (char *)"OK";
    writeMsg(&resMsg);
  }
}