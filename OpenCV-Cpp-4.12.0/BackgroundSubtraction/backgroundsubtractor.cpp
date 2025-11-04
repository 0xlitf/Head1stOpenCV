#include "backgroundsubtractor.h"
#include <QApplication>
#include <QDebug>

BackgroundSubtractor::BackgroundSubtractor(QWidget *parent)
    : QWidget(parent), threshold(50)
{
    setupUI();
}

BackgroundSubtractor::~BackgroundSubtractor()
{
}

void BackgroundSubtractor::setupUI()
{
    setWindowTitle("Qt6 背景减除工具");
    setMinimumSize(1000, 800);

    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 控制面板
    QHBoxLayout *controlLayout = new QHBoxLayout();

    loadButtonA = new QPushButton("加载图像 A（有物体）", this);
    loadButtonB = new QPushButton("加载图像 B（纯背景）", this);
    subtractButton = new QPushButton("执行背景减除", this);
    saveButton = new QPushButton("保存结果", this);

    thresholdValueLabel = new QLabel(QString("阈值: %1").arg(threshold), this);
    thresholdSlider = new QSlider(Qt::Horizontal, this);
    thresholdSlider->setRange(0, 255);
    thresholdSlider->setValue(threshold);

    methodComboBox = new QComboBox(this);
    methodComboBox->addItem("简单差值法");
    methodComboBox->addItem("高级背景减除");

    controlLayout->addWidget(loadButtonA);
    controlLayout->addWidget(loadButtonB);
    controlLayout->addWidget(subtractButton);
    controlLayout->addWidget(saveButton);
    controlLayout->addWidget(new QLabel("方法:", this));
    controlLayout->addWidget(methodComboBox);
    controlLayout->addWidget(new QLabel("阈值:", this));
    controlLayout->addWidget(thresholdSlider);
    controlLayout->addWidget(thresholdValueLabel);

    // 图像显示区域
    QHBoxLayout *imageLayout = new QHBoxLayout();

    QGroupBox *groupA = new QGroupBox("图像 A（有物体）", this);
    QGroupBox *groupB = new QGroupBox("图像 B（背景）", this);
    QGroupBox *groupMask = new QGroupBox("背景掩码", this);
    QGroupBox *groupResult = new QGroupBox("提取结果", this);

    QVBoxLayout *layoutA = new QVBoxLayout(groupA);
    QVBoxLayout *layoutB = new QVBoxLayout(groupB);
    QVBoxLayout *layoutMask = new QVBoxLayout(groupMask);
    QVBoxLayout *layoutResult = new QVBoxLayout(groupResult);

    imageALabel = new QLabel("未加载图像", this);
    imageBLabel = new QLabel("未加载图像", this);
    maskLabel = new QLabel("未计算", this);
    resultLabel = new QLabel("无结果", this);

    imageALabel->setAlignment(Qt::AlignCenter);
    imageBLabel->setAlignment(Qt::AlignCenter);
    maskLabel->setAlignment(Qt::AlignCenter);
    resultLabel->setAlignment(Qt::AlignCenter);

    imageALabel->setMinimumSize(300, 300);
    imageBLabel->setMinimumSize(300, 300);
    maskLabel->setMinimumSize(300, 300);
    resultLabel->setMinimumSize(300, 300);

    imageALabel->setStyleSheet("border: 1px solid gray;");
    imageBLabel->setStyleSheet("border: 1px solid gray;");
    maskLabel->setStyleSheet("border: 1px solid gray;");
    resultLabel->setStyleSheet("border: 1px solid gray;");

    layoutA->addWidget(imageALabel);
    layoutB->addWidget(imageBLabel);
    layoutMask->addWidget(maskLabel);
    layoutResult->addWidget(resultLabel);

    QHBoxLayout *groupsLayout = new QHBoxLayout();
    groupsLayout->addWidget(groupA);
    groupsLayout->addWidget(groupB);

    QHBoxLayout *groupsLayout2 = new QHBoxLayout();
    groupsLayout2->addWidget(groupMask);
    groupsLayout2->addWidget(groupResult);

    // 组装主布局
    mainLayout->addLayout(controlLayout);
    mainLayout->addLayout(groupsLayout);
    mainLayout->addLayout(groupsLayout2);

    // 连接信号槽
    connect(loadButtonA, &QPushButton::clicked, this, &BackgroundSubtractor::loadImageA);
    connect(loadButtonB, &QPushButton::clicked, this, &BackgroundSubtractor::loadImageB);
    connect(subtractButton, &QPushButton::clicked, this, &BackgroundSubtractor::subtractBackground);
    connect(saveButton, &QPushButton::clicked, this, &BackgroundSubtractor::saveResult);
    connect(thresholdSlider, &QSlider::valueChanged, this, &BackgroundSubtractor::onThresholdChanged);
    connect(methodComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BackgroundSubtractor::onMethodChanged);
}

