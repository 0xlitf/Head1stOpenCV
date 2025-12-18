
#pragma execution_character_set("utf-8")

#include "mainwindow.h"
#include <QApplication>
#include <QFontDatabase>
#include "messageinstaller.h"
#include "humomentsmatcher.h"
#include <opencv2/opencv.hpp>

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

    // MainWindow w;
    // w.setWindowFlags(Qt::Window);
    // w.show();
    // w.showMaximized();

    QString sceneImage = "C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/bin/Debug/A_20250429172418975_1044_16_hrotate_multiobj.png";

    HuMomentsMatcher matcher;
    matcher.setWhiteThreshold(240);
    matcher.setScoreThreshold(0.1);

    matcher.setTemplateFolder(
        "C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/HuMoments/dataset_foler");
    auto results = matcher.matchImage(sceneImage);

    auto sceneImg = cv::imread(sceneImage.toStdString(), cv::IMREAD_COLOR);

    auto resultImage = matcher.drawResultsOnImage(sceneImg, results);

    cv::imshow("resultImage", resultImage);

    return a.exec();
}
