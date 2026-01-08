
#pragma execution_character_set("utf-8")

#include "mainwindow.h"
#include "messageinstaller.h"
#include <QApplication>
#include <QElapsedTimer>
#include <QFontDatabase>
#include <opencv2/opencv.hpp>
#include "cutoutobject.h"
#include "humomentsmatcher.h"

int testHuMoments(int argc, char *argv[]) {
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
        QString sceneImageStr = QString(PROJECT_DIR) + "/input/chanel1/14-30-00-395.png";

        qDebug() << "sceneImageStr" << sceneImageStr;

        HuMomentsMatcher matcher;
        matcher.setWhiteThreshold(240);
        matcher.setScoreThreshold(0.1);

        auto image1 = cv::imread(QString(QString(PROJECT_DIR) + "/image1.png").toStdString(), cv::IMREAD_COLOR);
        auto image2 = cv::imread(QString(QString(PROJECT_DIR) + "/image2.png").toStdString(), cv::IMREAD_COLOR);

        if (auto showResult = false) {
            cv::imshow("image1 analyzeAndDrawContour", std::get<1>(matcher.analyzeAndDrawContour(image1)));
            cv::imshow("image2 analyzeAndDrawContour", std::get<1>(matcher.analyzeAndDrawContour(image2)));
        }

        QStringList templateDescStr;
        templateDescStr << "1"
                        << "2"
                        << "3"
                        << "4"
                        << "5"
                        << "6"
                        << "7"
                        << "8"
                        << "9"
                        << "10";
        QStringList templateFolderStr;
        templateFolderStr << QString(PROJECT_DIR) + "/dataset_folder_20251225/006"
                          << QString(PROJECT_DIR) + "/dataset_folder_20251225/88011-1"
                          << QString(PROJECT_DIR) + "/dataset_folder_20251225/88011-2"
                          << QString(PROJECT_DIR) + "/dataset_folder_20251225/88011-3"
                          << QString(PROJECT_DIR) + "/dataset_folder_20251225/88011-4"
                          << QString(PROJECT_DIR) + "/dataset_folder_20251225/88012-1"
                          << QString(PROJECT_DIR) + "/dataset_folder_20251225/A3"
                          << QString(PROJECT_DIR) + "/dataset_folder_20251225/A6"
                          << QString(PROJECT_DIR) + "/dataset_folder_20251225/M1"
                          << QString(PROJECT_DIR) + "/dataset_folder_20251225/rect";
        matcher.setTemplateFolder(templateDescStr, templateFolderStr);

        qDebug() << matcher.getTemplateSize();
        matcher.setTemplateFolder(templateDescStr, templateFolderStr);

        qDebug() << matcher.getTemplateSize();

        QFile sceneImageFile(sceneImageStr);
        if (!sceneImageFile.exists()) {
            qWarning() << "警告：匹配图片不存在";
            return -1;
        }

        // auto results = matcher.matchImage(sceneImage);
        // 等同于
        cv::Mat imageMat = cv::imread(sceneImageStr.toStdString(), cv::IMREAD_COLOR);

        QElapsedTimer timer;
        timer.start();
        auto results = matcher.quickMatchMat(imageMat);
        qDebug() << "matchMat nsecsElapsed:" << timer.nsecsElapsed();

        int i = 0;
        for (auto &result : results) {
            QString name = std::get<0>(result);                   // 名称
            std::vector<cv::Point> contour = std::get<1>(result); // 轮廓
            cv::Point2f center = std::get<2>(result);             // 中心点
            double score = std::get<3>(result);                   // 分数
            double areaDifferencePercent = std::get<4>(result);                   // 面积差值百分比

            qDebug() << "结果" << i + 1 << ":";
            qDebug() << "\t名称:" << name;
            qDebug() << "\t匹配分数:" << QString::number(score, 'f', 6);
            qDebug() << "\t中心坐标: (" << center.x << "," << center.y << ")";
            qDebug() << "\t轮廓点数:" << contour.size();
            qDebug() << "\t面积差值百分比:" << areaDifferencePercent;  // 如果模板没有匹配到，面积差值百分比为-100

            ++i;
        }

        auto sceneImg = cv::imread(sceneImageStr.toStdString(), cv::IMREAD_COLOR);

        auto resultImage = matcher.drawResultsOnImage(sceneImg, results);

        cv::imshow("resultImage detect result", resultImage);

        // 方法2：创建绿色小图并在另一位置放置
        // cv::Mat greenImage(80, 80, CV_8UC3, cv::Scalar(0, 255, 0));
        // cv::Rect roi2(200, 100, greenImage.cols, greenImage.rows);
        // greenImage.copyTo(bigImage(roi2));

        // // 方法3：居中放置蓝色小图
        // cv::Mat blueImage(120, 120, CV_8UC3, cv::Scalar(255, 0, 0));
        // placeImageCenter(bigImage, blueImage);

        if (auto testImageCombine = false) {
            cv::Mat bigImage(480, 640, CV_8UC3, cv::Scalar(255, 255, 255));

            cv::Mat smallImage(100, 100, CV_8UC3, cv::Scalar(0, 0, 0));

            cv::Rect roi1(50, 150, smallImage.cols, smallImage.rows);
            smallImage.copyTo(bigImage(roi1));

            cv::imshow("testImageCombine", bigImage);
        }


        cv::waitKey(0);
        cv::destroyAllWindows();

        return 0;
    }
}

