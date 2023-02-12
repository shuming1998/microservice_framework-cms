
#include "cfileservertask.h"
#include "ctools.h"
#include <iostream>
#include <string.h>

std::string CFileServerTask::curDir_ = "./";
std::mutex CFileServerTask::curDirMtx_;

void CFileServerTask::getDir(const CMsg *msg) {
  //std::string dir = "file1,1024;file2,4096;file3.zip,10240";
  if (!msg->data_) {
    return;
  }
  std::string root = msg->data_;
  if (root.empty()) {
    root = "./";
  }
  setCurDir(root);
  std::string dir = getDirData(root);
  CMsg resMsg;
  resMsg.type_ = MSG_DIR_LIST;
  resMsg.size_ = dir.size() + 1;  // '\0'
  resMsg.data_ = (char *)dir.c_str();
  writeMsg(&resMsg);
}

void CFileServerTask::uploadFile(const CMsg *msg) {
  // 1 获取文件名,文件大小
  if (!msg->data_ || !msg || msg->size_ <= 0) {
    return;
  }
  std::string str = msg->data_;
  if (str.empty()) {
    return;
  }
  int pos = str.find_last_of(',');
  if (pos <= 0) {
    return;
  }
  std::string fileName = str.substr(0, pos);
  int posSize = pos + 1;
  if (posSize >= (int)str.size()) {
    return;
  }
  std::string tmp = str.substr(posSize, str.size() - posSize);
  std::cout << fileName << ':' << tmp << '\n';
  fileSize_ = atoi(tmp.c_str());
  if (fileSize_ <= 0) {
    return;
  }

  // 2 打开本地文件开始写入
  std::string filePath = getCurDir() + fileName;  //当前路径
  ofs_.open(filePath, std::ios::out | std::ios::binary);
  if (!ofs_.is_open()) {
    std::cout << "Open file [" << filePath << "] failed!\n";
    return;
  }
  std::cout << "Open file [" << filePath << "] success!\n";

  // 3 回复 accept 消息
  CMsg resMsg;
  resMsg.type_ = MSG_UPLOAD_ACCEPT;
  resMsg.size_ = 3;
  resMsg.data_ = (char *)"OK";
  writeMsg(&resMsg);
  // 准备好接收文件后，就设置为不接收消息，准备接收文件
  setIsRecvMsg(false);
  recvSize_ = 0;
}

void CFileServerTask::downloadFile(const CMsg *msg) {
  if (!msg->data_ || !msg || msg->size_ <= 0) {
    return;
  }
  // 打开文件
  filePath_ = msg->data_;
  if (filePath_.empty()) {
    return;
  }
  // 获取文件名
  ifs_.open(filePath_.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
  if (!ifs_.is_open()) {
    std::cerr << "open file" << filePath_ << "failed!\n";
    return;
  }

  // 2 获取文件名称
  fileSize_ = ifs_.tellg();
  ifs_.seekg(0, std::ios::beg);
  std::cout << "open file" << filePath_ << "success!\n";

  // 回复消息 MSG_DOWNLOAD_ACCEPT
  char buf[32] = { 0 };
  sprintf(buf, "%d", fileSize_);
  CMsg resMsg;
  resMsg.type_ = MSG_DOWNLOAD_ACCEPT;
  resMsg.size_ = strlen(buf) + 1;
  resMsg.data_ = buf;
  writeMsg(&resMsg);
}

// 写入文件
void CFileServerTask::readCb(void *data, int size) {
  if (!data || size <= 0 || !ofs_.is_open()) {
    return;
  }
  ofs_.write((char *)data, size);
  recvSize_ += size;
  if (recvSize_ == fileSize_) {
    std::cout << "finish writing file\n";
    ofs_.close();
    CMsg resMsg;
    resMsg.type_ = MSG_UPLOAD_COMPLETE;
    resMsg.size_ = 3;
    resMsg.data_ = (char *)"OK";
    writeMsg(&resMsg);
  }
}

// 写入文件回调
void CFileServerTask::writeCb() {
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

bool CFileServerTask::readCb(const CMsg *msg) {
  switch (msg->type_) {
    case MSG_GET_DIR:
      std::cout << "MSG_GET_DIR\n";
      getDir(msg);
      break;
    case MSG_UPLOAD_INFO:
      std::cout << "MSG_UPLOAD_INFO\n";
      uploadFile(msg);
      break;
    case MSG_DOWNLOAD_INFO:
      std::cout << "MSG_DOWNLOAD_INFO\n";
      downloadFile(msg);
      break;
    case MSG_DOWNLOAD_COMPLETE:
      std::cout << "MSG_DOWNLOAD_COMPLETE\n";
      // 清理网络资源
      closeBev();
      return false;
      break;
    default:
      break;
  }
  return true;
}