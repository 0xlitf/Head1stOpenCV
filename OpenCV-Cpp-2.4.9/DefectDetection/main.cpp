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

    if (bool test = false) {
        MinimumBounding mini;
        BGR2HSVConverter converter;
        DefectDetector detector;

        cv::Mat bgMat = cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/1/test.png");

        int count{0};
        cv::Mat res;
        std::tie(count, res) = detector.analyzeAndDrawContour(bgMat);
        qDebug() << "count" << count;
        cv::imshow("res", res);
        // detector.setRemoveOuterBorderThickness(3);
        // detector.setDetectThickness(6);
        // detector.setWhiteThreshold(30);
        // detector.setScoreThreshold(15);

        // // cv::Mat objMat = cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/1-1.png");
        // // cv::Mat objMat = cv::imread(QString(PROJECT_DIR).append("/1-2.png").toStdString());
        // cv::Mat objMat = cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/2.png");
        // auto r = detector.analyzeAndDrawContour(objMat);
        // qDebug() << "count " << std::get<0>(r);
        // cv::imshow("std::get<1>(r)" , std::get<1>(r));

        // QStringList descList;
        // descList << "ok";
        // QStringList folderList;
        // folderList << QString(PROJECT_DIR).append("/template_black"); // template_brown template_black
        // detector.setTemplateFolder(descList, folderList);

        // cv::Mat dInput = objMat;
        // dInput = mini.findAndCropObject(dInput);

        // QElapsedTimer timer;
        // timer.start();
        // double defectScore = detector.fullMatchMat(dInput);

        // qDebug() << "defectScore:" << defectScore << ", fullMatchMat elapsed:" << timer.elapsed();

    }

    if (bool testContour = true) {
        // 1. 创建轮廓提取器
        ContourExtractor extractor;

        // 设置参数（面积阈值 > 1000）
        extractor.setParameters(1000.0, true, 3, 11);

        // 2. 读取白背景物料图像
        cv::Mat image = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/异形物料/OK/1_20260122153531310_5_52.png").toStdString());

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
        if (!contoursDebug.empty()) {
            qDebug() << "提取到 " << contoursDebug.size() << " 个轮廓：";

            for (size_t i = 0; i < contoursDebug.size(); ++i) {
                double area = cv::contourArea(contoursDebug[i]);
                cv::Rect bbox = cv::boundingRect(contoursDebug[i]);

                qDebug() << "轮廓 " << i + 1 << ": "
                          << "面积 = " << area
                          << ", 边界框 = " << bbox.width << bbox.height
                          << ", 点数 = " << contoursDebug[i].size();
            }

            // 显示调试图像
            cv::imshow("轮廓提取结果", debugImage);
            cv::waitKey(0);
        } else {
            qDebug() << "未找到符合条件的轮廓";
        }
    }

    return a.exec();
}
