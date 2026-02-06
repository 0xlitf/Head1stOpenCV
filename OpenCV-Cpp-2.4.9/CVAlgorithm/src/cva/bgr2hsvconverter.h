#ifndef BGR2HSVCONVERTER_H
#define BGR2HSVCONVERTER_H

#pragma execution_character_set("utf-8")

#include <opencv2/opencv.hpp>
#include <QObject>
#include <QDebug>
#include "cva_global.h"

class CVASHARED_EXPORT BGR2HSVConverter : public QObject {
    Q_OBJECT
public:
    explicit BGR2HSVConverter(QObject *parent = nullptr);

public:
    // 基本BGR到HSV转换
    static cv::Mat convertBGR2HSV(const cv::Mat& bgrImage);

    // 反向转换：HSV到BGR
    static cv::Mat convertHSV2BGR(const cv::Mat& hsvImage);
};

#endif // BGR2HSVCONVERTER_H
