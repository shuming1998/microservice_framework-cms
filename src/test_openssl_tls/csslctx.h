#ifndef CSSL_CTX_H
#define CSSL_CTX_H

class CSSLCtx {
public:
  // 初始化服务端
  // @param crtFile 服务端证书文件
  // @param keyFile 服务端私钥文件
  // @param caFile  验证客户端证书(可选)
  bool initServer(const char *crtFile, const char *keyFile, const char *caFile = nullptr);

private:
  struct ssl_ctx_st *sslCtx_ = nullptr;
};

#endif // !CSSL_CTX_H

