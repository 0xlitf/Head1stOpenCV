#ifndef IMAGELISTITEM_H
#define IMAGELISTITEM_H

#pragma execution_character_set("utf-8")

#include "imageinfowidget.h"
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QImage>
#include <QPixmap>

class ImageListItem : public QWidget
{
    Q_OBJECT

public:
    explicit ImageListItem(const QFileInfo& fileInfo, QWidget *parent = nullptr);
    QPixmap generateThumbnail(const QString& filePath, const QSize& size);

private:
    void setupUI(const QFileInfo& fileInfo);

    ImageInfoWidget* m_infoWidget = nullptr;
};

#endif // IMAGELISTITEM_H
