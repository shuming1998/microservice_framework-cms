#include "cthread.h"
#include "ctask.h"
#include <thread>
#include <iostream>
#include <event2/event.h>

#ifdef _WIN32
#else
#include <unistd.h>
#endif // _WIN32

//激活线程任务事件的回调函数
static void notifyCb(int fd, short which, void *arg) {
  CThread *t = (CThread *)arg;
  t->notify(fd, which);
}

void CThread::startThread() {
  setup();
  // 启动线程
  std::thread thread(&CThread::mainFunc, this);

  // 设置为分离线程
  thread.detach();
}

// 线程入口函数
void CThread::mainFunc() {
  std::cout << id_ << " CThread::mainFunc begin\n";
  if (!base_) {
    std::cerr << "CThread::mainFunc() failed because base_ is none!\n";
    std::cerr << "fix WSAStartup!\n";
    return;
  }
  // 设置为不阻塞分发消息
  while (!isExit_) {
    // 一次处理多条消息
    event_base_loop(base_, EVLOOP_NONBLOCK);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  
  event_base_free(base_);
  //event_base_dispatch(base_);
  std::cout << id_ << " CThread::mainFunc end\n";
}

bool CThread::setup() {
  // windows 用 socketpair， linux 用 pipe
#ifdef _WIN32
  // 创建一个 socketpair，可以互相通信，fds[0] 读， fds[1] 写
  evutil_socket_t fds[2];
  if (evutil_socketpair(AF_INET, SOCK_STREAM, 0, fds) < 0) {
    std::cout << "evutil_socketpair failed!\n";
    return false;
  }
  // 设置为非阻塞
  evutil_make_socket_nonblocking(fds[0]);
  evutil_make_socket_nonblocking(fds[1]);
#else
  // linux 用管道，不能 send/recv， 只能 write/read
  int fds[2];
  if (pipe(fds)) {
    std::cerr << "pipe failed!\n";
    return false;
  }
#endif // _WIN32

  // 读取 fd 会绑定到 event 事件中，写入 fd 则要保存
  notifySendFd_ = fds[1];

  // 创建 libevent 上下文(相关操作在一个线程中做，所以无锁) 
  event_config *ev_conf = event_config_new();
  event_config_set_flag(ev_conf, EVENT_BASE_FLAG_NOLOCK);
  this->base_ = event_base_new_with_config(ev_conf);
  if (!base_) {
    std::cerr << "event_base_new_with_config failed!\n";
    return false;
  }

  event_config_free(ev_conf);

  // 添加管道监听事件，用于激活线程执行任务
  event *ev = event_new(base_, fds[0], EV_READ | EV_PERSIST, notifyCb, this);
  event_add(ev, 0);

  return true;
}

void CThread::notify(int fd, short which) {
  // 水平触发， 只要没有接受完成，会再次进来
  char buf[2] = { 0 };

#ifdef _WIN32
  // 读一个字符
  int res = recv(fd, buf, 1, 0);
#else
  // linux 中是管道，只能用 read
  int res = read(fd, buf, 1);
#endif // _WIN32

  if (res <= 0) {
    return;
  }
  std::cout << id_ << " thread " << buf << '\n';

  // 获取并初始化任务
  CTask *task = nullptr;
  taskMutex_.lock();
  if (tasks_.empty()) {
    taskMutex_.unlock();
    return;
  }
  task = tasks_.front();
  tasks_.pop_front();
  taskMutex_.unlock();
  task->init();
}

void CThread::addTask(CTask *task) {
  if (!task) {
    return;
  }

  task->setBase(this->base_);

  //std::lock_guard<std::mutex> guard(taskMutex_);
  taskMutex_.lock();
  tasks_.push_back(task);
  taskMutex_.unlock();
}

// 接收到一个客户端连接之后，就需要调用此函数激活线程
void CThread::activate() {
#ifdef _WIN32
  int res = send(this->notifySendFd_, "c", 1, 0);
#else
  // linux 中是管道，只能用 read
  int res = write(this->notifySendFd_, "c", 1);
#endif // _WIN32
  if (res <= 0) {
    std::cerr << "CThread::activate  failed!\n";
  }
}