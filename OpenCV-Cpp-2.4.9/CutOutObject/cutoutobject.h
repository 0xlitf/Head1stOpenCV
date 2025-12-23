#ifndef CUTOUTOBJECT_H
#define CUTOUTOBJECT_H

#pragma execution_character_set("utf-8")

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

    // 新增接口1：返回物体所在的轴对称矩形，背景白色，物体黑色
    cv::Mat getObjectInBoundingRect(const cv::Mat& inputImage,
                                    int colorThreshold = 30,
                                    int blueThreshold = 50,
                                    int kernelSize = 3);

    // 新增接口2：返回原图尺寸的掩码，背景白色，物体黑色
    cv::Mat getObjectInOriginalSize(const cv::Mat& inputImage,
                                    int colorThreshold = 30,
                                    int blueThreshold = 50,
                                    int kernelSize = 3);

    void testExtractLargestContour(const QString& imageFilename);
};

#endif // CUTOUTOBJECT_H
