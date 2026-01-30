#ifndef IMAGEUTILS_H
#define IMAGEUTILS_H

#include "cva_global.h"
#include <QString>
#include <QFile>
#include <opencv2/opencv.hpp>

class ImageUtils {
public:
    ImageUtils();

    static QString getImageFormatBySignature(const QString &filePath);

    // 辅助函数：将 cv::Mat 转换为 QPixmap 用于显示
    static QPixmap cvMatToQPixmap(const cv::Mat &inMat);

};

#endif // IMAGEUTILS_H
