
#pragma execution_character_set("utf-8")

#include "humomentsmatcher.h"
#include "mainwindow.h"
#include "messageinstaller.h"
#include <QApplication>
#include <QFontDatabase>
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

    if (auto useMainWindow = false) {
        MainWindow w;
        w.setWindowFlags(Qt::Window);
        w.show();
        w.showMaximized();

        return a.exec();
    } else {
        QString sceneImageStr = "C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/bin/Debug/"
                             "A_20250429172418975_1044_16_hrotate_multiobj.png";

        HuMomentsMatcher matcher;
        matcher.setWhiteThreshold(240);
        matcher.setScoreThreshold(0.1);

        QString templateFolderStr = "C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/HuMoments/dataset_foler";
        matcher.setTemplateFolder(templateFolderStr);

        // 3. 检查模板文件夹
        QDir templateDir(templateFolderStr);
        if (!templateDir.exists()) {
            qWarning() << "警告：模板文件夹不存在";
            return -1;
        }

        QFile sceneImageFile(sceneImageStr);
        if (!sceneImageFile.exists()) {
            qWarning() << "警告：匹配图片不存在";
            return -1;
        }

        // auto results = matcher.matchImage(sceneImage);
        // 等同于
        cv::Mat imageMat = cv::imread(sceneImageStr.toStdString(), cv::IMREAD_COLOR);
        auto results = matcher.matchMat(imageMat);

        int i = 0;
        for (auto &result : results) {
            QString name = std::get<0>(result);                   // 名称
            std::vector<cv::Point> contour = std::get<1>(result); // 轮廓
            cv::Point2f center = std::get<2>(result);             // 中心点
            double score = std::get<3>(result);                   // 分数

            qDebug() << "结果" << i + 1 << ":";
            qDebug() << "\t名称:" << name;
            qDebug() << "\t匹配分数:" << QString::number(score, 'f', 6);
            qDebug() << "\t中心坐标: (" << center.x << "," << center.y << ")";
            qDebug() << "\t轮廓点数:" << contour.size();

            ++i;
        }

        auto sceneImg = cv::imread(sceneImageStr.toStdString(), cv::IMREAD_COLOR);

        auto resultImage = matcher.drawResultsOnImage(sceneImg, results);

        cv::imshow("resultImage", resultImage);
        cv::waitKey(0);
        cv::destroyAllWindows();

        return 0;
    }
}
