#include "cdisk_gui.h"
#include "cdiskclient.h"
#include <QMessageBox>
#include <QFileDialog>
#include <string>


CDiskGui::CDiskGui(QWidget *parent) : QWidget(parent) {
    ui.setupUi(this);
    CDiskClient::get()->init();

    // 注册信号支持的类型
    qRegisterMetaType<std::string>("std::string");

    // 绑定获取目录的信号
    QObject::connect(CDiskClient::get(), SIGNAL(sDir(std::string)),
                     this, SLOT(updateDir(std::string)));
    // 绑定上传文件完成后刷新界面的信号
    QObject::connect(CDiskClient::get(), SIGNAL(sUploadComplete()),
      this, SLOT(refresh()));

    // 绑定下载文件完成后刷新界面的信号
    QObject::connect(CDiskClient::get(), SIGNAL(sDownloadComplete()),
      this, SLOT(downloadComplete()));

    refresh();
}

CDiskGui::~CDiskGui() {}

void CDiskGui::downloadComplete() {
  QMessageBox::information(this, "", "download complete");
}


void CDiskGui::updateDir(std::string dir) {
  //QMessageBox::information(this, "", dir.c_str());

  // 将获取的文件插入到文件列表,格式：文件名1,大小1;文件名2,大小2;...
  QString str = dir.c_str();
  str = str.trimmed();
  if (str.isEmpty()) {
    return;
  }
  QStringList fileStr = str.split(';');
  ui.filelistWidget->setRowCount(fileStr.size());
  for (int i = 0; i < fileStr.size(); ++i) {
    // 分割 ',' ，插入列表
    QStringList fileInfo = fileStr[i].split(',');
    if (fileInfo.size() != 2) {
      continue;
    }
    ui.filelistWidget->setItem(i, 0, new QTableWidgetItem(fileInfo[0]));
    ui.filelistWidget->setItem(i, 1, new QTableWidgetItem(tr("%1Byte").arg(fileInfo[1])));
  }

}

void CDiskGui::updateServerInfo() {
  std::string ip = ui.ipEdit->text().toStdString();
  std::string root = ui.pathEdit->text().toStdString();
  int port = ui.portBox->value();

  CDiskClient::get()->setServerIp(ip);
  CDiskClient::get()->setServerRoot(root);
  CDiskClient::get()->setServerPort(port);
}

void CDiskGui::refresh() {
  updateServerInfo();
  CDiskClient::get()->getDir();

  // 1 连接服务器
  // 2 设置回调
}

void CDiskGui::upload() {
  // 用户选择一个文件
  QString fileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("请选择上传文件"));
  if (fileName.isEmpty()) {
    return;
  }

  updateServerInfo();
  CDiskClient::get()->uploadFile(fileName.toStdString());

  // 插入到文件列表
  //ui.filelistWidget->insertRow(0);
  //ui.filelistWidget->setItem(0, 0, new QTableWidgetItem(fileName));
  //ui.filelistWidget->setItem(0, 1, new QTableWidgetItem(tr("%1Byte").arg(1900)));
}

void CDiskGui::download() {
  // 用户选择下载的文件和路径
  updateServerInfo();
  int row = ui.filelistWidget->currentRow();
  if (row < 0) {
    QMessageBox::information(this, "", QString::fromLocal8Bit("请选择下载文件"));
    return;
  }
  // 获取选择的文件名
  auto item = ui.filelistWidget->item(row, 0);
  std::string fileName = item->text().toStdString();
  // 获取下载路径
  QString localPath = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("请选择下载路径"));
  if (localPath.isEmpty()) {
    return;
  }
  std::string filePath = ui.pathEdit->text().toStdString();
  filePath += "/";
  filePath += fileName;
  CDiskClient::get()->downloadFile(filePath, localPath.toStdString());
}