int testCutoutObjectAndHu(int argc, char *argv[]) {
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


    HuMomentsMatcher matcher;
    matcher.setScoreThreshold(0.1);
    matcher.setWhiteThreshold(240);
    matcher.setAreaThreshold(0.2);

    QStringList templateDescStr;
    templateDescStr << "1"
                    << "2"
                    << "3"
                    << "4"
                    << "5"
                    << "6"
                    << "7"
                    << "8"
                    << "9"
                    << "10";
    QStringList templateFolderStr;
    templateFolderStr << QString(PROJECT_DIR) + "/dataset_folder_20251225/006"
                      << QString(PROJECT_DIR) + "/dataset_folder_20251225/88011-1"
                      << QString(PROJECT_DIR) + "/dataset_folder_20251225/88011-2"
                      << QString(PROJECT_DIR) + "/dataset_folder_20251225/88011-3"
                      << QString(PROJECT_DIR) + "/dataset_folder_20251225/88011-4"
                      << QString(PROJECT_DIR) + "/dataset_folder_20251225/88012-1"
                      << QString(PROJECT_DIR) + "/dataset_folder_20251225/A3"
                      << QString(PROJECT_DIR) + "/dataset_folder_20251225/A6"
                      << QString(PROJECT_DIR) + "/dataset_folder_20251225/M1"
                      << QString(PROJECT_DIR) + "/dataset_folder_20251225/rect";

    matcher.setTemplateFolder(templateDescStr, templateFolderStr);


    CutOutObject cutout;
    // cutout.testExtractLargestContour(imageName.toStdString());

    // QString imageName = QString(PROJECT_DIR) + "/dataset/bg.png"; // 纯背景
    // QString imageName = QString(PROJECT_DIR) + "/dataset/1.png"; // 1个物体
    // QString imageName = QString(PROJECT_DIR) + "/dataset/2.png"; // 2个物体

    auto allImage = FileUtils::findAllImageFiles(QString(PROJECT_DIR) + "/input/chanel1");

    qDebug() << "allImage:" << allImage.size();
    for (auto & imageName: allImage) {

        auto originImage = cv::imread(imageName.toStdString());
        if (originImage.empty()) {
            qDebug() << "无法读取图像文件:" << imageName;
            return -1;
        }
        cv::imshow("imageName", originImage);

        double minArea = 2000.0;   // 最小面积阈值
        double maxArea = 10000000.0; // 最大面积阈值


        cv::Mat eraseBlueBackground;
        cv::Mat singleChannelZeroImage;
        std::tie(eraseBlueBackground, singleChannelZeroImage) = cutout.eraseBlueBackground(originImage);

        std::vector<ObjectDetectionResult> results = cutout.extractMultipleObjects(singleChannelZeroImage, minArea, maxArea);

        // cv::Mat resultImg(singleChannelZeroImage.size(), CV_8UC3, cv::Scalar(255, 255, 255));

        cv::Mat mask;
        if (bool getMultipleObjectsInOriginalSize = true) {
            mask = cutout.getMultipleObjectsInOriginalSize(results, originImage);
            cv::imshow(QString("getMultipleObjectsInOriginalSize").toStdString(), mask);
        }

        cv::Mat whiteBackground(singleChannelZeroImage.size(), CV_8UC3, cv::Scalar(255, 255, 255));
        cv::Mat closeContour = cutout.drawObjectsContour(results, whiteBackground);
        cv::imshow(QString("closeContour").toStdString(), closeContour);

        QElapsedTimer timer;
        timer.start();

        // 输入黑白图，接口会自动进行二值化
        qDebug() << "closeContour" << closeContour.channels();
        auto binary = matcher.binaryProcess(closeContour);
        cv::imshow(QString("binary channels:%1").arg(binary.channels()).toStdString(), binary);
        auto matchResults = matcher.quickMatchMat(closeContour);
        qDebug() << "matchMat nsecsElapsed:" << timer.nsecsElapsed();

        int i = 0;
        for (auto &result : matchResults) {
            QString name = std::get<0>(result);                   // 名称

            if (name.isEmpty()) {
                qDebug() << "图中没有物体或者物体为杂料";
                break;
            }

            std::vector<cv::Point> contour = std::get<1>(result); // 轮廓
            cv::Point2f center = std::get<2>(result);             // 中心点
            double score = std::get<3>(result);                   // 分数
            double areaDifferencePercent = std::get<4>(result);                   // 面积差值百分比

            qDebug() << "结果" << i + 1 << ":";
            qDebug() << "\t名称:" << name;
            qDebug() << "\t匹配分数:" << QString::number(score, 'f', 6);
            qDebug() << "\t中心坐标: (" << center.x << "," << center.y << ")";
            qDebug() << "\t轮廓点数:" << contour.size();
            qDebug() << "\t面积差值百分比:" << areaDifferencePercent;  // 如果模板没有匹配到，面积差值百分比为-100

            ++i;
        }

        auto resultImage = matcher.drawResultsOnImage(mask, matchResults);

        cv::imshow("resultImage detect result", resultImage);

        cv::waitKey(0);
    }

    cv::destroyAllWindows();

    return a.exec();
}
int main(int argc, char *argv[]) {
    MessageInstaller::instance()->install();

    // testHuMoments(argc, argv);

    testCutoutObjectAndHu(argc, argv);
}
