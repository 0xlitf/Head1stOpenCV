#include "minimumbounding.h"
#include <QDebug>
#include <opencv2/opencv.hpp>

MinimumBounding::MinimumBounding(QObject *parent)
    : QObject{parent}
{
}

cv::Mat MinimumBounding::findAndCropObject(const cv::Mat& inputImage)
{
    // 0. 输入验证
    if (inputImage.empty()) {
        qWarning() << "输入图像为空！";
        return cv::Mat();
    }

    // 1. 转换为灰度图
    cv::Mat gray;
    if (inputImage.channels() == 3) {
        cv::cvtColor(inputImage, gray, cv::COLOR_BGR2GRAY);
    } else if (inputImage.channels() == 1) {
        gray = inputImage.clone();
    } else {
        qWarning() << "不支持的图像通道数:" << inputImage.channels();
        return cv::Mat();
    }

    // 2. 二值化处理 - 白底黑物情况下取反，让物体变为白色（前景）
    cv::Mat thresh;
    cv::threshold(gray, thresh, 250, 255, cv::THRESH_BINARY_INV);

    // 3. 形态学操作去除噪声 [4](@ref)
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(thresh, thresh, cv::MORPH_OPEN, kernel);

    // 4. 查找轮廓 [1,2](@ref)
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 5. 找到面积最大的轮廓（假设最主要的物体）
    if (contours.empty()) {
        qWarning() << "未找到任何轮廓！";
        return cv::Mat();
    }

    auto maxContour = std::max_element(contours.begin(), contours.end(),
                                       [](const std::vector<cv::Point>& c1, const std::vector<cv::Point>& c2) {
                                           return cv::contourArea(c1) < cv::contourArea(c2);
                                       });

    // 6. 计算最小外接矩形 [1,3](@ref)
    cv::RotatedRect minRect = cv::minAreaRect(*maxContour);

    // 获取矩形尺寸和角度
    cv::Size2f rectSize = minRect.size;
    float angle = minRect.angle;

    // 7. 调整方向：确保长边为水平方向（横向），短边为垂直方向（纵向）
    bool needSwap = false;
    if (rectSize.width < rectSize.height) {
        // 如果当前是短边横向，长边纵向，则交换宽高
        std::swap(rectSize.width, rectSize.height);
        angle += 90.0f;  // 调整角度
        needSwap = true;
    }

    // 8. 进行仿射变换，扭正图像 [4,10](@ref)
    cv::Mat rotated;
    cv::Mat rotationMatrix = cv::getRotationMatrix2D(minRect.center, angle, 1.0);
    cv::warpAffine(inputImage, rotated, rotationMatrix, inputImage.size(),
                   cv::INTER_CUBIC, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255));

    // 9. 从扭正后的图像中截取感兴趣区域(ROI) [1](@ref)
    cv::Mat cropped;
    cv::getRectSubPix(rotated,
                      cv::Size(static_cast<int>(rectSize.width),
                               static_cast<int>(rectSize.height)),
                      minRect.center, cropped);

    // 10. 创建纯色背景 [6,8](@ref)
    cv::Mat blackBackground = cv::Mat::zeros(cropped.size(), cropped.type());
    // cv::Mat whiteBackground(cropped.size(), cropped.type(), cv::Scalar(255, 255, 255));

    // 11. 提取前景物体并放置到黑色背景上
    if (cropped.channels() == 3) {
        // 对于彩色图像，转换为灰度图创建掩码
        cv::Mat grayCropped;
        cv::cvtColor(cropped, grayCropped, cv::COLOR_BGR2GRAY);

        // 创建前景掩码 - 非白色区域视为前景
        cv::Mat foregroundMask;
        cv::threshold(grayCropped, foregroundMask, 250, 255, cv::THRESH_BINARY);

        // 反转掩码，使物体区域为白色
        cv::bitwise_not(foregroundMask, foregroundMask);

        // 对掩码进行形态学操作，去除噪声 [4](@ref)
        cv::morphologyEx(foregroundMask, foregroundMask, cv::MORPH_CLOSE, kernel);

        // 将前景物体复制到黑色背景上
        cropped.copyTo(blackBackground, foregroundMask);
    } else {
        // 对于灰度图像，直接使用阈值处理
        cv::Mat foregroundMask;
        cv::threshold(cropped, foregroundMask, 250, 255, cv::THRESH_BINARY);
        cv::bitwise_not(foregroundMask, foregroundMask);
        cropped.copyTo(blackBackground, foregroundMask);
    }

    // 12. 最终验证：确保输出图像不为空
    if (blackBackground.empty()) {
        qWarning() << "处理后的图像为空！";
        return cv::Mat();
    }

    return blackBackground;
}

