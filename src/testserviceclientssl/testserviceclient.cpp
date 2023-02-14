#include "ctestclient.h"
#include "csslctx.h"
#include <iostream>
#include <sstream>
#include <thread>


int main(int argc, char *argv[]) {
  CTestClient::regMsgCb();
  CTestClient *client = new CTestClient();

  // 设置客户端以 ssl 通信
  CSSLCtx ctx;
  ctx.initClient();
  client->setSslCtx(&ctx);

  client->setServerIp("127.0.0.1");
  client->setServerPort(API_GATEWAY_PORT);
  client->startConnect();

  for (int i = 0; i < 10000; ++i) {
    std::stringstream ss;
    ss << "/root/" << i;
    client->getDir(ss.str());
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  CThreadPool::wait();
  return 0;
}