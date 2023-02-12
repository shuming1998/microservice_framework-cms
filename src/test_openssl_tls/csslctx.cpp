#include "csslctx.h"
#include <openssl/ssl.h>
#include <iostream>

bool CSSLCtx::initServer(const char *crtFile, const char *keyFile, const char *caFile) {
  // 创建 ssl ctx 上下文
  sslCtx_ = SSL_CTX_new(TLS_server_method());
  if (!sslCtx_) {
    std::cerr << "SSL_CTX_new TLS_server_method failed!\n";
    return false;
  }

  return true;
}