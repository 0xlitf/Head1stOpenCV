#include "imagelistwidgetitem.h"
#include <QFileInfo>
#include <QHBoxLayout>
#include <QImage>

ImageListWidgetItem::ImageListWidgetItem(const QFileInfo &fileInfo, QWidget *parent)
    : QWidget(parent)
    , m_infoWidget(new ImageInfoWidget) {
    setupUI(fileInfo);
}

void ImageListWidgetItem::setupUI(const QFileInfo &fileInfo) {
    QPixmap thumbnail = generateThumbnail(fileInfo.absoluteFilePath(), QSize(80, 60));

    m_infoWidget->setFileInfo(fileInfo);
    Layouting::RowWithMargin{m_infoWidget}.attachTo(this);
}

QPixmap ImageListWidgetItem::generateThumbnail(const QString &filePath, const QSize &size) {
    QPixmap pixmap(filePath);
    if (pixmap.isNull()) {
        return QPixmap(size);
    }
    return pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}
