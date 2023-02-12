#ifndef CSSL_H
#define CSSL_H

class CSSL {
public:

  bool isEmpty() { return ssl_ == nullptr; }
  void setSSL(struct ssl_st *ssl) { this->ssl_ = ssl; }

  // 服务端接受 ssl 连接
  bool accept();

  // 客户端处理 ssl 握手
  bool connect();

  // 打印通信使用的算法
  void printCipher();

  // 打印对方证书信息
  void printCert();

private:
  struct ssl_st *ssl_ = nullptr;  // 用来通信的对象
};




#endif