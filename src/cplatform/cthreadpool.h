#ifndef CTHREAD_POOL_H
#define CTHREAD_POOL_H

#ifdef _WIN32
#ifdef CCOME_EXPORTS
#define CCOME_API __declspec(dllexport)
#else
#define CCOME_API __declspec(dllimport)
#endif
#else
#define CCOME_API
#endif // _WIN32


#include <vector>

class CTask;
class CThread;
class CCOME_API CThreadPool {
public:
  // 初始化所有线程并启动
  virtual void init(int threadNums) = 0;

  // 分发线程
  virtual void dispatch(CTask *task) = 0;

  // 退出所有线程
  static void exitAllThread();

  // 阻塞等待 exitAllThread
  static void wait();
};

class CCOME_API CThreadPoolFactory {
public:
  // 创建线程池对象
  static CThreadPool *create();
};

#endif

