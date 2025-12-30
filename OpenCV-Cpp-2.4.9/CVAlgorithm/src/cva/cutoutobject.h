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

    std::tuple<cv::Mat, cv::Mat> eraseBlueBackground(cv::Mat inputImage, int colorThreshold,
                                int blueThreshold);

    // 新增：检测多个轮廓，基于面积阈值
    std::vector<ObjectDetectionResult> extractMultipleObjects(
        const cv::Mat& inputImage,
        double minAreaThreshold = 1000.0,    // 最小面积阈值
        double maxAreaThreshold = 1000000.0); // 最大面积阈值

    // 新增：获取多个物体的边界框结果
    std::vector<cv::Mat> getMultipleObjectsInBoundingRect(std::vector<ObjectDetectionResult> results);

    // 新增：获取多个物体的原图尺寸掩码
    cv::Mat getMultipleObjectsInOriginalSize(std::vector<ObjectDetectionResult> results, const cv::Mat& resultImg);

    cv::Mat drawObjectsInfo(std::vector<ObjectDetectionResult> results, const cv::Mat& inputImage);
};

#endif // CUTOUTOBJECT_H
