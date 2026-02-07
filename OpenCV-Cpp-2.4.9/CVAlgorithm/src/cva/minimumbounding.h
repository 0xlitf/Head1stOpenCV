#ifndef MINIMUMBOUNDING_H
#define MINIMUMBOUNDING_H

#pragma execution_character_set("utf-8")

#include <QObject>
#include <opencv2/opencv.hpp>
#include "cva_global.h"

class CVASHARED_EXPORT MinimumBounding : public QObject
{
    Q_OBJECT
public:
    explicit MinimumBounding(QObject *parent = nullptr);

    cv::Mat findAndCropObject(const cv::Mat& inputImage);

    cv::Mat fillCenterWithWhite(const cv::Mat& inputImage, int borderWidth);

    cv::Mat removeOuterBorder(const cv::Mat& inputImage, int borderWidth);

    static cv::Mat rotate180degree(cv::Mat src);
};

#endif // MINIMUMBOUNDING_H
