#include "cauthclient.h"
#include "clogclient.h"
#include "ctools.h"
#include "cmsgcom.pb.h"
#include <string>

void CAuthClient::LoginReq(std::string username, std::string password) {
  cmsg::CLoginReq req;
  req.set_username(username);
  auto md5Password = CMD5Base64((unsigned char *)password.data(), password.size());
  req.set_password(md5Password);
  // 先清理之前的数据再发送登录请求
  {
    std::lock_guard<std::mutex> guard(loginMapMtx_);
    loginMap_.erase(username);
  }
  sendMsg(cmsg::MSG_LOGIN_REQ, &req);
}

void CAuthClient::LoginRes(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("CAuthClient::LoginRes");
  cmsg::CLoginRes res;
  if (!res.ParseFromArray(msg->data_, msg->size_)) {
    LOG_DEBUG("CAuthClient::LoginRes failed: CLoginRes ParseFromArray error!");
    return;
  }

  if (res.username().empty()) {
    return;
  }

  {
    // 存储用户登录信息
    std::lock_guard<std::mutex> guard(loginMapMtx_);
    loginMap_[res.username()] = res;
  }

  LOG_DEBUG(res.DebugString());
}

void CAuthClient::addUserRes(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("CAuthClient::addUserRes");
  cmsg::CMessageRes res;
  if (!res.ParseFromArray(msg->data_, msg->size_)) {
    LOG_DEBUG("CAuthClient::addUserRes failed: CMessageRes ParseFromArray error!");
    return;
  }

  if (res.return_() == cmsg::CMessageRes::ERROR) {
    std::stringstream ss;
    ss << "addUser failed: " << res.msg();
    LOG_DEBUG(ss.str().c_str());
    return;
  }

  LOG_DEBUG("addUser succsee!");
}

bool CAuthClient::getLoginInfo(std::string username, cmsg::CLoginRes *outInfo, int timeoutMs) {
  if (!outInfo) {
    LOG_DEBUG("outInfo is nullptr!");
    return false;
  }
  int count = timeoutMs / 10;
  if (count <= 0) {
    count = 1;
  }
  for (int i = 0; i < count; ++i) {
    loginMapMtx_.lock();
    auto it = loginMap_.find(username);
    if (it == loginMap_.end()) {
      loginMapMtx_.unlock();
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      continue;
    }
    auto loginInfo = it->second;
    loginMapMtx_.unlock();
    if (loginInfo.res() != cmsg::CLoginRes::OK) {
      return false;
    }
    outInfo->CopyFrom(loginInfo);
    return true;
  }


  return false;
}

void CAuthClient::addUserReq(cmsg::CAddUserReq *user) {
  if (!user)return;
  cmsg::CAddUserReq req;
  req.CopyFrom(*user);
  std::string pass = user->password();

  // 将用户传入的密码进行 md5 base64 编码
  auto passMd = CMD5Base64((unsigned char*)pass.c_str(), pass.size());
  req.set_password(passMd);
  //SendMsg(MSG_ADD_USER_REQ, &req);
  static int i = 0;
  i++;
  //XMsgHead head;
  //head.set_msg_type(MSG_ADD_USER_REQ);
  //head.set_service_name(AUTH_NAME);
  //head.set_msg_id(i);
  //{
  //    XMUTEX(&logins_mutex_);
  //    head.set_token(login_.token());
  //    head.set_username(login_.username());
  //}
  sendMsg(cmsg::MSG_ADD_USER_REQ, &req);

  std::cout << i << "CAuthClient::Get()->addUserReq(&req);" << std::endl;
}