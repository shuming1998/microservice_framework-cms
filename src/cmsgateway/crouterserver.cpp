#include "crouterserver.h"
#include "crouterhandle.h"
#include "cconfigclient.h"
#include "csslctx.h"
#include <string>

CServiceHandle *CRouterServer::createServiceHandle() {
  auto router = new CRouterHandle();
  bool isSSL = CConfigClient::get()->getBool("isSSL");
  if (!isSSL) {
    return router;
  }

  // 已经设置过，暂时不考虑修改
  if (sslCtx()) {
    return router;
  }

  auto ctx = new CSSLCtx();
  std::string crtPath = CConfigClient::get()->getString("crtPath");
  std::string keyPath = CConfigClient::get()->getString("keyPath");
  std::string caPath = CConfigClient::get()->getString("caPath");
  ctx->initServer(crtPath.c_str(), keyPath.c_str(), caPath.c_str());
  setSslCtx(ctx);
  return router;
}