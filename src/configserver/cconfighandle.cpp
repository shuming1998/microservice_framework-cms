#include "cconfighandle.h"
#include "configdao.h"
#include "ctools.h"

void CConfigHandle::uploadConfig(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("接收到保存配置的消息");

  // 响应消息
  cmsg::CMessageRes res;

  // 从接收到的消息中反序列化出消息内容
  cmsg::CConfig conf;
  if (!conf.ParseFromArray(msg->data_, msg->size_)) {
    LOG_DEBUG("CConfigHandle::uploadConfig ParseFromArray failed!");
    res.set_return_(cmsg::CMessageRes_CReturn_ERROR);
    res.set_msg("format error!");
    sendMsg(cmsg::MSG_UPLOAD_CONFIG_RES, &res);
    return;
  }

  if (conf.serviceip().empty()) {
    std::string ip = clientIp();
    conf.set_serviceip(ip);
  }

  if (ConfigDAO::get()->uploadConfig(&conf)) {
    res.set_return_(cmsg::CMessageRes_CReturn_OK);
    res.set_msg("OK");
    sendMsg(cmsg::MSG_UPLOAD_CONFIG_RES, &res);
    return;
  }

  res.set_return_(cmsg::CMessageRes_CReturn_ERROR);
  res.set_msg("insert to db failed!");
  sendMsg(cmsg::MSG_UPLOAD_CONFIG_RES, &res);
}

void CConfigHandle::downloadConfig(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("接收到下载配置的消息");
  cmsg::CDownloadconfigReq req;

  if (!req.ParseFromArray(msg->data_, msg->size_)) {
    LOG_DEBUG("downloadConfig ParseFromArray failed!");
    return;
  }

  // 如果 ip 为空，使用客户端 ip
  std::string ip = req.serviceip();
  if (ip.empty()) {
    ip = clientIp();
  }

  // 根据 ip 和 port 获取配置项，然后发送给客户端
  cmsg::CConfig conf = ConfigDAO::get()->downloadConfig(ip.c_str(), req.serviceport());
  sendMsg(cmsg::MSG_DOWNLOAD_CONFIG_RES, &conf);

}

void CConfigHandle::downloadAllConfig(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("接收到下载有分页的全部配置的消息");
  cmsg::CDownloadAllConfigReq req;
  if (!req.ParseFromArray(msg->data_, msg->size_)) {
    LOG_DEBUG("downloadAllConfig ParseFromArray failed!");
    return;
  }

  auto configs = ConfigDAO::get()->downloadAllConfig(req.page(), req.pagecount());
  // 发送给客户端
  sendMsg(cmsg::MSG_DOWNLOAD_ALL_CONFIG_RES, &configs);
}

void CConfigHandle::deleteConfig(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("接收到删除指定微服务配置的消息");
  // 响应消息
  cmsg::CMessageRes res;
  cmsg::CDownloadconfigReq req;

  if (!req.ParseFromArray(msg->data_, msg->size_)) {
    LOG_DEBUG("deleteConfig ParseFromArray failed!");
    return;
  }

  if (ConfigDAO::get()->deleteConfig(req.serviceip().c_str(), req.serviceport())) {
    res.set_return_(cmsg::CMessageRes_CReturn_OK);
    res.set_msg("OK");
    sendMsg(cmsg::MSG_DELETE_CONFIG_RES, &res);
    return;
  }

  res.set_return_(cmsg::CMessageRes_CReturn_ERROR);
  res.set_msg("ConfigDAO::get()->deleteConfig failed!");
  sendMsg(cmsg::MSG_DOWNLOAD_CONFIG_RES, &res);
}
