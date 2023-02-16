#pragma once

#include <QDialog>
#include "ui_clogingui.h"

class CLoginGui : public QDialog
{
    Q_OBJECT

public:
    CLoginGui(QWidget *parent = nullptr);
    ~CLoginGui();

public slots:
  void login();

private:
    Ui::CLoginGuiClass ui;
};
