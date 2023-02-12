#pragma once

#include <QtWidgets/QWidget>
#include "ui_configgui.h"

class ConfigGui : public QWidget
{
    Q_OBJECT

public:
    ConfigGui(QWidget *parent = nullptr);
    ~ConfigGui();
    void mouseMoveEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);

public slots:
  // 刷新显示配置列表
  void refresh();
  // 新增配置
  void addConfig();
  // 删除选中的配置
  void delConfig();
  // 编辑选中的配置
  void editConfig();

  // 显示在日志列表中
  void addLog(const char *log);
private:
    Ui::ConfigGuiClass ui;

};
