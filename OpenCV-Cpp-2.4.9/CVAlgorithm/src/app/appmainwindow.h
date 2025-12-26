#pragma once

#include "centralwidget.h"
#include <QMainWindow>

#pragma execution_character_set("utf-8")

class AppMainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit AppMainWindow(QWidget* parent = nullptr);

signals:

private:
    void createComponents();
   CentralWidget* m_central = new CentralWidget;
    // MainPage* m_central = new MainPage;
};
