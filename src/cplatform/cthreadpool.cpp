#include "cthreadpool.h"
#include "cthread.h"
#include "ctask.h"
#include <thread>
#include <iostream>

#ifdef _WIN32
//和protobuf头文件会有冲突 ，protobuf的头文件要在windows.h之前
#include <windows.h>
#else
#include <signal.h>
#endif

// 用于线程的循环推出哦判断
static bool isAllExit = false;
// 所有的线程对象
static std::vector<CThread *> allThreads;
static std::mutex  allThreadsMtx;

void CThreadPool::exitAllThread() {
  isAllExit = true;
  allThreadsMtx.lock();
  for (auto t : allThreads) {
    t->exit();
  }
  allThreadsMtx.unlock();
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

void CThreadPool::wait() {
  while (!isAllExit) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

// 线程池的实现不对外开放
class CCThreadPool : public CThreadPool {
public:
  void init(int threadNums) {
    this->threadNums_ = threadNums;
    this->lastThread_ = -1;
    for (int i = 0; i < threadNums_; ++i) {
      CThread *thread = new CThread();
      std::cout << "create thread NO." << i << '\n';
      thread->id_ = i + 1;
      // 启动线程
      thread->startThread();
      threads_.push_back(thread);
      allThreadsMtx.lock();
      allThreads.push_back(thread);
      allThreadsMtx.unlock();
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }

  void dispatch(CTask *task) {
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

private:
  int threadNums_ = 0;    // 线程数量
  int lastThread_ = -1;   // 上次分发线程的位置
  std::vector<CThread *> threads_;  // 线程队列

};

CThreadPool *CThreadPoolFactory::create() {
  //socket库初始化
  static std::mutex mtx;
  static bool isInit = false;
  mtx.lock();
  if (!isInit) {
#ifdef _WIN32 
    //初始化socket库
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#else
    //使用断开连接socket，会发出此信号，造成程序退出
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
      return 1;
    }
#endif
    isInit = true;
  }
  mtx.unlock();
  return new CCThreadPool();
}



