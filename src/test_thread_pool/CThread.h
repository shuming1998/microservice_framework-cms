#pragma once
#include <event2/event.h>
#include <list>
#include <mutex>

class CTask;
class CThread {
public:
  CThread();
  ~CThread();
  // 启动线程
  void startThread();

  // 线程入口函数
  void mainFunc();

  // 安装线程，初始化 event_base 和管道(linux)/socket(windows)监听事件，用于激活线程
  bool setup();

  // 收到主线程发出的激活消息(线程池的分发)
  void notify(evutil_socket_t fd, short which);

  // 激活线程
  void activate();

  // 添加需要处理的任务，一个线程对应一个 event_base，但可以同时处理多个任务
  void addTask(CTask *task);

  void setId(int id) { id_ = id; }

  int getId() const { return id_; }


private:
  int id_ = 0;                // 线程编号
  int notifySendFd_ = 0;
  struct event_base *base_ = nullptr;

  std::list<CTask *> tasks_;  // 任务列表
  std::mutex taskMutex_;
};

