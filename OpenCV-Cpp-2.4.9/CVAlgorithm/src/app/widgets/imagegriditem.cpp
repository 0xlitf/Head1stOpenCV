#include "imagegriditem.h"
#include "imageutils.h"
#include "controls/layoutbuilder.h"
#include <QDebug>

ImageGridItem::ImageGridItem(const QString& imageName, const cv::Mat& imageData, QWidget *parent)
    : QWidget(parent), m_imageName(imageName), m_imageData(imageData), m_imageLabel(new QLabel), m_infoLabel(new QLabel)
{
    setupUI();
}

void ImageGridItem::setupUI()
{
    // 1. 显示图像缩略图
    // 获取当前label的尺寸（去边框影响）
    QSize labelSize = m_imageLabel->size();
    QPixmap pixmap = ImageUtils::cvMatToQPixmap(m_imageData);
    QPixmap scaledPixmap = pixmap.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    m_imageLabel->setPixmap(scaledPixmap);

    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setStyleSheet("border: 1px solid #cccccc;");

    // 2. 显示图像信息
    QString infoText = QString("名称: %1\n尺寸: %2x%3\n通道: %4")
                           .arg(m_imageName)
                           .arg(m_imageData.cols)
                           .arg(m_imageData.rows)
                           .arg(m_imageData.channels());
    m_infoLabel->setText(infoText);
    m_infoLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_infoLabel->setStyleSheet("font-size: 9pt; color: #666666;");
    m_infoLabel->setWordWrap(true);

    Layouting::ColumnWithMargin{m_imageLabel, m_infoLabel}.attachTo(this);

    // 可选：设置固定大小或大小策略，使每个Item看起来整齐
    // setFixedSize(640, 500);
}

void ImageGridItem::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    updatePixmap();
}

void ImageGridItem::updatePixmap()
{
    if (m_imageData.empty()) return;
    QSize labelSize = m_imageLabel->size();
    qDebug() << "labelSize" << labelSize;
}
