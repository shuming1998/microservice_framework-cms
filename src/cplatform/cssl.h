#ifndef CSSL_H
#define CSSL_H

#ifdef _WIN32
#ifdef CCOME_EXPORTS
#define CCOME_API __declspec(dllexport)
#else
#define CCOME_API __declspec(dllimport)
#endif
#else
#define CCOME_API
#endif // _WIN32

class CCOME_API CSSL {
public:
  CSSL() {}
  ~CSSL() {}

  bool isEmpty() { return ssl_ == nullptr; }
  void setSSL(struct ssl_st *ssl) { this->ssl_ = ssl; }

  // 服务端接受 ssl 连接
  bool accept();

  // 客户端处理 ssl 握手
  bool connect();

  // 发送数据
  int write(const void *data, int dataSize);

  // 接收数据
  int read(void *buf, int bufSize);

  // 释放 ssl
  void close();

  // 打印通信使用的算法
  void printCipher();

  // 打印对方证书信息
  void printCert();

  struct ssl_st *ssl() { return ssl_;  }

private:
  struct ssl_st *ssl_ = nullptr;  // 用来通信的对象
};




#endif