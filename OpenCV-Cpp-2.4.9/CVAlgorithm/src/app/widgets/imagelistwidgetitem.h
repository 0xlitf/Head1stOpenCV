#ifndef IMAGELISTWIDGETITEM_H
#define IMAGELISTWIDGETITEM_H

#pragma execution_character_set("utf-8")

#include "imageinfowidget.h"
#include <QFileInfo>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>
#include <QWidget>

class ImageListWidgetItem : public QWidget {
    Q_OBJECT

public:
    explicit ImageListWidgetItem(const QFileInfo &fileInfo, QWidget *parent = nullptr);
    QPixmap generateThumbnail(const QString &filePath, const QSize &size);

private:
    void setupUI(const QFileInfo &fileInfo);

    ImageInfoWidget *m_infoWidget = nullptr;
};

#endif // IMAGELISTWIDGETITEM_H
