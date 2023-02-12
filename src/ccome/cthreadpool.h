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
	// 单例模式获取线程池对象
	static CThreadPool* get() {
		static CThreadPool p;
		return &p;
	}

  // 初始化所有线程并启动
  void init(int threadNums);

  // 分发线程
  void dispatch(CTask *task);

  CThreadPool() {}
private:

  int threadNums_ = 0;    // 线程数量
  int lastThread_ = -1;   // 上次分发线程的位置
  std::vector<CThread *> threads_;  // 线程队列
};


#endif

