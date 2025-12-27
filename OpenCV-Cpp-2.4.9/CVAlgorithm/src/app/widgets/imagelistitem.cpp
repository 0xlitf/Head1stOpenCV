#include "imagelistitem.h"

ImageListItem::ImageListItem(const QFileInfo& fileInfo, QWidget *parent)
    : QWidget(parent), m_thumbnailLabel(new QLabel), m_nameLabel(new QLabel), m_infoLabel(new QLabel)
{
    setupUI(fileInfo);
}

void ImageListItem::setupUI(const QFileInfo& fileInfo)
{
    // 主布局为水平布局
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5); // 设置边距
    mainLayout->setSpacing(10); // 设置控件间距

    // 1. 左侧：缩略图
    QPixmap thumbnail = generateThumbnail(fileInfo.absoluteFilePath(), QSize(80, 60));
    m_thumbnailLabel->setPixmap(thumbnail);
    m_thumbnailLabel->setFixedSize(80, 60); // 固定缩略图标签大小
    m_thumbnailLabel->setStyleSheet("border: 1px solid #cccccc;"); // 可选：添加边框
    m_thumbnailLabel->setScaledContents(false);
    mainLayout->addWidget(m_thumbnailLabel);

    // 2. 右侧：文件信息（垂直布局）
    QVBoxLayout* infoLayout = new QVBoxLayout;
    infoLayout->setSpacing(4);

    // 文件名
    m_nameLabel->setText(fileInfo.fileName());
    m_nameLabel->setStyleSheet("font-weight: bold; font-size: 12pt;");
    infoLayout->addWidget(m_nameLabel);

    // 图片尺寸和文件大小信息
    QImage image(fileInfo.absoluteFilePath());
    QString sizeInfo = QString("尺寸: %1 x %2 像素").arg(image.width()).arg(image.height());

    qint64 fileSizeBytes = fileInfo.size();
    QString sizeStr;
    if (fileSizeBytes < 1024) {
        sizeStr = QString("%1 B").arg(fileSizeBytes);
    } else if (fileSizeBytes < 1024 * 1024) {
        sizeStr = QString("%1 KB").arg(fileSizeBytes / 1024.0, 0, 'f', 1);
    } else {
        sizeStr = QString("%1 MB").arg(fileSizeBytes / (1024.0 * 1024.0), 0, 'f', 1);
    }
    QString fileSizeInfo = QString("大小: %1").arg(sizeStr);

    m_infoLabel->setText(QString("%1\n%2").arg(sizeInfo, fileSizeInfo));
    m_infoLabel->setStyleSheet("color: #666666; font-size: 10pt;");
    infoLayout->addWidget(m_infoLabel);

    infoLayout->addStretch(); // 将信息向上对齐
    mainLayout->addLayout(infoLayout);
    mainLayout->addStretch(); // 使整个项在列表中左对齐
}

QPixmap ImageListItem::generateThumbnail(const QString& filePath, const QSize& size)
{
    QPixmap pixmap(filePath);
    if(pixmap.isNull()) {
        // 如果加载失败，返回一个默认的错误图片或空图片
        return QPixmap(size);
    }
    // 缩放图片，保持宽高比，使用平滑变换
    return pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}
