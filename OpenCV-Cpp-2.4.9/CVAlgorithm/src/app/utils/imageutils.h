#ifndef IMAGEUTILS_H
#define IMAGEUTILS_H

#include <QString>
#include <QFile>

class ImageUtils {
public:
    ImageUtils();

    static QString getImageFormatBySignature(const QString &filePath);
};

#endif // IMAGEUTILS_H
