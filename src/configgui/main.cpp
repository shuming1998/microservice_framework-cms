#include "configgui.h"
#include "cconfigclient.h"
#include "configedit.h"
#include "cauthclient.h"
#include <QtWidgets/QApplication>


int main(int argc, char *argv[]) {
  // 初始化配置客户端，创建线程池
  CConfigClient::get()->startGetConfig("127.0.0.1", CONFIG_PORT, 0, 0, 0);
  CAuthClient::get()->setServerIp("127.0.0.1");
  CAuthClient::get()->setServerPort(AUTH_PORT);
  CAuthClient::get()->regMsgCallback();
  CAuthClient::get()->startConnect();

  QApplication a(argc, argv);
  //ConfigEdit edit;
  //edit.exec();
  //return 0;
  ConfigGui w;
  w.show();
  return a.exec();
}
