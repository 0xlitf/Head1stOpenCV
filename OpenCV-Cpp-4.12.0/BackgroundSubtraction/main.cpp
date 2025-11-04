#include "mainwindow.h"
#include "backgroundsubtractor.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    BackgroundSubtractor window;
    window.show();

    return a.exec();
}
