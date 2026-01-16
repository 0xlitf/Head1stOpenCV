#include "mainwindow.h"
#include <QApplication>
#include "minimumbounding.h"
#include "opencv2/opencv.hpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // MainWindow window;
    // window.setWindowTitle("HSV缺陷检测");
    // window.resize(1200, 900);
    // window.show();

    // 读取图像
    cv::Mat inputImage = cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/2/ok/2026-01-15_16-07-45_741.png");
    if (inputImage.empty()) {
        qDebug() << "无法读取图像文件！";
        return -1;
    }

    // 创建处理器实例
    MinimumBounding processor;

    // 处理图像
    cv::Mat resultImage = processor.findAndCropObject(inputImage);

    // 检查结果并保存或显示
    if (!resultImage.empty()) {
        cv::imshow("Original", inputImage);
        cv::imshow("Cropped and Corrected", resultImage);
        cv::waitKey(0);

        // 保存结果
        cv::imwrite("result.jpg", resultImage);
    } else {
        qDebug() << "处理失败或未找到物体。";
    }

    return a.exec();
}
