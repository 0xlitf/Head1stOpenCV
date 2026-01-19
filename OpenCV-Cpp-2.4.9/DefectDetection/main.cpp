#include "mainwindow.h"
#include <QApplication>
#include <QElapsedTimer>
#include "minimumbounding.h"
#include "opencv2/opencv.hpp"
#include "bgr2hsvconverter.h"

void test() {

    // 读取图像
    // cv::Mat inputImage = cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/2/ok/2026-01-15_16-07-45_741.png");
    cv::Mat inputImage = cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/2/ng/2026-01-15_16-09-20_925.png");
    if (inputImage.empty()) {
        qDebug() << "无法读取图像文件！";
        return;
    }

    // 创建处理器实例
    MinimumBounding processor;
    BGR2HSVConverter converter;

    // 处理图像
    cv::Mat resultImage = processor.findAndCropObject(inputImage);

    // 检查结果并保存或显示
    if (!resultImage.empty()) {
        cv::imshow("Original", inputImage);
        cv::imshow("Cropped and Corrected", resultImage);


        // cv::imshow("Original", converter.convertBGR2HSV(inputImage));
        // cv::imshow("Cropped and Corrected", converter.convertBGR2HSV(resultImage));

        cv::waitKey(0);

        // 保存结果
        cv::imwrite("result.jpg", resultImage);
    } else {
        qDebug() << "处理失败或未找到物体。";
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow window;
    window.setWindowTitle("HSV缺陷检测");
    window.resize(1200, 900);
    window.show();


    // MinimumBounding mini;
    // BGR2HSVConverter converter;
    // DefectDetector detector;

    // // cv::Mat objMat = cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/1-1.png");
    // cv::Mat objMat = cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/1-2.png");
    // // cv::Mat objMat = cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/2.png");
    // auto r = detector.analyzeAndDrawContour(objMat);
    // qDebug() << "count " << std::get<0>(r);

    // QStringList descList;
    // descList << "ok";
    // QStringList folderList;
    // folderList << "C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/template_black"; // template_brown template_black
    // detector.setTemplateFolder(descList, folderList);

    // cv::Mat dInput = objMat;
    // dInput = mini.findAndCropObject(dInput);

    // QElapsedTimer timer;
    // timer.start();
    // double defectScore = detector.fullMatchMat(dInput);

    // qDebug() << "defectScore:" << defectScore << ", fullMatchMat elapsed:" << timer.elapsed();

    return a.exec();
}
