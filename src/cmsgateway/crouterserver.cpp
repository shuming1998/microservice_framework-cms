#include "crouterserver.h"
#include "crouterhandle.h"

CServiceHandle *CRouterServer::createServiceHandle() {
  return new CRouterHandle();
}