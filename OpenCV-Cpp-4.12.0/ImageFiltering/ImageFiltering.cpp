#include "ImageFiltering.h"
#include <QApplication>
#include <QDebug>
#include <opencv2/imgproc.hpp>
#include <vector>


ImageFiltering::ImageFiltering(QWidget *parent)
    : QWidget(parent), threshold(25)
{
    setupUI();

    m_fileNameA = qApp->applicationDirPath() + QDir::separator() + "obj.jpg";
    m_fileNameB = qApp->applicationDirPath() + QDir::separator() + "bg.jpg";

    m_imageA.load(m_fileNameA);
    m_imageB.load(m_fileNameB);
    updateDisplays();
}

ImageFiltering::~ImageFiltering()
{
}

cv::Mat ImageFiltering::ImageFiltering::qImageToCvMat(const QImage &qimage) {
    if (qimage.isNull()) {
        return cv::Mat();
    }

    QImage swapped = qimage;

    // 统一转换为RGB888格式
    if (qimage.format() != QImage::Format_RGB888) {
        swapped = qimage.convertToFormat(QImage::Format_RGB888);
    }

    cv::Mat mat(swapped.height(), swapped.width(), CV_8UC3,
                (void*)swapped.bits(), swapped.bytesPerLine());

    // 注意：QImage是RGB，OpenCV需要BGR
    cv::Mat result;
    cv::cvtColor(mat, result, cv::COLOR_RGB2BGR);

    return result.clone(); // 克隆以确保数据独立
}

