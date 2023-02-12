#ifndef CSSL_CTX_H
#define CSSL_CTX_H
#include "cssl.h"


class CSSLCtx {
public:
  // 初始化服务端
  // @param crtFile 服务端证书文件
  // @param keyFile 服务端私钥文件
  // @param caFile  验证客户端证书(可选)
  virtual bool initServer(const char *crtFile, const char *keyFile, const char *caFile = nullptr);


  // 初始化客户端
  // @param caFile 验证服务端证书
  virtual bool initClient(const char *caFile = nullptr);


  // 创建 SSL 通信对象，socket 和 ssl_st 资源由调用者释放
  // 是否创建失败通过 CSSL::isEmpty() 判断
  CSSL newCSSL(int socket);

private:
  // 验证对方证书
  void verify(const char *caCrt);
  struct ssl_ctx_st *sslCtx_ = nullptr;
};

#endif // !CSSL_CTX_H

