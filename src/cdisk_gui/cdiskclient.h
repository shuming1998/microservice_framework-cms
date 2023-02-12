#ifndef CDISK_CLIENT_H
#define CDISK_CLIENT_H

#include <QObject>
#include <string>
#include <iostream>

class CDiskClient : public QObject {

  Q_OBJECT

public:
  // 单例模式
  static CDiskClient *get() {
    static CDiskClient cc;
    return &cc;
  }

  bool init();

  // 获取服务端文件目录
  void getDir();

  // 上传文件
  void uploadFile(std::string filePath);
  // 下载文件
  //@param 服务端文件相对路径
  //@param 本地存储目录
  void downloadFile(std::string serverPath, std::string localDir);

  void setServerIp(std::string ip) { serverIp_ = ip; }
  void setServerPort(int port) { serverPort_ = port; }
  void setServerRoot(std::string root) { serverRoot_ = root; }

signals:
  void sDir(std::string dir);
  void sUploadComplete();
  void sDownloadComplete();

private:
  std::string serverIp_ = "";   // 服务器 ip 地址
  std::string serverRoot_ = "";  // 服务器 root 目录
  int serverPort_ = 0;          // 服务器端口号

  CDiskClient() {}
};



#endif

