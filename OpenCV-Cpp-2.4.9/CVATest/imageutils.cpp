#include "imageutils.h"
#include <QPixmap>

ImageUtils::ImageUtils() {}


QString ImageUtils::getImageFormatBySignature(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return "无法打开文件";
    }

    QByteArray header = file.read(8); // 读取前8个字节通常足够识别常见格式
    file.close();

    if (header.startsWith("\x89PNG\x0D\x0A\x1A\x0A")) {
        return "PNG";
    } else if (header.startsWith("\xFF\xD8\xFF")) {
        return "JPEG";
    } else if (header.startsWith("BM")) {
        return "BMP";
    } else if (header.startsWith("GIF8")) {
        return "GIF";
    } else if (header.startsWith("II") || header.startsWith("MM")) {
        return "TIFF";
    } else {
        return "未知格式";
    }
}


QPixmap ImageUtils::cvMatToQPixmap(const cv::Mat &inMat) {
    if (inMat.empty())
        return QPixmap();

    // 转换颜色空间 BGR -> RGB
    cv::Mat rgbMat;
    if (inMat.channels() == 3) {
        cv::cvtColor(inMat, rgbMat, cv::COLOR_BGR2RGB);
    } else if (inMat.channels() == 1) {
        cv::cvtColor(inMat, rgbMat, cv::COLOR_GRAY2RGB);
    } else {
        rgbMat = inMat.clone();
    }

    QImage img((const uchar *)rgbMat.data, rgbMat.cols, rgbMat.rows, rgbMat.step,
               QImage::Format_RGB888);
    // bits() 只是浅拷贝，必须 deep copy 才能让 QPixmap 在 cv::Mat 释放后继续存在
    // img.bits();
    return QPixmap::fromImage(img.copy());
}