QImage ImageFiltering::ImageFiltering::cvMatToQImage(const cv::Mat &mat) {
    if (mat.empty()) {
        return QImage();
    }

    // 处理不同的图像格式
    switch (mat.channels()) {
    case 1: { // 灰度图
        QImage image(mat.data, mat.cols, mat.rows,
                     static_cast<int>(mat.step), QImage::Format_Grayscale8);
        return image.copy();
    }

    case 3: { // 彩色图（BGR → RGB）
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        QImage image(rgb.data, rgb.cols, rgb.rows,
                     static_cast<int>(rgb.step), QImage::Format_RGB888);
        return image.copy();
    }

    case 4: { // 带透明通道
        QImage image(mat.data, mat.cols, mat.rows,
                     static_cast<int>(mat.step), QImage::Format_RGBA8888);
        return image.copy();
    }

    default:
        // 转换不支持的格式
        cv::Mat temp;
        if (mat.channels() == 2) {
            cv::cvtColor(mat, temp, cv::COLOR_BGRA2BGR);
        } else {
            temp = mat;
        }

        if (temp.channels() == 3) {
            cv::cvtColor(temp, temp, cv::COLOR_BGR2RGB);
            QImage image(temp.data, temp.cols, temp.rows,
                         static_cast<int>(temp.step), QImage::Format_RGB888);
            return image.copy();
        }

        qWarning() << "无法转换的图像格式，通道数:" << mat.channels();
        return QImage();
    }
}
// 查找轮廓
std::vector<std::vector<cv::Point>> ImageFiltering::findContours(const cv::Mat &maskMat) {
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;

    // 查找轮廓
    cv::findContours(maskMat, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    return contours;
}

// 过滤小轮廓
std::vector<std::vector<cv::Point>> ImageFiltering::filterLargeContours(
    const std::vector<std::vector<cv::Point>> &contours,
    double minArea) {

    std::vector<std::vector<cv::Point>> largeContours;

    for (const auto &contour : contours) {
        double area = cv::contourArea(contour);
        if (area > minArea) {
            largeContours.push_back(contour);
        }
    }

    return largeContours;
}

// 主函数：提取大物体并添加红色边框
QImage ImageFiltering::extractLargeObjectsWithBoundingBox(const QImage &mask, const QImage &original) {
    if (mask.isNull() || original.isNull() || mask.size() != original.size()) {
        qWarning() << "输入图像无效或尺寸不匹配";
        return QImage();
    }

    // 将QImage转换为cv::Mat
    cv::Mat maskMat = qImageToCvMat(mask);
    cv::Mat originalMat = qImageToCvMat(original);

    // 确保mask是二值图像
    if (maskMat.channels() > 1) {
        cv::cvtColor(maskMat, maskMat, cv::COLOR_BGR2GRAY);
    }

    // 二值化处理（确保是0和255）
    cv::threshold(maskMat, maskMat, 128, 255, cv::THRESH_BINARY);

    // 形态学操作（可选，去除噪声）
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(maskMat, maskMat, cv::MORPH_OPEN, kernel);

    // 查找轮廓
    std::vector<std::vector<cv::Point>> contours = findContours(maskMat);

    if (contours.empty()) {
        qDebug() << "未找到任何轮廓";
        return original; // 返回原图
    }

    qDebug() << "找到轮廓数量:" << contours.size();

    // 过滤小轮廓（面积阈值可调整）
    std::vector<std::vector<cv::Point>> largeContours = filterLargeContours(contours, 1000.0);

    if (largeContours.empty()) {
        qDebug() << "未找到足够大的轮廓";
        return original;
    }

    qDebug() << "大轮廓数量:" << largeContours.size();

    // 创建结果图像（复制原图）
    cv::Mat resultMat = originalMat.clone();

    // 为每个大轮廓绘制红色边框
    for (const auto &contour : largeContours) {
        // 计算最小外接矩形
        cv::Rect boundingRect = cv::boundingRect(contour);

        // 计算轮廓面积
        double area = cv::contourArea(contour);

        // 绘制红色矩形边框（BGR格式：蓝色,绿色,红色）
        cv::Scalar redColor(0, 0, 255); // 红色
        int thickness = 3;

        cv::rectangle(resultMat, boundingRect, redColor, thickness);

        // 可选：在矩形上方显示面积信息
        std::string areaText = "Area: " + std::to_string(static_cast<int>(area));
        cv::Point textPosition(boundingRect.x, boundingRect.y - 10);

        cv::putText(resultMat, areaText, textPosition,
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, redColor, 1);

        qDebug() << "绘制边框: x=" << boundingRect.x << "y=" << boundingRect.y
                 << "width=" << boundingRect.width << "height=" << boundingRect.height
                 << "area=" << area;
    }

    return cvMatToQImage(resultMat);
}

QImage ImageFiltering::extractLargeObjectsWithBoundingBox(const QImage &mask, const QImage &original, double minArea) {
    if (mask.isNull() || original.isNull() || mask.size() != original.size()) {
        qWarning() << "输入图像无效或尺寸不匹配";
        return QImage();
    }

    // 将QImage转换为cv::Mat
    cv::Mat maskMat = qImageToCvMat(mask);
    cv::Mat originalMat = qImageToCvMat(original);

    // 确保mask是二值图像
    if (maskMat.channels() > 1) {
        cv::cvtColor(maskMat, maskMat, cv::COLOR_BGR2GRAY);
    }

    // 二值化处理
    cv::threshold(maskMat, maskMat, 128, 255, cv::THRESH_BINARY);

    // 查找轮廓
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(maskMat, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if (contours.empty()) {
        qDebug() << "未找到任何轮廓";
        return original;
    }

    qDebug() << "找到轮廓数量:" << contours.size();

    // 过滤小轮廓并统计像素数量
    std::vector<std::vector<cv::Point>> largeContours;
    std::vector<int> pixelCounts; // 存储每个轮廓的像素数量

    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);
        if (area >= minArea) {
            largeContours.push_back(contour);

            // 方法1: 使用轮廓面积（近似）
            // pixelCounts.push_back(static_cast<int>(area));

            // 方法2: 精确计算轮廓内像素数量
            cv::Mat contourMask = cv::Mat::zeros(maskMat.size(), CV_8UC1);
            cv::drawContours(contourMask, std::vector<std::vector<cv::Point>>{contour}, -1, 255, cv::FILLED);
            int pixelCount = cv::countNonZero(contourMask);
            pixelCounts.push_back(pixelCount);
        }
    }

    if (largeContours.empty()) {
        qDebug() << "未找到面积大于" << minArea << "的轮廓";
        return original;
    }

    qDebug() << "大轮廓数量:" << largeContours.size();

    // 创建结果图像
    cv::Mat resultMat = originalMat.clone();

    // 为每个大轮廓绘制边界框并显示统计信息
    for (size_t i = 0; i < largeContours.size(); ++i) {
        const auto& contour = largeContours[i];
        int pixelCount = pixelCounts[i];

        // 计算边界矩形和轮廓面积
        cv::Rect boundingRect = cv::boundingRect(contour);
        double contourArea = cv::contourArea(contour);

        // 绘制红色矩形边框
        cv::Scalar redColor(0, 0, 255);
        int thickness = 2;
        cv::rectangle(resultMat, boundingRect, redColor, thickness);

        // 创建信息标签
        std::stringstream label;
        label << "Obj_" << (i+1)
              << " Pixels:" << pixelCount
              << " Area:" << static_cast<int>(contourArea);

        cv::Point textPos(boundingRect.x, boundingRect.y - 5);
        if (textPos.y < 20) textPos.y = boundingRect.y + 20;

        cv::putText(resultMat, label.str(), textPos,
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, redColor, 1);

        qDebug() << QString("物体%1: 位置(%2,%3) 尺寸(%4×%5) 轮廓面积%6 像素数量%7")
                        .arg(i+1).arg(boundingRect.x).arg(boundingRect.y)
                        .arg(boundingRect.width).arg(boundingRect.height)
                        .arg(contourArea).arg(pixelCount);
    }

    return cvMatToQImage(resultMat);
}

