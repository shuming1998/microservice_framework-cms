#pragma once
#include <vector>

class CTask;
class CThread;
class CThreadPool {
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

private:
  CThreadPool() {}

  int threadNums_ = 0;  // 线程数量
  int lastThread_ = -1;  // 上次线程位置
  std::vector<CThread *> threads_;
};

