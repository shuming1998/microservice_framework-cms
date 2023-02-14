#ifndef CTASK_H
#define CTASK_H

#ifdef _WIN32
#ifdef CCOME_EXPORTS
#define CCOME_API __declspec(dllexport)
#else
#define CCOME_API __declspec(dllimport)
#endif
#else
#define CCOME_API
#endif // _WIN32

class CCOME_API CTask {
public:
  // 初始化任务，在线程池的任务初始化时调用
  // 子类必须重写
  // @return 初始化是否成功
  virtual bool init() = 0;

  void setId(int threadId) { threadId_ = threadId; }
  int getId() const { return threadId_; }

  int getSock() const { return sock_; }
  void setSock(int sock) { sock_ = sock; }

  struct event_base *getBase() const { return base_;  }
  void setBase(struct event_base *base) { base_ = base; }


private:
  struct event_base *base_ = nullptr;
  int sock_ = 0;
  int threadId_ = 0;
};

#endif

