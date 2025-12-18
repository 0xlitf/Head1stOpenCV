
#pragma execution_character_set("utf-8")

#include "mainwindow.h"
#include <QApplication>
#include <QFontDatabase>
#include "messageinstaller.h"

int main(int argc, char *argv[]) {
    MessageInstaller::instance()->install();

    QApplication a(argc, argv);

    QFontDatabase fontDB;
    QStringList fonts = fontDB.families();
    auto fontName = QString("微软雅黑");
    if (fonts.contains(fontName)) {
        QFont font(fontName, 11);
        qApp->setFont(font);
    } else {
        qDebug() << "微软雅黑字体未找到，使用系统默认字体";
    }

    MainWindow w;
    w.show();
    return a.exec();
}
