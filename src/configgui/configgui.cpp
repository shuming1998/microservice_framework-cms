#include "configgui.h"
#include "ctools.h"
#include "clogclient.h"
#include "cconfigclient.h"
#include "configedit.h"
#include "clogingui.h"
#include <QMouseEvent>
#include <QMessageBox>
#include <QTime>
#include <string>
#include <sstream>

ConfigGui::ConfigGui(QWidget *parent) : QWidget(parent) {
    ui.setupUi(this);

    // 去除原窗口边框
    setWindowFlags(Qt::FramelessWindowHint);

    // 隐藏背景，用于显示圆角
    setAttribute(Qt::WA_TranslucentBackground);

    // 设置窗口可拖动
    // 监听鼠标事件
    setMouseTracking(true);

    refresh();
}

ConfigGui::~ConfigGui() {}

static bool mousePress = false;
static QPoint mousePoint;

void ConfigGui::mouseMoveEvent(QMouseEvent *ev) {
  // 没有按下，处理原事件
  if (!mousePress) {
    QWidget::mouseMoveEvent(ev);
    return;
  }
  auto curPos = ev->globalPos();
  this->move(curPos - mousePoint);
}

void ConfigGui::mousePressEvent(QMouseEvent *ev) {
  // 鼠标左键按下，记录位置
  if (ev->button() == Qt::LeftButton) {
    mousePress = true;
    mousePoint = ev->pos();
  }
}

void ConfigGui::mouseReleaseEvent(QMouseEvent *ev) {
  mousePress = false;
}

void ConfigGui::refresh() {
  // 1 清理历史列表
  addLog("清理历史列表");
  while (ui.tableWidget->rowCount() > 0) {
    ui.tableWidget->removeRow(0);
  }
  
  // 2 如果修改了配置中心的 IP 或端口，断开重连
  std::string serverIp = ui.serverIpEdit->text().toStdString();
  int serverPort = ui.servicePortBox->value();
  std::stringstream ss;
  ss << serverIp << ':' << serverPort;
  LOG_DEBUG(ss.str().c_str());

  // 打开鉴权窗口，登陆验证
  CLoginGui gui;
  if (gui.exec() != QDialog::Accepted) {
    return;
  }


  // 断开之前的连接，重新建立连接
  CConfigClient::get()->setServerIp(serverIp.c_str());
  CConfigClient::get()->setServerPort(serverPort);
  // 断开重连时不要清理对象
  CConfigClient::get()->setAutoDelete(false);
  CConfigClient::get()->close();
  if (!CConfigClient::get()->autoConnect(3)) {
    addLog("连接配置中心失败");
    return;
  }
  addLog("连接配置中心成功");

  // 3 从配置中心获取配置列表
  auto configs = CConfigClient::get()->downloadAllConfig(1, 10000, 10);

  // 4 插入获取的列表
  int rows = configs.config_size();
  ui.tableWidget->setRowCount(rows);
  for (int i = 0; i < rows; ++i) {
    auto config = configs.config(i);
    ui.tableWidget->setItem(i, 0, new QTableWidgetItem(config.servicename().c_str()));
    ui.tableWidget->setItem(i, 1, new QTableWidgetItem(config.serviceip().c_str()));
    std::string servicePort = std::to_string(config.serviceport());
    ui.tableWidget->setItem(i, 2, new QTableWidgetItem(servicePort.c_str()));
  }
  addLog("更新配置列表完成");
}

void ConfigGui::addLog(const char *log) {
  // 加入日期显示
  QString str = QTime::currentTime().toString("HH:mm:ss");
  str += " ";
  str += QString::fromLocal8Bit(log);
  LOG_DEBUG(log);
  ui.logListWidget->insertItem(0, new QListWidgetItem(str));
}

void ConfigGui::addConfig() {
  // 打开模态窗口，等待退出
  ConfigEdit edit;
  if (edit.exec() == QDialog::Accepted) {
    addLog("新增配置成功");
  }

  refresh();
} 

void ConfigGui::delConfig() {
  if (ui.tableWidget->rowCount() == 0) {
    return;
  }
  int row = ui.tableWidget->currentRow();
  if (row < 0) {
    return;
  }

  // 获取选中的配置：name IP port
  auto itemName = ui.tableWidget->item(row, 0);
  auto itemIp = ui.tableWidget->item(row, 1);
  auto itemPort = ui.tableWidget->item(row, 2);
  std::string name = itemName->text().toStdString();
  std::string ip = itemIp->text().toStdString();
  int port = atoi(itemPort->text().toStdString().c_str());

  std::stringstream ss;
  ss << "请确认删除微服务配置：[" << name << '|' << ip << ':' << port << "] !";
  if (QMessageBox::information(0, "", QString::fromLocal8Bit(ss.str().c_str()),
                               QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
    return; 
  }
  CConfigClient::get()->deleteConfig(ip.c_str(), port);

  std::stringstream sss;
  sss << "删除微服务配置: " << name << '|' << ip << ':' << port << "]";
  addLog(sss.str().c_str());
  refresh();
}

void ConfigGui::editConfig() {
  if (ui.tableWidget->rowCount() == 0) {
    return;
  }
  // 获取需要编辑的配置
  int row = ui.tableWidget->currentRow();
  if (row < 0) {
    return;
  }

  // 获取选中的配置：IP port
  auto itemIp = ui.tableWidget->item(row, 1);
  auto itemPort = ui.tableWidget->item(row, 2);
  std::string ip = itemIp->text().toStdString();
  int port = atoi(itemPort->text().toStdString().c_str());

  //LOG_DEBUG(ip.c_str());

  // 打开配置界面
  ConfigEdit edit;
  if (!edit.loadConfig(ip.c_str(), port)) {
    addLog("读取配置失败!");
    return;
  }
  if (edit.exec() == QDialog::Accepted) {
    addLog("修改配置成功");
  }

  refresh();
}