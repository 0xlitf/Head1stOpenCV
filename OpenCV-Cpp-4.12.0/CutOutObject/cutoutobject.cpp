#include "cutoutobject.h"

CutOutObject::CutOutObject() {}

bool CutOutObject::extractLargestContour(const cv::Mat &inputImage,
                                         std::vector<cv::Point> &contour,
                                         double &area, int colorThreshold,
                                         int blueThreshold, int kernelSize) {

    if (inputImage.empty()) {
        std::cerr << "错误：输入图像为空！" << std::endl;
        return false;
    }

    // 步骤1：蓝色区域提取（使用图片中的算法）
    cv::Mat cvImage = inputImage.clone();
    for (int i = 0; i < cvImage.rows; ++i) {
        for (int j = 0; j < cvImage.cols; ++j) {
            cv::Vec3b &pixel = cvImage.at<cv::Vec3b>(i, j);
            if ((pixel[0] - pixel[1] > colorThreshold) &&
                (pixel[0] - pixel[2] > colorThreshold) &&
                (pixel[0] > blueThreshold)) {
                // 蓝色区域：设为白色
                pixel[0] = 255;
                pixel[1] = 255;
                pixel[2] = 255;
            } else {
                // 非蓝色区域：设为黑色
                pixel[0] = 0;
                pixel[1] = 0;
                pixel[2] = 0;
            }
        }
    }

    cv::bitwise_not(cvImage, cvImage);

    cv::imshow("宽松阈值 (30,50)", cvImage);

    // 转换为灰度图用于形态学操作
    cv::Mat gray;
    cv::cvtColor(cvImage, gray, cv::COLOR_BGR2GRAY);

    // 步骤2：形态学操作 - 去除噪声和小区域
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE,
                                               cv::Size(kernelSize, kernelSize));

    // 开操作：先腐蚀后膨胀，去除小噪点
    cv::Mat morphResult;
    cv::morphologyEx(gray, morphResult, cv::MORPH_OPEN, kernel);

    // 闭操作：先膨胀后腐蚀，填充内部空洞
    cv::morphologyEx(morphResult, morphResult, cv::MORPH_CLOSE, kernel);

    // 可选：额外的膨胀操作连接相邻区域
    cv::dilate(morphResult, morphResult, kernel, cv::Point(-1, -1), 2);

    // 步骤3：轮廓检测
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(morphResult, contours, hierarchy, cv::RETR_EXTERNAL,
                     cv::CHAIN_APPROX_SIMPLE);

    if (contours.empty()) {
        std::cout << "未找到任何轮廓！" << std::endl;
        return false;
    }

    // 步骤4：找到面积最大的轮廓
    auto maxContourIt = std::max_element(
        contours.begin(), contours.end(),
        [](const std::vector<cv::Point> &a, const std::vector<cv::Point> &b) {
            return cv::contourArea(a) < cv::contourArea(b);
        });

    if (maxContourIt == contours.end()) {
        return false;
    }

    contour = *maxContourIt;
    area = cv::contourArea(contour);

    return true;
}

bool CutOutObject::extractLargestContourEnhanced(
    const cv::Mat &inputImage, std::vector<cv::Point> &contour, double &area,
    int colorThreshold, int blueThreshold, int openIterations,
    int closeIterations, int dilateIterations) {

    if (inputImage.empty())
        return false;

    // 蓝色区域提取
    cv::Mat blueMask = inputImage.clone();
    for (int i = 0; i < blueMask.rows; ++i) {
        for (int j = 0; j < blueMask.cols; ++j) {
            cv::Vec3b &pixel = blueMask.at<cv::Vec3b>(i, j);
            if ((pixel[0] - pixel[1] > colorThreshold) &&
                (pixel[0] - pixel[2] > colorThreshold) &&
                (pixel[0] > blueThreshold)) {
                pixel = cv::Vec3b(255, 255, 255);
            } else {
                pixel = cv::Vec3b(0, 0, 0);
            }
        }
    }

    cv::bitwise_not(blueMask, blueMask);

    // 转换为灰度图
    cv::Mat gray;
    cv::cvtColor(blueMask, gray, cv::COLOR_BGR2GRAY);

    // 形态学核
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));

    // 形态学操作序列
    cv::Mat processed;
    gray.copyTo(processed);

    // 开操作去除噪点
    if (openIterations > 0) {
        cv::morphologyEx(processed, processed, cv::MORPH_OPEN, kernel,
                         cv::Point(-1, -1), openIterations);
    }

    // 闭操作填充空洞
    if (closeIterations > 0) {
        cv::morphologyEx(processed, processed, cv::MORPH_CLOSE, kernel,
                         cv::Point(-1, -1), closeIterations);
    }

    // 膨胀连接区域
    if (dilateIterations > 0) {
        cv::dilate(processed, processed, kernel, cv::Point(-1, -1),
                   dilateIterations);
    }

    // 轮廓检测
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(processed, contours, cv::RETR_EXTERNAL,
                     cv::CHAIN_APPROX_SIMPLE);

    if (contours.empty())
        return false;

    // 找到最大轮廓
    int maxIndex = -1;
    double maxArea = 0;
    for (size_t i = 0; i < contours.size(); i++) {
        double currentArea = cv::contourArea(contours[i]);
        if (currentArea > maxArea) {
            maxArea = currentArea;
            maxIndex = i;
        }
    }

    if (maxIndex == -1)
        return false;

    contour = contours[maxIndex];
    area = maxArea;

    return true;
}
