#include "cdiskclient.h"
#include "cdirtask.h"
#include "cuploadtask.h"
#include "cthreadpool.h"
#include "cdownloadtask.h"
#include <iostream>

bool CDiskClient::init() {
  CThreadPool::get()->init(7);
  return true;
}

static void dirCb(std::string dir) {
  std::cout << dir << '\n';
  CDiskClient::get()->sDir(dir);
}

static void uploadCb() {
  std::cout << "sUploadComplete" << '\n';
  CDiskClient::get()->sUploadComplete();
}

static void downloadCb() {
  std::cout << "sDownloadComplete" << '\n';
  CDiskClient::get()->sDownloadComplete();
}

void CDiskClient::uploadFile(std::string filePath) {
  auto task = new CUploadTask();
  task->setServerIp(serverIp_);
  task->setServerPort(serverPort_);
  task->setFilePath(filePath);
  task->uploadCb_ = uploadCb;
  CThreadPool::get()->dispatch(task);
}

void CDiskClient::downloadFile(std::string serverPath, std::string localDir) {
  auto task = new CDownloadTask();
  task->setServerIp(serverIp_);
  task->setServerPort(serverPort_);
  task->setFilePath(serverPath);
  task->setLocalDir(localDir);
  task->downloadCb_ = downloadCb;
  CThreadPool::get()->dispatch(task);
}

void CDiskClient::getDir() {
  std::cout << serverIp_ << ':' << serverPort_ << '\n';

  // 此时 task 未初始化，且 task 没有 event_base，不能操作
  auto task = new CDirTask();
  task->setServerIp(serverIp_);
  task->setServerPort(serverPort_);
  task->setServerRoot(serverRoot_);
  task->dirCb_ = dirCb;
  CThreadPool::get()->dispatch(task);
}