void ImageFiltering::setupUI()
{
    setWindowTitle("Qt6 背景减除");
    setMinimumSize(1000, 800);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *controlLayout = new QHBoxLayout();

    m_loadButtonA = new QPushButton("加载图像 A（with_object）", this);
    m_loadButtonB = new QPushButton("加载图像 B（纯背景）", this);
    m_subtractButton = new QPushButton("执行背景减除", this);
    m_saveButton = new QPushButton("保存结果", this);

    m_thresholdValueLabel = new QLabel(QString("阈值: %1").arg(threshold), this);
    m_thresholdSlider = new QSlider(Qt::Horizontal, this);
    m_thresholdSlider->setRange(0, 255);
    m_thresholdSlider->setValue(threshold);

    m_methodComboBox = new QComboBox(this);
    m_methodComboBox->addItem("简单差值法");
    m_methodComboBox->addItem("高级背景减除");

    controlLayout->addWidget(m_loadButtonA);
    controlLayout->addWidget(m_loadButtonB);
    controlLayout->addWidget(m_subtractButton);
    controlLayout->addWidget(m_saveButton);
    controlLayout->addWidget(new QLabel("方法:", this));
    controlLayout->addWidget(m_methodComboBox);
    controlLayout->addWidget(new QLabel("阈值:", this));
    controlLayout->addWidget(m_thresholdSlider);
    controlLayout->addWidget(m_thresholdValueLabel);

    // 图像显示区域
    QHBoxLayout *imageLayout = new QHBoxLayout();

    QGroupBox *groupA = new QGroupBox("图像 A（with_object）", this);
    QGroupBox *groupB = new QGroupBox("图像 B（背景）", this);
    QGroupBox *groupMaskBefore = new QGroupBox("未经形态学背景掩码", this);
    QGroupBox *groupMask = new QGroupBox("经形态学后背景掩码", this);
    QGroupBox *groupResult = new QGroupBox("提取结果", this);
    QGroupBox *groupResultWithRect = new QGroupBox("框选", this);

    QVBoxLayout *layoutA = new QVBoxLayout(groupA);
    QVBoxLayout *layoutB = new QVBoxLayout(groupB);
    QVBoxLayout *layoutMask = new QVBoxLayout(groupMask);
    QVBoxLayout *layoutMaskBefore = new QVBoxLayout(groupMaskBefore);
    QVBoxLayout *layoutResult = new QVBoxLayout(groupResult);
    QVBoxLayout *layoutResultWithRect = new QVBoxLayout(groupResultWithRect);

    m_imageALabel = new QLabel("未加载图像", this);
    m_imageBLabel = new QLabel("未加载图像", this);
    m_maskLabel = new QLabel("未计算", this);
    m_maskLabelBefore = new QLabel("无结果", this);
    m_resultLabel = new QLabel("无结果", this);
    m_resultLabelWithRect = new QLabel("无结果", this);

    m_imageALabel->setAlignment(Qt::AlignCenter);
    m_imageBLabel->setAlignment(Qt::AlignCenter);
    m_maskLabel->setAlignment(Qt::AlignCenter);
    m_maskLabelBefore->setAlignment(Qt::AlignCenter);
    m_resultLabel->setAlignment(Qt::AlignCenter);
    m_resultLabelWithRect->setAlignment(Qt::AlignCenter);

    m_imageALabel->setMinimumSize(300, 300);
    m_imageBLabel->setMinimumSize(300, 300);
    m_maskLabelBefore->setMinimumSize(300, 300);
    m_maskLabel->setMinimumSize(300, 300);
    m_resultLabel->setMinimumSize(300, 300);
    m_resultLabelWithRect->setMinimumSize(300, 300);

    m_imageALabel->setStyleSheet("border: 1px solid gray;");
    m_imageBLabel->setStyleSheet("border: 1px solid gray;");
    m_maskLabel->setStyleSheet("border: 1px solid gray;");
    m_maskLabelBefore->setStyleSheet("border: 1px solid gray;");
    m_resultLabel->setStyleSheet("border: 1px solid gray;");
    m_resultLabelWithRect->setStyleSheet("border: 1px solid gray;");

    layoutA->addWidget(m_imageALabel);
    layoutB->addWidget(m_imageBLabel);
    layoutMaskBefore->addWidget(m_maskLabelBefore);
    layoutMask->addWidget(m_maskLabel);
    layoutResult->addWidget(m_resultLabel);
    layoutResultWithRect->addWidget(m_resultLabelWithRect);

    QHBoxLayout *groupsLayout = new QHBoxLayout();
    groupsLayout->addWidget(groupA);
    groupsLayout->addWidget(groupB);
    groupsLayout->addWidget(groupMaskBefore);

    QHBoxLayout *groupsLayout2 = new QHBoxLayout();
    groupsLayout2->addWidget(groupMask);
    groupsLayout2->addWidget(groupResult);
    groupsLayout2->addWidget(groupResultWithRect);

    // 组装主布局
    mainLayout->addLayout(controlLayout, 1);
    mainLayout->addLayout(groupsLayout, 5);
    mainLayout->addLayout(groupsLayout2, 5);

    // 连接信号槽
    connect(m_loadButtonA, &QPushButton::clicked, this, &ImageFiltering::loadImageA);
    connect(m_loadButtonB, &QPushButton::clicked, this, &ImageFiltering::loadImageB);
    connect(m_subtractButton, &QPushButton::clicked, this, &ImageFiltering::subtractBackground);
    connect(m_saveButton, &QPushButton::clicked, this, &ImageFiltering::saveResult);
    connect(m_thresholdSlider, &QSlider::valueChanged, this, &ImageFiltering::onThresholdChanged);
    connect(m_methodComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ImageFiltering::onMethodChanged);
}

