#include "cserviceclient.h"


CServiceClient::CServiceClient() {
  threadPool_ = CThreadPoolFactory::create();
}

CServiceClient::~CServiceClient() {
  //delete threadPool_;
  //threadPool_ = nullptr;

}

// 将任务加入到线程池中，进行连接
void CServiceClient::startConnect() {
  threadPool_->init(1);
  threadPool_->dispatch(this);
  // 客户端需要重连，不自动销毁
  setAutoDelete(false);
}