#include "imageutils.h"

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

