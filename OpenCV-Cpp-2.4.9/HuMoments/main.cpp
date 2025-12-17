
#pragma execution_character_set("utf-8")

#include "mainwindow.h"
#include <QApplication>
#include "messageinstaller.h"

int main(int argc, char *argv[]) {
    MessageInstaller::instance()->install();

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
