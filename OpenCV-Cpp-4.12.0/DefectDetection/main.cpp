#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow window;
    window.setWindowTitle("HSV缺陷检测");
    window.resize(1200, 900);
    window.show();

    return a.exec();
}
