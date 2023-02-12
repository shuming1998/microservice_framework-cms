#ifndef CDIR_TASK_H
#define CDIR_TASK_H

#include "ccometask.h"
#include <functional>

typedef void(*dirCbFunc)(std::string dir);

class CDirTask : public CComeTask {
public:
  // 接收到消息后处理消息的回调
  bool readCb(const CMsg *msg) override;
  // 连接成功的消息回调
  void connetedCb() override;

  dirCbFunc dirCb_ = nullptr;  // 获取目录后调用的回调函数

  void setServerRoot(std::string root) { this->serverRoot_ = root; }

private:
  std::string serverRoot_ = "";
};


#endif // !CDIR_TASK_H

