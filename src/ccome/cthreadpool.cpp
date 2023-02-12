#include "cthreadpool.h"
#include "cthread.h"
#include "ctask.h"
#include <thread>
#include <iostream>

void CThreadPool::init(int threadNums) {
  this->threadNums_ = threadNums;
  this->lastThread_ = -1;
  for (int i = 0; i < threadNums_; ++i) {
    CThread *thread = new CThread();
    std::cout << "create thread NO." << i << '\n';
    thread->id_ = i + 1;
    // 启动线程
    thread->startThread();
    threads_.push_back(thread);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

void CThreadPool::dispatch(CTask *task) {
  if (!task) {
    return;
  }
  // 轮询方式分发线程
  int tid = (lastThread_ + 1) % threadNums_;
  lastThread_ = tid;
  CThread *t = threads_[tid];

  // 提交任务
  t->addTask(task);

  // 激活线程
  t->activate();
}