void ImageFiltering::loadImageA()
{
    QString m_fileNameA = QFileDialog::getOpenFileName(this,
                                                    "打开with_object的图像", "", "图像文件 (*.png *.jpg *.bmp *.jpeg)");

    if (!m_fileNameA.isEmpty()) {
        m_imageA.load(m_fileNameA);
        if (!m_imageA.isNull()) {
            m_imageA = m_imageA.convertToFormat(QImage::Format_RGB32);
            updateDisplays();
        }
    }
}

void ImageFiltering::loadImageB()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "打开背景图像", "", "图像文件 (*.png *.jpg *.bmp *.jpeg)");

    if (!fileName.isEmpty()) {
        m_imageB.load(fileName);
        if (!m_imageB.isNull()) {
            m_imageB = m_imageB.convertToFormat(QImage::Format_RGB32);
            updateDisplays();
        }
    }
}

void ImageFiltering::subtractBackground()
{
    if (m_imageA.isNull() || m_imageB.isNull()) {
        qDebug() << "请先加载图像A和图像B";
        return;
    }

    if (m_imageA.size() != m_imageB.size()) {
        qDebug() << "图像尺寸不匹配，正在调整...";
        m_imageB = m_imageB.scaled(m_imageA.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    if (m_methodComboBox->currentIndex() == 0) {
        std::tie(m_resultImage, m_resultImageWithRect) = subtractSimple(m_imageA, m_imageB);
    } else {
        std::tie(m_resultImage, m_resultImageWithRect) = subtractAdvanced(m_imageA, m_imageB);
    }

    updateDisplays();
}

std::tuple<QImage, QImage> ImageFiltering::subtractSimple(const QImage &imgA, const QImage &imgB)
{
    QImage result(imgA.size(), QImage::Format_ARGB32);
    m_maskImageBefore = QImage(imgA.size(), QImage::Format_Grayscale8);

    for (int y = 0; y < imgA.height(); ++y) {
        const QRgb *lineA = reinterpret_cast<const QRgb*>(imgA.constScanLine(y));
        const QRgb *lineB = reinterpret_cast<const QRgb*>(imgB.constScanLine(y));
        QRgb *lineResult = reinterpret_cast<QRgb*>(result.scanLine(y));
        uchar *lineMask = m_maskImageBefore.scanLine(y);

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
    m_maskImage = applyMorphologicalOperations(m_maskImageBefore);

    // 提取大物体并添加红色边框
    QImage finalResult = extractLargeObjectsWithBoundingBox(m_maskImage, imgA, 1000);

    return std::make_tuple(result, finalResult);
}

std::tuple<QImage, QImage> ImageFiltering::subtractAdvanced(const QImage &imgA, const QImage &imgB)
{
    // 转换为灰度图进行更精确的处理
    QImage grayA = imgA.convertToFormat(QImage::Format_Grayscale8);
    QImage grayB = imgB.convertToFormat(QImage::Format_Grayscale8);

    QImage result(imgA.size(), QImage::Format_ARGB32);
    m_maskImageBefore = QImage(imgA.size(), QImage::Format_Grayscale8);

    // 第一遍：计算灰度差异
    for (int y = 0; y < grayA.height(); ++y) {
        const uchar *lineA = grayA.constScanLine(y);
        const uchar *lineB = grayB.constScanLine(y);
        uchar *lineMask = m_maskImageBefore.scanLine(y);

        for (int x = 0; x < grayA.width(); ++x) {
            int diff = qAbs(lineA[x] - lineB[x]);
            lineMask[x] = (diff > threshold) ? 255 : 0;
        }
    }

    // 应用形态学操作
    m_maskImage = applyMorphologicalOperations(m_maskImageBefore);

    // 第二遍：使用处理后的掩码提取前景
    for (int y = 0; y < imgA.height(); ++y) {
        const QRgb *lineA = reinterpret_cast<const QRgb*>(imgA.constScanLine(y));
        QRgb *lineResult = reinterpret_cast<QRgb*>(result.scanLine(y));
        const uchar *lineMask = m_maskImage.constScanLine(y);

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

    // 提取大物体并添加红色边框
    QImage finalResult = extractLargeObjectsWithBoundingBox(m_maskImage, imgA, 1000);

    return std::make_tuple(result, finalResult);
}

QImage ImageFiltering::applyMorphologicalOperations(const QImage &binaryImage)
{
    // 将QImage转换为cv::Mat
    cv::Mat mat = qImageToCvMat(binaryImage);

    if (mat.channels() > 1) {
        cv::cvtColor(mat, mat, cv::COLOR_BGR2GRAY);
    }

    // 二值化确保是0和255
    cv::threshold(mat, mat, 128, 255, cv::THRESH_BINARY);

    // 创建形态学操作核（大小可调整）
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));

    // 先开运算去除小噪声
    cv::morphologyEx(mat, mat, cv::MORPH_OPEN, kernel);

    // 再闭运算填充空洞
    cv::morphologyEx(mat, mat, cv::MORPH_CLOSE, kernel);

    return cvMatToQImage(mat);
}

void ImageFiltering::onThresholdChanged(int value)
{
    threshold = value;
    m_thresholdValueLabel->setText(QString("阈值: %1").arg(threshold));

    // 实时更新结果
    if (!m_imageA.isNull() && !m_imageB.isNull()) {
        subtractBackground();
    }
}

void ImageFiltering::onMethodChanged(int index)
{
    Q_UNUSED(index)
    if (!m_imageA.isNull() && !m_imageB.isNull()) {
        subtractBackground();
    }
}

void ImageFiltering::saveResult()
{
    qDebug() << "m_fileNameA: " << m_fileNameA;
    if (m_resultImage.isNull()) {
        return;
    }
    if (m_fileNameA.isEmpty()) {
        return;
    }

    QFileInfo fileInfo(m_fileNameA);
    QString dirPath = fileInfo.absolutePath();
    QString baseName = fileInfo.baseName();
    QString suffix = fileInfo.suffix();

    QString fileName5 = dirPath + QDir::separator() + baseName + "-5." + suffix;
    QString fileName6 = dirPath + QDir::separator() + baseName + "-6." + suffix;

    qDebug() << "saveResult: " << fileName5;
    qDebug() << "saveResult: " << fileName6;

    if (!m_fileNameA.isEmpty()) {
        m_resultImage.save(fileName5);
        m_resultImageWithRect.save(fileName6);
    }
}

void ImageFiltering::updateDisplays()
{
    // 显示图像A
    if (!m_imageA.isNull()) {
        QPixmap pixmapA = QPixmap::fromImage(m_imageA).scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_imageALabel->setPixmap(pixmapA);
    }

    // 显示图像B
    if (!m_imageB.isNull()) {
        QPixmap pixmapB = QPixmap::fromImage(m_imageB).scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_imageBLabel->setPixmap(pixmapB);
    }

    // 显示结果和掩码
    if (!m_maskImageBefore.isNull()) {
        QPixmap maskResult = QPixmap::fromImage(m_maskImageBefore).scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_maskLabelBefore->setPixmap(maskResult);
    }

    if (!m_maskImage.isNull()) {
        QPixmap pixmapMask = QPixmap::fromImage(m_maskImage).scaled(300, 300, Qt::KeepAspectRatio, Qt::FastTransformation);
        m_maskLabel->setPixmap(pixmapMask);
    }

    if (!m_resultImage.isNull()) {
        QPixmap pixmapResult = QPixmap::fromImage(m_resultImage).scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_resultLabel->setPixmap(pixmapResult);
    }

    if (!m_resultImageWithRect.isNull()) {
        QPixmap pixmapResultWithRect = QPixmap::fromImage(m_resultImageWithRect).scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_resultLabelWithRect->setPixmap(pixmapResultWithRect);
    }
}
