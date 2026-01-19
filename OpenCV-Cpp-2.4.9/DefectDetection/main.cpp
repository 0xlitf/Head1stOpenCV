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


    MinimumBounding mini;
    BGR2HSVConverter converter;
    DefectDetector detector;

    // cv::Mat objMat = cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/1-1.png");
    cv::Mat objMat = cv::imread(QString(PROJECT_DIR).append("/1-2.png").toStdString());
    // cv::Mat objMat = cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/2.png");
    auto r = detector.analyzeAndDrawContour(objMat);
    qDebug() << "count " << std::get<0>(r);

    QStringList descList;
    descList << "ok";
    QStringList folderList;
    folderList << QString(PROJECT_DIR).append("/template_black"); // template_brown template_black
    detector.setTemplateFolder(descList, folderList);

    cv::Mat dInput = objMat;
    dInput = mini.findAndCropObject(dInput);

    QElapsedTimer timer;
    timer.start();
    double defectScore = detector.fullMatchMat(dInput);

    qDebug() << "defectScore:" << defectScore << ", fullMatchMat elapsed:" << timer.elapsed();

    return a.exec();
}
