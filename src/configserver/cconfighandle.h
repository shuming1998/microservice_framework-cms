#ifndef CCONFIG_HANDLE_H
#define CCONFIG_HANDLE_H

#include "cservicehandle.h"


class CConfigHandle : public CServiceHandle {
public:
  // 接收上传配置的消息
  void uploadConfig(cmsg::CMsgHead *head, CMsg *msg);

  // 接收下载配置的消息
  void downloadConfig(cmsg::CMsgHead *head, CMsg *msg);

  // 接收所有下载配置的消息(分页)
  void downloadAllConfig(cmsg::CMsgHead *head, CMsg *msg);

  // 接收删除指定微服务配置的消息
  void deleteConfig(cmsg::CMsgHead *head, CMsg *msg);

  // 注册回调函数
  static void regMsgCallback() {
    regCb(cmsg::MSG_UPLOAD_CONFIG_REQ, (msgCbFunc)&CConfigHandle::uploadConfig);
    regCb(cmsg::MSG_DOWNLOAD_CONFIG_REQ, (msgCbFunc)&CConfigHandle::downloadConfig);
    regCb(cmsg::MSG_DOWNLOAD_ALL_CONFIG_REQ, (msgCbFunc)&CConfigHandle::downloadAllConfig);
    regCb(cmsg::MSG_DELETE_CONFIG_REQ, (msgCbFunc)&CConfigHandle::deleteConfig);
  }

private:

};



#endif

