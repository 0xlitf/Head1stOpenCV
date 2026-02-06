#ifndef AREADIFFERENCE_H
#define AREADIFFERENCE_H

#pragma execution_character_set("utf-8")

#include <QObject>
#include <QImage>
#include <QString>
#include <opencv2/opencv.hpp>
#include "cva_global.h"

class CVASHARED_EXPORT AreaDifference : public QObject
{
    Q_OBJECT
public:
    explicit AreaDifference(QObject *parent = nullptr);


};
#endif // AREADIFFERENCE_H
