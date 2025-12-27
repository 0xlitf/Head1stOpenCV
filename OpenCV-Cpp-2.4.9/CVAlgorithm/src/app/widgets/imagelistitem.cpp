#include "imagelistitem.h"
#include <QHBoxLayout>
#include <QFileInfo>
#include <QImage>

ImageListItem::ImageListItem(const QFileInfo& fileInfo, QWidget *parent)
    : QWidget(parent)
    , m_infoWidget(new ImageInfoWidget) // 使用新的信息控件
{
    setupUI(fileInfo);
}

void ImageListItem::setupUI(const QFileInfo& fileInfo)
{
    // 主布局为水平布局
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(10);

    // 1. 左侧：缩略图 (保持不变)
    QPixmap thumbnail = generateThumbnail(fileInfo.absoluteFilePath(), QSize(80, 60));

    // 2. 右侧：使用 ImageInfoWidget 显示信息
    m_infoWidget->setFileInfo(fileInfo); // 设置文件信息
    mainLayout->addWidget(m_infoWidget);

    mainLayout->addStretch(); // 使整个项在列表中左对齐
}

// generateThumbnail 函数保持不变
QPixmap ImageListItem::generateThumbnail(const QString& filePath, const QSize& size)
{
    QPixmap pixmap(filePath);
    if(pixmap.isNull()) {
        return QPixmap(size);
    }
    return pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}
