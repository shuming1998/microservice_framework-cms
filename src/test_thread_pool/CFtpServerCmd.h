#pragma once
#include "CTask.h"

class CFtpServerCmd : public CTask {
public:
  CFtpServerCmd() {}
  ~CFtpServerCmd() {}

  // 初始化任务
  bool init() override;
};

