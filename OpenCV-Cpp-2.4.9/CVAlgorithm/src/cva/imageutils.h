#ifndef IMAGEUTILS_H
#define IMAGEUTILS_H

#include "cva_global.h"
#include <QString>
#include <QFile>

class CVASHARED_EXPORT ImageUtils {
public:
    ImageUtils();

    static QString getImageFormatBySignature(const QString &filePath);
};

#endif // IMAGEUTILS_H
