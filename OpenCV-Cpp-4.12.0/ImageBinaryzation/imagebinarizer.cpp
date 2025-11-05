#include "imagebinarizer.h"

void ImageBinarizer::loadImage() {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "打开图像", "", "图像文件 (*.png *.jpg *.bmp *.jpeg)");

    if (!fileName.isEmpty()) {
        originalImage.load(fileName);
        if (!originalImage.isNull()) {
            displayOriginalImage();
            binarizeImage();
        }
    }
}

void ImageBinarizer::onThresholdChanged(int value) {
    thresholdValue = value;
    thresholdLabel->setText(QString("阈值: %1").arg(value));
    if (!originalImage.isNull()) {
        binarizeImage();
    }
}

void ImageBinarizer::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 创建控件
    QPushButton *loadButton = new QPushButton("加载图像", this);
    QHBoxLayout *sliderLayout = new QHBoxLayout();

    thresholdLabel = new QLabel("阈值: 200", this);
    QSlider *thresholdSlider = new QSlider(Qt::Horizontal, this);
    thresholdSlider->setRange(0, 255);
    thresholdSlider->setValue(200);

    sliderLayout->addWidget(loadButton);
    sliderLayout->addWidget(new QLabel("阈值:", this));
    sliderLayout->addWidget(thresholdSlider);
    sliderLayout->addWidget(thresholdLabel);

    // 图像显示标签
    originalLabel = new QLabel("原图", this);
    binaryLabel = new QLabel("二值图", this);

    originalLabel->setAlignment(Qt::AlignCenter);
    binaryLabel->setAlignment(Qt::AlignCenter);
    originalLabel->setMinimumSize(400, 300);
    binaryLabel->setMinimumSize(400, 300);

    // 布局
    // mainLayout->addWidget(loadButton);
    mainLayout->addLayout(sliderLayout, 1);

    QHBoxLayout *imageLayout = new QHBoxLayout();
    imageLayout->addWidget(originalLabel);
    imageLayout->addWidget(binaryLabel);

    mainLayout->addLayout(imageLayout, 10);

    // 连接信号槽
    connect(loadButton, &QPushButton::clicked, this, &ImageBinarizer::loadImage);
    connect(thresholdSlider, &QSlider::valueChanged, this, &ImageBinarizer::onThresholdChanged);
}

void ImageBinarizer::displayOriginalImage() {
    QPixmap pixmap = QPixmap::fromImage(originalImage)
    .scaled(400, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    originalLabel->setPixmap(pixmap);
}

void ImageBinarizer::binarizeImage() {
    if (originalImage.isNull()) return;

    // 转换为灰度图（如果不是的话）
    QImage grayImage;
    if (originalImage.format() != QImage::Format_Grayscale8) {
        grayImage = originalImage.convertToFormat(QImage::Format_Grayscale8);
    } else {
        grayImage = originalImage;
    }

    // 创建二值图像（单通道）
    QImage binaryImage(grayImage.size(), QImage::Format_Mono);

    // 设置二值化颜色表：索引0=黑，索引1=白
    binaryImage.setColorTable({qRgb(0, 0, 0), qRgb(255, 255, 255)});

    // 执行二值化
    for (int y = 0; y < grayImage.height(); ++y) {
        const uchar *grayLine = grayImage.constScanLine(y);
        uchar *binaryLine = binaryImage.scanLine(y);

        for (int x = 0; x < grayImage.width(); ++x) {
            uchar grayValue = grayLine[x];
            // 如果灰度值大于阈值，设为白色(1)，否则黑色(0)
            if (grayValue > thresholdValue) {
                // 设置对应位为1（白色）
                binaryLine[x / 8] |= (1 << (7 - (x % 8)));
            } else {
                // 设置对应位为0（黑色）
                binaryLine[x / 8] &= ~(1 << (7 - (x % 8)));
            }
        }
    }

    // 显示结果
    QPixmap binaryPixmap = QPixmap::fromImage(binaryImage)
                               .scaled(400, 300, Qt::KeepAspectRatio, Qt::FastTransformation);
    binaryLabel->setPixmap(binaryPixmap);
}
