#include "cauthhandle.h"
#include "ctools.h"
#include "cauthdao.h"

void CAuthHandle::loginReq(cmsg::CMsgHead *head, CMsg *msg) {
  int timeoutSec = 1800;
  cmsg::CLoginReq req;
  if (!req.ParseFromArray(msg->data_, msg->size_)) {
    LOG_DEBUG("CAuthHandle::loginReq failed!");
    return;
  }

  cmsg::CLoginRes res;
  // 验证用户名密码
  //LOG_DEBUG(req.DebugString());
  bool re = CAuthDAO::get()->login(&req, &res, timeoutSec);
  if (!re) {
    LOG_DEBUG("CAuthDAO::get()->login failed!");
  }
  head->set_msg_type(cmsg::MSG_LOGIN_RES);
  sendMsg(head, &res);
}

void CAuthHandle::addUserReq(cmsg::CMsgHead *head, CMsg *msg) {
  cmsg::CAddUserReq req;
  cmsg::CMessageRes res;
  // 解析失败
  if (!req.ParseFromArray(msg->data_, msg->size_)) {
    res.set_return_(cmsg::CMessageRes::ERROR);
    res.set_msg("CAuthHandle::addUserReq failed: ParseFromArray failed!");
    sendMsg(cmsg::MSG_ADD_USER_RES, &res);
    return;
  }

  bool re = CAuthDAO::get()->addUser(&req);
  if (!re) {
    res.set_return_(cmsg::CMessageRes::ERROR);
    res.set_msg("CAuthDAO::get()->addUser failed: addUser failed!");
    sendMsg(cmsg::MSG_ADD_USER_RES, &res);
    return;
  }

  res.set_return_(cmsg::CMessageRes::OK);
  res.set_msg("CAuthHandle::addUserReq OK");
  sendMsg(cmsg::MSG_ADD_USER_RES, &res);
}