cv::Mat MinimumBounding::fillCenterWithWhite(const cv::Mat& inputImage, int borderWidth) {
    if (inputImage.empty()) {
        return cv::Mat();
    }

    // 创建原图的副本
    cv::Mat result = inputImage.clone();

    // 计算中心矩形的坐标[1,3](@ref)
    int centerWidth = inputImage.cols - 2 * borderWidth;
    int centerHeight = inputImage.rows - 2 * borderWidth;

    // 确保中心矩形有有效的尺寸[6](@ref)
    if (centerWidth <= 0 || centerHeight <= 0) {
        // 如果边框宽度太大，直接返回全白图像
        result = cv::Mat::ones(inputImage.size(), inputImage.type()) * 255;
        return result;
    }

    // 计算中心矩形的左上角和右下角坐标[1,3](@ref)
    cv::Point topLeft(borderWidth, borderWidth);
    cv::Point bottomRight(inputImage.cols - borderWidth, inputImage.rows - borderWidth);

    // 使用白色填充中心矩形区域[1,6,7](@ref)
    // thickness参数设置为-1表示填充矩形[3,4](@ref)
    cv::rectangle(result, topLeft, bottomRight, cv::Scalar(255, 255, 255), -1);

    return result;
}

cv::Mat MinimumBounding::removeOuterBorder(const cv::Mat& inputImage, int borderWidth) {
    // 检查输入图像是否有效
    if (inputImage.empty()) {
        qDebug() << "错误：输入图像为空。";
        return cv::Mat();
    }

    // 检查边框宽度是否有效[1](@ref)
    int imgWidth = inputImage.cols;
    int imgHeight = inputImage.rows;

    // 计算移除边框后新图像的尺寸
    int newWidth = imgWidth - 2 * borderWidth;
    int newHeight = imgHeight - 2 * borderWidth;

    // 有效性检查：确保新图像的宽和高都大于0[1](@ref)
    if (borderWidth <= 0 || newWidth <= 0 || newHeight <= 0) {
        qDebug() << "错误：边框宽度" << borderWidth << "无效。"
                 << "图像尺寸(" << imgWidth << "x" << imgHeight << ")"
                 << "减去边框后尺寸(" << newWidth << "x" << newHeight << ")无效。";
        return cv::Mat();
    }

    // 定义要保留的矩形区域 (ROI)[2](@ref)
    // 参数说明: Rect(x, y, width, height)
    // x, y: 矩形左上角坐标，从原图的 (borderWidth, borderWidth) 开始
    // width, height: 新图像的宽度和高度
    cv::Rect roi(borderWidth, borderWidth, newWidth, newHeight);

    // 检查ROI是否在图像边界内[1](@ref)
    if (roi.x < 0 || roi.y < 0 || roi.x + roi.width > imgWidth || roi.y + roi.height > imgHeight) {
        qDebug() << "错误：计算的ROI区域超出了图像边界。";
        return cv::Mat();
    }

    // 使用cv::Mat的拷贝构造函数创建ROI的副本[2](@ref)
    // 注意：这里使用.clone()确保返回的是深拷贝，独立于原图
    cv::Mat croppedImage = inputImage(roi).clone();

    return croppedImage;
}
