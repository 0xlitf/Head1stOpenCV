#include "mainwindow.h"
#include <QApplication>
#include <QElapsedTimer>
#include "minimumbounding.h"
#include "opencv2/opencv.hpp"
#include "bgr2hsvconverter.h"

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

    return a.exec();
}
