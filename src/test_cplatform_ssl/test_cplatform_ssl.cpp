#include "cserviceclient.h"
#include "csslctx.h"
#include "cservice.h"
#include "cmsgtype.pb.h"
#include "cmsgcom.pb.h"
#include <iostream>
#include <thread>

class MySSLClient : public CServiceClient {
public:
   void connetedCb() {
    std::cout << "MySSLClient conneted\n";
    cmsg::CLoginReq req;
    req.set_username("test_ssl_user");
    req.set_password("test_ssl_password");
    sendMsg(cmsg::MSG_LOGIN_REQ, &req);
  }
};


class MySSLHandle : public CServiceHandle {
public:
  void loginReq(cmsg::CMsgHead *head, CMsg *msg) {
    std::cout << "===========MySSLHandle::loginReq===========\n";
    cmsg::CLoginReq req;
    req.ParseFromArray(msg->data_, msg->size_);
    std::cout << req.DebugString() << '\n';
  }

  static void regMsgCallback() {
    std::cout << "====================================\n";
    regCb(cmsg::MSG_LOGIN_REQ, (msgCbFunc)&MySSLHandle::loginReq);
  }

  void connetedCb() {
    std::cout << "service ssl accept\n";
  }
};

class MySSLService : public CService {
public:
  virtual CServiceHandle *createServiceHandle() override {
    auto handle = new MySSLHandle();
    return handle;
  }
};

int main(int argc, char *argv[]) {
  CSSLCtx serverCtx;
  serverCtx.initServer("server.crt", "server.key", "server.crt");

  // 测试 SSL 服务端
  MySSLHandle::regMsgCallback();
  MySSLService service;
  service.setServerPort(20030);
  service.setSslCtx(&serverCtx);
  service.start();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));


  // 测试 SSL 客户端
  MySSLClient client;
  client.setServerIp("127.0.0.1");
  client.setServerPort(20030);
  CSSLCtx ctx;
  ctx.initClient("server.crt");
  client.setSslCtx(&ctx);
  client.startConnect();

  CThreadPool::wait();

  return 0;
}