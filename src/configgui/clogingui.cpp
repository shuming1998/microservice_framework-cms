#include "clogingui.h"
#include "cauthclient.h"
#include <QMessageBox>
#include <string>

CLoginGui::CLoginGui(QWidget *parent) : QDialog(parent) {
    ui.setupUi(this);
}

CLoginGui::~CLoginGui() {}

void CLoginGui::login() {
  if (ui.usernameEdit->text().isEmpty() || ui.passwordEdit->text().isEmpty()) {
    QMessageBox::information(this, "", QString::fromLocal8Bit("用户名或密码不能为空!"));
    return;
  }

  std::string username = ui.usernameEdit->text().toStdString();
  std::string password = ui.passwordEdit->text().toStdString();
  CAuthClient::get()->LoginReq(username, password);

  cmsg::CLoginRes login;
  bool re = CAuthClient::get()->getLoginInfo(username, &login, 1000);
  if (!re) {
    QMessageBox::information(this, "", QString::fromLocal8Bit("用户名或者密码错误!"));
    return;
  }
  std::cout << "Login Success!\n";
  // 关闭 dialog 窗口
  accept();
}


