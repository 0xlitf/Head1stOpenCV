#include "imagegriditem.h"

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
    QPixmap pixmap = matToPixmap(m_imageData);
    // 缩放缩略图到合适大小
    QPixmap scaledPixmap = pixmap.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
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
    setFixedSize(150, 200);
}

QPixmap ImageGridItem::matToPixmap(const cv::Mat& mat)
{
    // 注意颜色空间转换：OpenCV默认是BGR，Qt默认是RGB
    cv::Mat rgbMat;
    if (mat.channels() == 3) {
        cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);
    } else if (mat.channels() == 1) {
        cv::cvtColor(mat, rgbMat, cv::COLOR_GRAY2RGB);
    } else {
        rgbMat = mat.clone(); // 其他情况直接复制，可能需要额外处理
    }

    // 将Mat转换为QImage，进而转换为QPixmap
    QImage img(rgbMat.data, rgbMat.cols, rgbMat.rows, rgbMat.step, QImage::Format_RGB888);
    return QPixmap::fromImage(img);
}