void BackgroundSubtractor::loadImageA()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "打开有物体的图像", "", "图像文件 (*.png *.jpg *.bmp *.jpeg)");

    if (!fileName.isEmpty()) {
        imageA.load(fileName);
        if (!imageA.isNull()) {
            imageA = imageA.convertToFormat(QImage::Format_RGB32);
            updateDisplays();
        }
    }
}

void BackgroundSubtractor::loadImageB()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "打开背景图像", "", "图像文件 (*.png *.jpg *.bmp *.jpeg)");

    if (!fileName.isEmpty()) {
        imageB.load(fileName);
        if (!imageB.isNull()) {
            imageB = imageB.convertToFormat(QImage::Format_RGB32);
            updateDisplays();
        }
    }
}

void BackgroundSubtractor::subtractBackground()
{
    if (imageA.isNull() || imageB.isNull()) {
        qDebug() << "请先加载图像A和图像B";
        return;
    }

    if (imageA.size() != imageB.size()) {
        qDebug() << "图像尺寸不匹配，正在调整...";
        imageB = imageB.scaled(imageA.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    if (methodComboBox->currentIndex() == 0) {
        resultImage = subtractSimple(imageA, imageB);
    } else {
        resultImage = subtractAdvanced(imageA, imageB);
    }

    updateDisplays();
}

QImage BackgroundSubtractor::subtractSimple(const QImage &imgA, const QImage &imgB)
{
    QImage result(imgA.size(), QImage::Format_ARGB32);
    maskImage = QImage(imgA.size(), QImage::Format_Grayscale8);

    for (int y = 0; y < imgA.height(); ++y) {
        const QRgb *lineA = reinterpret_cast<const QRgb*>(imgA.constScanLine(y));
        const QRgb *lineB = reinterpret_cast<const QRgb*>(imgB.constScanLine(y));
        QRgb *lineResult = reinterpret_cast<QRgb*>(result.scanLine(y));
        uchar *lineMask = maskImage.scanLine(y);

        for (int x = 0; x < imgA.width(); ++x) {
            QRgb pixelA = lineA[x];
            QRgb pixelB = lineB[x];

            // 计算RGB通道的绝对差值
            int diffR = qAbs(qRed(pixelA) - qRed(pixelB));
            int diffG = qAbs(qGreen(pixelA) - qGreen(pixelB));
            int diffB = qAbs(qBlue(pixelA) - qBlue(pixelB));

            // 计算总体差异度
            int difference = (diffR + diffG + diffB) / 3;

            // 创建二值掩码
            uchar maskValue = (difference > threshold) ? 255 : 0;
            lineMask[x] = maskValue;

            // 如果差异大于阈值，保留原像素；否则设为透明
            if (maskValue == 255) {
                lineResult[x] = pixelA;  // 前景：显示原图
            } else {
                lineResult[x] = qRgba(0, 0, 0, 0);  // 背景：透明
            }
        }
    }

    // 对掩码进行形态学处理
    maskImage = applyMorphologicalOperations(maskImage);

    return result;
}

QImage BackgroundSubtractor::subtractAdvanced(const QImage &imgA, const QImage &imgB)
{
    // 转换为灰度图进行更精确的处理
    QImage grayA = imgA.convertToFormat(QImage::Format_Grayscale8);
    QImage grayB = imgB.convertToFormat(QImage::Format_Grayscale8);

    QImage result(imgA.size(), QImage::Format_ARGB32);
    maskImage = QImage(imgA.size(), QImage::Format_Grayscale8);

    // 第一遍：计算灰度差异
    for (int y = 0; y < grayA.height(); ++y) {
        const uchar *lineA = grayA.constScanLine(y);
        const uchar *lineB = grayB.constScanLine(y);
        uchar *lineMask = maskImage.scanLine(y);

        for (int x = 0; x < grayA.width(); ++x) {
            int diff = qAbs(lineA[x] - lineB[x]);
            lineMask[x] = (diff > threshold) ? 255 : 0;
        }
    }

    // 应用形态学操作
    maskImage = applyMorphologicalOperations(maskImage);

    // 第二遍：使用处理后的掩码提取前景
    for (int y = 0; y < imgA.height(); ++y) {
        const QRgb *lineA = reinterpret_cast<const QRgb*>(imgA.constScanLine(y));
        QRgb *lineResult = reinterpret_cast<QRgb*>(result.scanLine(y));
        const uchar *lineMask = maskImage.constScanLine(y);

        for (int x = 0; x < imgA.width(); ++x) {
            if (lineMask[x] > 128) {  // 前景
                lineResult[x] = lineA[x];
            } else {  // 背景
                // 可以设置为透明、黑色或其他颜色
                lineResult[x] = qRgba(0, 0, 0, 0);  // 透明背景
                // lineResult[x] = qRgb(255, 0, 0);  // 红色背景（用于调试）
            }
        }
    }

    return result;
}

QImage BackgroundSubtractor::applyMorphologicalOperations(const QImage &binaryImage)
{
    QImage result = binaryImage;

    // 简单的形态学操作：先腐蚀后膨胀（开运算）
    int kernelSize = 3;

    // 创建临时图像用于处理
    QImage temp = binaryImage;

    // 腐蚀：去除小噪声点
    for (int y = kernelSize/2; y < binaryImage.height() - kernelSize/2; ++y) {
        const uchar *lines[3];
        for (int i = 0; i < 3; ++i) {
            lines[i] = binaryImage.constScanLine(y + i - kernelSize/2);
        }
        uchar *lineTemp = temp.scanLine(y);

        for (int x = kernelSize/2; x < binaryImage.width() - kernelSize/2; ++x) {
            bool allWhite = true;
            for (int ky = 0; ky < 3; ++ky) {
                for (int kx = 0; kx < 3; ++kx) {
                    if (lines[ky][x + kx - kernelSize/2] < 128) {
                        allWhite = false;
                        break;
                    }
                }
                if (!allWhite) break;
            }
            lineTemp[x] = allWhite ? 255 : 0;
        }
    }

    // 膨胀：填充空洞
    result = temp;
    for (int y = kernelSize/2; y < temp.height() - kernelSize/2; ++y) {
        const uchar *lines[3];
        for (int i = 0; i < 3; ++i) {
            lines[i] = temp.constScanLine(y + i - kernelSize/2);
        }
        uchar *lineResult = result.scanLine(y);

        for (int x = kernelSize/2; x < temp.width() - kernelSize/2; ++x) {
            bool hasWhite = false;
            for (int ky = 0; ky < 3; ++ky) {
                for (int kx = 0; kx < 3; ++kx) {
                    if (lines[ky][x + kx - kernelSize/2] > 128) {
                        hasWhite = true;
                        break;
                    }
                }
                if (hasWhite) break;
            }
            lineResult[x] = hasWhite ? 255 : 0;
        }
    }

    return result;
}

void BackgroundSubtractor::onThresholdChanged(int value)
{
    threshold = value;
    thresholdValueLabel->setText(QString("阈值: %1").arg(threshold));

    // 实时更新结果
    if (!imageA.isNull() && !imageB.isNull()) {
        subtractBackground();
    }
}

void BackgroundSubtractor::onMethodChanged(int index)
{
    Q_UNUSED(index)
    if (!imageA.isNull() && !imageB.isNull()) {
        subtractBackground();
    }
}

void BackgroundSubtractor::saveResult()
{
    if (resultImage.isNull()) return;

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "保存结果图像", "", "PNG图像 (*.png);;JPEG图像 (*.jpg)");

    if (!fileName.isEmpty()) {
        resultImage.save(fileName);
    }
}

void BackgroundSubtractor::updateDisplays()
{
    // 显示图像A
    if (!imageA.isNull()) {
        QPixmap pixmapA = QPixmap::fromImage(imageA)
        .scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        imageALabel->setPixmap(pixmapA);
    }

    // 显示图像B
    if (!imageB.isNull()) {
        QPixmap pixmapB = QPixmap::fromImage(imageB)
        .scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        imageBLabel->setPixmap(pixmapB);
    }

    // 显示结果和掩码
    if (!resultImage.isNull()) {
        QPixmap pixmapResult = QPixmap::fromImage(resultImage)
        .scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        resultLabel->setPixmap(pixmapResult);

        QPixmap pixmapMask = QPixmap::fromImage(maskImage)
                                 .scaled(300, 300, Qt::KeepAspectRatio, Qt::FastTransformation);
        maskLabel->setPixmap(pixmapMask);
    }
}
