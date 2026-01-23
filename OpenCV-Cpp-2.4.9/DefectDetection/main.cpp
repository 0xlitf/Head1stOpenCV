#include "mainwindow.h"
#include <QApplication>
#include <QElapsedTimer>
#include "minimumbounding.h"
#include "opencv2/opencv.hpp"
#include "bgr2hsvconverter.h"
#include "contourextractor.h"

#pragma execution_character_set("utf-8")

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // MainWindow window;
    // window.setWindowTitle("HSV缺陷检测");
    // window.resize(1200, 900);
    // window.show();

    if (bool test = true) {
        MinimumBounding mini;
        BGR2HSVConverter converter;
        DefectDetector detector;

        // cv::Mat bgMat = cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/1/test.png");
        // cv::Mat bgMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/Irregular/OK/1_20260122153531310_5_52.png").toStdString());

        // int count{0};
        // cv::Mat res;
        // std::tie(count, res) = detector.analyzeAndDrawContour(bgMat);
        // qDebug() << "count" << count;

        // ContourExtractor extractor;

        // 4. 或者使用调试版本（返回轮廓+可视化结果）
        // std::vector<std::vector<cv::Point>> contoursDebug;
        // cv::Mat debugImage;
        // std::tie(contoursDebug, debugImage) = extractor.extractContourWithDebug(res);

        // cv::imshow("res", res);

        // 5. 处理提取的轮廓
        // if (contoursDebug.size() == 1) {
        //     qDebug() << "提取到 " << contoursDebug.size() << " 个轮廓：";
        //     auto contour = contoursDebug[0];
        //     double area = cv::contourArea(contour);
        //     cv::Rect bbox = cv::boundingRect(contour);

        //     qDebug() << "轮廓 " << 0 << ": "
        //              << "面积 = " << area
        //              << ", 边界框 = " << bbox.width << bbox.height
        //              << ", 点数 = " << contour.size();

        //     // 显示调试图像
        //     cv::imshow("轮廓提取结果", debugImage);
        //     cv::waitKey(0);
        // } else {
        //     qDebug() << "提取到 " << contoursDebug.size() << " 个轮廓：";
        //     qDebug() << "contoursDebug.size() != 1";
        // }

        detector.setRemoveOuterBorderThickness(3);
        detector.setDetectThickness(6);
        detector.setWhiteThreshold(30);
        detector.setScoreThreshold(15);

        // cv::Mat objMat = cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/1-1.png");
        // cv::Mat objMat = cv::imread(QString(PROJECT_DIR).append("/1-2.png").toStdString());
        // cv::Mat objMat = cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/2.png");
        // cv::Mat objMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/Irregular/OK/1_20260122153531310_5_52.png").toStdString());
        // objMat = mini.findAndCropObject(objMat);
        // auto r = detector.analyzeAndDrawContour(objMat);
        // qDebug() << "count " << std::get<0>(r);
        // // cv::imshow("std::get<1>(r)" , std::get<1>(r));

        // auto objContour = detector.findContours(objMat);
        // auto objMatClone = objMat.clone();

        // if (!objContour.empty()) {
        //     // detector.displayContourMask(objMatClone, objContour, 20);

        //     // 或者使用方法2
        //     detector.displayOuterEdge(objMatClone, objContour, 10);
        // }

        // QStringList descList;
        // descList << "ok";
        // QStringList folderList;
        // folderList << QString(PROJECT_DIR).append("/template_black"); // template_brown template_black
        // detector.setTemplateFolder(descList, folderList);

        cv::Mat tInputMat;
        cv::Mat dInputMat;

        switch (0) {
        case 0: {
            // 黑色异形
            tInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/"
                                           "Irregular/OK/1_20260122153531310_5_52.png")
                                       .toStdString());
            dInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/"
                                           "Irregular/OK/1_20260122153759941_30_51.png")
                                       .toStdString());
        } break;
        case 1: {
            // 黑色矩形
            tInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/"
                                           "DefectDetection/template_black/1ok.png")
                                       .toStdString());
            dInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/"
                                           "DefectDetection/2/NG/2026-01-15_16-09-20_925.png")
                                       .toStdString());
        } break;
        case 2: {
            // 黑色异形
            tInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/"
                                           "Irregular/OK/1_20260122153531310_5_52.png")
                                       .toStdString());
            dInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/"
                                           "Irregular/OK/1_20260122153759941_30_51.png")
                                       .toStdString());
        } break;
        case 3: {
        } break;
        }

        cv::Mat tInput = tInputMat;
        tInput = mini.findAndCropObject(tInput);

        cv::Mat dInput = dInputMat;
        dInput = mini.findAndCropObject(dInput);


        // auto tContour = detector.findContours(tInput);
        // tInput = detector.processOuterEdge(tInput, tContour, 20);

        // auto dContour = detector.findContours(dInput);
        // dInput = detector.processOuterEdge(dInput, dContour, 20);


        QElapsedTimer timer;
        timer.start();
        // double defectScore = detector.fullMatchMat(dInput);
        double defectScore = detector.matchMat(tInput, dInput);

        qDebug() << "defectScore:" << defectScore << ", fullMatchMat elapsed:" << timer.elapsed();

    }

    if (bool testContour = false) {
        // 1. 创建轮廓提取器
        ContourExtractor extractor;

        // 设置参数（面积阈值 > 1000）
        extractor.setParameters(1000.0, true, 3, 11);

        // 2. 读取白背景物料图像
        cv::Mat image = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/Irregular/OK/1_20260122153531310_5_52.png").toStdString());

        if (image.empty()) {
            qDebug() << "无法读取图像！";
            return -1;
        }

        // 3. 提取轮廓（基础版本）
        std::vector<std::vector<cv::Point>> contours =
            extractor.extractWhiteBackgroundContour(image);

        // 4. 或者使用调试版本（返回轮廓+可视化结果）
        std::vector<std::vector<cv::Point>> contoursDebug;
        cv::Mat debugImage;
        std::tie(contoursDebug, debugImage) = extractor.extractContourWithDebug(image);

        // 5. 处理提取的轮廓
        if (contoursDebug.size() == 1) {
            qDebug() << "提取到 " << contoursDebug.size() << " 个轮廓：";
            auto contour = contoursDebug[0];
            double area = cv::contourArea(contour);
            cv::Rect bbox = cv::boundingRect(contour);

            qDebug() << "轮廓 " << 0 << ": "
                     << "面积 = " << area
                     << ", 边界框 = " << bbox.width << bbox.height
                     << ", 点数 = " << contour.size();

            // 显示调试图像
            cv::imshow("轮廓提取结果", debugImage);
            cv::waitKey(0);
        } else {
            qDebug() << "提取到 " << contoursDebug.size() << " 个轮廓：";
            qDebug() << "contoursDebug.size() != 1";
        }
    }

    return a.exec();
}
