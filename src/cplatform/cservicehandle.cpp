#include "cservicehandle.h"
#include <string.h>

CServiceHandle::CServiceHandle() {}

CServiceHandle::~CServiceHandle() {}

void CServiceHandle::setClientIp(const char *ip) {
  if (!ip) {
    return;
  }
  strncpy(clientIp_, ip, sizeof(clientIp_));
}