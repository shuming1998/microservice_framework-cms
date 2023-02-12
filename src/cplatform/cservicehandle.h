#ifndef CSERVICE_HANDLE_H
#define CSERVICE_HANDLE_H
#include "cmsgevent.h"

class CCOME_API CServiceHandle : public CMsgEvent {
public:
  CServiceHandle();
  ~CServiceHandle();

  void setClientIp(const char *ip);
  void setClientPort(int port) { clientPort_ = port; }
  const char *clientIp() const { return clientIp_; }

private:
  char clientIp_[16] = { 0 };
  int clientPort_ = 0;
}; 

#endif

