#pragma once

#include <QDialog>
#include "ui_configedit.h"

namespace google {
  namespace protobuf {
    class Message;
  }
}

namespace cmsg {
  class CConfig;
}

class ConfigEdit : public QDialog
{
    Q_OBJECT

public:
    ConfigEdit(QWidget *parent = nullptr);
    ~ConfigEdit();

    // 加载配置项，从配置中心获取并解析生成界面
    bool loadConfig(const char *ip, int port);

    // 根据 message、config 生成界面
    void initGui();

signals:
  void addLog(const char *log);
  // 消息回调
  void messageCbSig(bool isOk, const char *msg);

public slots:
  void save();
  // 选择 proto 文件并加载动态编译
  void loadProto();
  // 消息回调
  void messageCb(bool isOk, const char *msg);


private:
    Ui::ConfigEdit ui;
    int configRowCount_ = 0;        // 基础配置信息的行数，用于清理根据 proto 文件生成的配置信息
    google::protobuf::Message *message_ = nullptr;  // 用于存储配置项
    cmsg::CConfig *config_ = nullptr;               // 用于存储从配置中心获取的配置项

};
