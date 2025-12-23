#ifndef CUTOUTOBJECT_H
#define CUTOUTOBJECT_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>
#include <QString>
#include <QDebug>

class CutOutObject {
public:
    CutOutObject();

    bool extractLargestContour(const cv::Mat& inputImage,
                               std::vector<cv::Point>& contour,
                               double& area,
                               cv::RotatedRect &minRect,
                               int colorThreshold = 30,
                               int blueThreshold = 50,
                               int kernelSize = 3);

    bool extractLargestContourEnhanced(const cv::Mat& inputImage,
                                       std::vector<cv::Point>& contour,
                                       double& area,
                                       int colorThreshold = 30,
                                       int blueThreshold = 50,
                                       int openIterations = 1,
                                       int closeIterations = 1,
                                       int dilateIterations = 2);

    void testExtractLargestContour(const QString& imageFilename) {
        // 读取测试图像
        cv::Mat image = cv::imread(imageFilename.toStdString());
        if (image.empty()) {
            qDebug() << "无法读取图像，创建测试图像...";
            // 创建包含蓝色区域的测试图像
            image = cv::Mat(400, 600, CV_8UC3, cv::Scalar(255, 255, 255));
            cv::rectangle(image, cv::Rect(100, 100, 200, 150), cv::Scalar(255, 0, 0), -1);
            cv::rectangle(image, cv::Rect(50, 50, 80, 80), cv::Scalar(200, 50, 50), -1);
        }

        std::vector<cv::Point> largestContour;
        double contourArea;

        // 提取最大轮廓
        // extractLargestContourEnhanced
        // extractLargestContour
        if (extractLargestContour(image, largestContour, contourArea)) {
            qDebug() << "找到最大轮廓，面积: " << contourArea << " 像素";

            // 绘制轮廓
            cv::Mat result = image.clone();
            cv::drawContours(result, std::vector<std::vector<cv::Point>>{largestContour},
                             -1, cv::Scalar(0, 255, 0), 3);

            // 显示结果
            cv::imshow("origin", image);
            cv::imshow("result", result);
            cv::waitKey(0);
        } else {
            qDebug() << "未找到有效轮廓！";
        }
    }
};

#endif // CUTOUTOBJECT_H
