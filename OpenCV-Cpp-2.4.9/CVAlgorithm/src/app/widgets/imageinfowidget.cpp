#include "imageinfowidget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QImage>

ImageInfoWidget::ImageInfoWidget(QWidget *parent)
    : QWidget(parent)
    , m_nameLabel(new QLabel)
    , m_infoLabel(new QLabel)
    , m_layout(new QVBoxLayout)
    , m_fileSizeBytes(0)
{
    setupUI();
}

void ImageInfoWidget::setupUI()
{
    // 设置布局和边距
    m_layout->setContentsMargins(0, 0, 0, 0); // 使用外部控制边距
    m_layout->setSpacing(4);

    // 文件名样式
    m_nameLabel->setStyleSheet("font-weight: bold; font-size: 12pt;");
    m_nameLabel->setWordWrap(true); // 文件名过长时自动换行
    m_layout->addWidget(m_nameLabel);

    // 图片信息样式
    m_infoLabel->setStyleSheet("color: #666666; font-size: 10pt;");
    m_infoLabel->setWordWrap(true);
    m_layout->addWidget(m_infoLabel);

    m_layout->addStretch(); // 将信息向上对齐
    setLayout(m_layout);
}

void ImageInfoWidget::setFileInfo(const QFileInfo& fileInfo)
{
    m_fileName = fileInfo.fileName();
    m_fileSizeBytes = fileInfo.size();

    // 获取图片尺寸
    QImage image(fileInfo.absoluteFilePath());
    if (!image.isNull()) {
        m_imageSize = image.size();
    } else {
        m_imageSize = QSize(0, 0);
    }

    updateDisplay();
}

void ImageInfoWidget::updateDisplay()
{
    m_nameLabel->setText(m_fileName);

    QString sizeInfo;
    if (m_imageSize.isValid() && m_imageSize.width() > 0 && m_imageSize.height() > 0) {
        sizeInfo = QString("尺寸: %1 x %2 像素\n").arg(m_imageSize.width()).arg(m_imageSize.height());
    } else {
        sizeInfo = "尺寸: 未知\n";
    }

    QString sizeStr;
    if (m_fileSizeBytes < 1024) {
        sizeStr = QString("%1 B").arg(m_fileSizeBytes);
    } else if (m_fileSizeBytes < 1024 * 1024) {
        sizeStr = QString("%1 KB").arg(m_fileSizeBytes / 1024.0, 0, 'f', 1);
    } else {
        sizeStr = QString("%1 MB").arg(m_fileSizeBytes / (1024.0 * 1024.0), 0, 'f', 1);
    }
    QString fileSizeInfo = QString("大小: %1").arg(sizeStr);

    m_infoLabel->setText(sizeInfo + fileSizeInfo);
}

// 以下为可选接口，用于更精细的控制
void ImageInfoWidget::setFileName(const QString& fileName)
{
    m_fileName = fileName;
    updateDisplay();
}

void ImageInfoWidget::setImageSize(const QSize& size)
{
    m_imageSize = size;
    updateDisplay();
}

void ImageInfoWidget::setFileSize(qint64 bytes)
{
    m_fileSizeBytes = bytes;
    updateDisplay();
}
