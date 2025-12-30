#include "imagegriditem.h"
#include "imageutils.h"

ImageGridItem::ImageGridItem(const QString& imageName, const cv::Mat& imageData, QWidget *parent)
    : QWidget(parent), m_imageName(imageName), m_imageData(imageData), m_imageLabel(new QLabel), m_infoLabel(new QLabel)
{
    setupUI();
}

void ImageGridItem::setupUI()
{
    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    // 1. 显示图像缩略图
    QPixmap pixmap = ImageUtils::cvMatToQPixmap(m_imageData);
    // 缩放缩略图到合适大小
    QPixmap scaledPixmap = pixmap.scaled(640, 500, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_imageLabel->setPixmap(scaledPixmap);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setStyleSheet("border: 1px solid #cccccc;");
    mainLayout->addWidget(m_imageLabel);

    // 2. 显示图像信息
    QString infoText = QString("名称: %1\n尺寸: %2x%3\n通道: %4")
                           .arg(m_imageName)
                           .arg(m_imageData.cols)
                           .arg(m_imageData.rows)
                           .arg(m_imageData.channels());
    m_infoLabel->setText(infoText);
    m_infoLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_infoLabel->setStyleSheet("font-size: 9pt; color: #666666;");
    m_infoLabel->setWordWrap(true); // 允许文本换行
    mainLayout->addWidget(m_infoLabel);

    // 可选：设置固定大小或大小策略，使每个Item看起来整齐
    setFixedSize(640, 500);
}
