#include "mainwindow.h"
#include "imagebinarizer.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // MainWindow w;
    // w.show();

    ImageBinarizer window;
    window.setWindowTitle("Qt6 图像二值化工具");
    window.resize(900, 600);
    window.show();

    return a.exec();
}
