#include "mainwindow.h"

#include <QApplication>
#include <opencv2/opencv.hpp>

#include <iostream>
#include "cutoutobject.h"

void cutoutForBlue(cv::Mat &cvImage, int nColorThreshold, int nBThreshold)
{
    for (int i = 0; i < cvImage.rows; ++i) {
        for (int j = 0; j < cvImage.cols; ++j) {
            cv::Vec3b& pixel = cvImage.at<cv::Vec3b>(i,j);
            if ((pixel[0] - pixel[1] > nColorThreshold) &&
                (pixel[0] - pixel[2] > nColorThreshold) &&
                (pixel[0] > nBThreshold))
            {
                // 蓝色区域：设为白色
                pixel[0] = 255;
                pixel[1] = 255;
                pixel[2] = 255;
            }
            else
            {
                // 非蓝色区域：设为黑色
                pixel[0] = 0;
                pixel[1] = 0;
                pixel[2] = 0;
            }
        }
    }
}

void test() {
    auto imageWithBackground = cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/CutOutObject/dataset/14-39-42-685.png");
    cv::imshow("imageWithBackground", imageWithBackground);

    // 复制图像用于处理
    cv::Mat result1 = imageWithBackground.clone();
    cv::Mat result2 = imageWithBackground.clone();
    cv::Mat result3 = imageWithBackground.clone();

    cutoutForBlue(result1, 30, 50);
    cv::imshow("宽松阈值 (30,50)", result1);

    // 中等阈值：平衡检测
    cutoutForBlue(result2, 50, 100);
    cv::imshow("中等阈值 (50,100)", result2);

    // 严格阈值：只检测明显的蓝色
    cutoutForBlue(result3, 80, 150);
    cv::imshow("严格阈值 (80,150)", result3);

    cv::waitKey(0);
    cv::destroyAllWindows();
}

int main(int argc, char *argv[]) {
    // QApplication a(argc, argv);
    // MainWindow w;
    // w.show();
    // return a.exec();

    CutOutObject cutout;
    cutout.testExtractLargestContour("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/CutOutObject/dataset/14-39-42-685.png");

    return 0;
}
