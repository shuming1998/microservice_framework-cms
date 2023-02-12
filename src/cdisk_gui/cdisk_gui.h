#ifndef CDISK_GUI_H
#define CDISK_GUI_H

#include "ui_cdisk_gui.h"
#include <QtWidgets/QWidget>


class CDiskGui : public QWidget
{
    Q_OBJECT

public:
    CDiskGui(QWidget *parent = nullptr);
    ~CDiskGui();

    void updateServerInfo();

public slots:
  void refresh();
  void updateDir(std::string dir);
  void upload();
  void download();
  void downloadComplete();


private:
    Ui::CDiskGuiClass ui;
};
#endif // !CDISK_GUI_H