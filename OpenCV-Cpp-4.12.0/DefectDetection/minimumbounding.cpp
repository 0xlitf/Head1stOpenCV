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

    // 2. 二值化处理，以便查找轮廓。白底黑物的情况下，我们取反，让物体变为白色（前景）。
    cv::Mat thresh;
    cv::threshold(gray, thresh, 250, 255, cv::THRESH_BINARY_INV); // 阈值可调整

    // 3. 查找轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 4. 找到面积最大的轮廓（假设最主要的物体）
    if (contours.empty()) {
        qWarning() << "未找到任何轮廓！";
        return cv::Mat();
    }

    auto maxContour = std::max_element(contours.begin(), contours.end(),
                                       [](const std::vector<cv::Point>& c1, const std::vector<cv::Point>& c2) {
                                           return cv::contourArea(c1) < cv::contourArea(c2);
                                       });

    // 5. 计算最小外接矩形[1,5](@ref)
    cv::RotatedRect minRect = cv::minAreaRect(*maxContour);
    cv::Mat rotated; // 用于存储旋转后的图像
    cv::Mat cropped; // 用于存储裁剪出的矩形区域

    // 获取外接矩形的尺寸，确保长边为水平方向
    cv::Size2f rectSize = minRect.size;
    float angle = minRect.angle;
    // 如果矩形的宽大于高，说明当前不是我们想要的“横向长边”方向，需要调整
    if (rectSize.width > rectSize.height) {
        // 交换宽高，并将角度调整90度，使长边始终为水平方向
        std::swap(rectSize.width, rectSize.height);
        angle += 90.0f;
    }

    // 6. 进行仿射变换，扭正图像
    cv::Mat rotationMatrix = cv::getRotationMatrix2D(minRect.center, angle, 1.0);
    cv::warpAffine(inputImage, rotated, rotationMatrix, inputImage.size(), cv::INTER_CUBIC);

    // 7. 从扭正后的图像中截取感兴趣区域(ROI)[5](@ref)
    cv::getRectSubPix(rotated, cv::Size(static_cast<int>(rectSize.width), static_cast<int>(rectSize.height)),
                      minRect.center, cropped);

    // 8. 返回处理后的图像
    return cropped;
}
