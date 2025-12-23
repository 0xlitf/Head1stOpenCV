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

    void testExtractLargestContour(const QString& imageFilename);
};

#endif // CUTOUTOBJECT_H
