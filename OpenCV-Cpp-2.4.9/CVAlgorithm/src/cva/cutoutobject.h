#ifndef CUTOUTOBJECT_H
#define CUTOUTOBJECT_H

#pragma execution_character_set("utf-8")

#include "cva_global.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>
#include <QString>
#include <QDebug>

// 新增：物体检测结果结构体
struct ObjectDetectionResult {
    std::vector<cv::Point> contour;  // 物体轮廓
    double area;                      // 轮廓面积
    cv::RotatedRect minRect;          // 最小外接矩形
    cv::Rect boundingRect;           // 轴对称矩形
};

class CVASHARED_EXPORT CutOutObject {

public:
    CutOutObject();

    // 修改：检测单个最大轮廓（保持向后兼容）
    bool extractLargestContour(const cv::Mat& inputImage,
                               std::vector<cv::Point>& contour,
                               double& area,
                               cv::RotatedRect &minRect,
                               int colorThreshold = 30,
                               int blueThreshold = 50,
                               int kernelSize = 3);

    // 新增：检测多个轮廓，基于面积阈值
    std::vector<ObjectDetectionResult> extractMultipleObjects(
        const cv::Mat& inputImage,
        int colorThreshold = 30,
        int blueThreshold = 50,
        int kernelSize = 3,
        double minAreaThreshold = 1000.0,    // 最小面积阈值
        double maxAreaThreshold = 1000000.0); // 最大面积阈值

    // 新增：获取多个物体的边界框结果
    std::vector<cv::Mat> getMultipleObjectsInBoundingRect(
        const cv::Mat& inputImage,
        int colorThreshold = 30,
        int blueThreshold = 50,
        int kernelSize = 3,
        double minAreaThreshold = 1000.0,
        double maxAreaThreshold = 1000000.0);

    // 新增：获取多个物体的原图尺寸掩码
    cv::Mat getMultipleObjectsInOriginalSize(
        const cv::Mat& inputImage,
        int colorThreshold = 30,
        int blueThreshold = 50,
        int kernelSize = 3,
        double minAreaThreshold = 1000.0,
        double maxAreaThreshold = 1000000.0);

    void testExtractMultipleObjects(const QString& imageFilename,
                                    double minAreaThreshold = 1000.0,
                                    double maxAreaThreshold = 1000000.0);

    void testExtractLargestContour(const QString& imageFilename);
};

#endif // CUTOUTOBJECT_H
