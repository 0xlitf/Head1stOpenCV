#ifndef MINIMUMBOUNDING_H
#define MINIMUMBOUNDING_H

#pragma execution_character_set("utf-8")

#include <QObject>
#include <opencv2/opencv.hpp>

class MinimumBounding : public QObject
{
    Q_OBJECT
public:
    explicit MinimumBounding(QObject *parent = nullptr);

    // 修改后的接口：输入和输出均为cv::Mat，底色为纯黑色，短边纵向、长边横向
    cv::Mat findAndCropObject(const cv::Mat& inputImage);

    cv::Mat fillCenterWithWhite(const cv::Mat& inputImage, int borderWidth);

    cv::Mat removeOuterBorder(const cv::Mat& inputImage, int borderWidth);
};

#endif // MINIMUMBOUNDING_H
