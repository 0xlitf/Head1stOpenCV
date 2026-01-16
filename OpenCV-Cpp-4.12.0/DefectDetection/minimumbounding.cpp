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
    // cv::Mat blackBackground = cv::Mat::zeros(cropped.size(), cropped.type());
    cv::Mat whiteBackground(cropped.size(), cropped.type(), cv::Scalar(255, 255, 255));

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
        cropped.copyTo(whiteBackground, foregroundMask);
    } else {
        // 对于灰度图像，直接使用阈值处理
        cv::Mat foregroundMask;
        cv::threshold(cropped, foregroundMask, 250, 255, cv::THRESH_BINARY);
        cv::bitwise_not(foregroundMask, foregroundMask);
        cropped.copyTo(whiteBackground, foregroundMask);
    }

    // 12. 最终验证：确保输出图像不为空
    if (whiteBackground.empty()) {
        qWarning() << "处理后的图像为空！";
        return cv::Mat();
    }

    return whiteBackground;
}
