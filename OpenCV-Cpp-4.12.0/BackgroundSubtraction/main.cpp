#include "mainwindow.h"
#include "backgroundsubtractor.h"
#include <QApplication>
#include <opencv2/opencv.hpp>
// #include <opencv2/bgsegm.hpp>  // 背景减除模块

int main(int argc, char *argv[])
{
    if (auto usingQt = false) {
        QApplication a(argc, argv);

        BackgroundSubtractor window;
        window.show();

        return a.exec();
    } else {
        // 动态背景减除
        cv::Ptr<cv::BackgroundSubtractor> bgSubtractor;

        // 选择一种背景减除算法
        // bgSubtractor = cv::createBackgroundSubtractorMOG2();  // 高斯混合模型
        bgSubtractor = cv::createBackgroundSubtractorKNN();      // K最近邻算法

        cv::Mat imageA = cv::imread("with_object.png");

        // 由于我们只有单张图片，需要"训练"背景模型
        // 在实际视频流中，这个模型会持续更新
        cv::Mat fgMask;
        bgSubtractor->apply(imageA, fgMask, 0.5);  // 学习率0.5

        // 对掩码进行后处理
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
        cv::morphologyEx(fgMask, fgMask, cv::MORPH_OPEN, kernel);

        // 提取前景
        cv::Mat result;
        imageA.copyTo(result, fgMask);

        cv::imshow("前景掩码", fgMask);
        cv::imshow("提取的物体", result);
        cv::waitKey(0);

        return 0;
    }
}
