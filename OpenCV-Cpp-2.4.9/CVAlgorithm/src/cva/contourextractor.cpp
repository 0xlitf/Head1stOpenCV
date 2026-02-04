#include "contourextractor.h"
#include <QDebug>

std::tuple<int, cv::Mat> ContourExtractor::analyzeAndDrawContour(const cv::Mat &inputImage, int whiteThreshold, int areaThreshold) {
    auto filteredContours = ContourExtractor::findContours(inputImage, whiteThreshold, areaThreshold);

    cv::Mat outputImage;
    if (inputImage.channels() == 1) {
        cv::cvtColor(inputImage, outputImage, cv::COLOR_GRAY2BGR);
    } else {
        outputImage = inputImage.clone();
    }

    int contourCount = static_cast<int>(filteredContours.size());

    // 6. 根据轮廓数量选择颜色
    cv::Scalar contourColor;
    if (contourCount > 1) {
        contourColor = cv::Scalar(0, 0, 255); // 红色 (BGR) - 多个轮廓
    } else if (contourCount == 1) {
        contourColor = cv::Scalar(0, 255, 0); // 绿色 (BGR) - 单个轮廓
    } else {
        contourColor = cv::Scalar(255, 255, 255); // 白色 - 无轮廓
    }

    // 7. 绘制轮廓线
    for (size_t i = 0; i < filteredContours.size(); ++i) {
        if (filteredContours[i].empty() || filteredContours[i].size() < 3)
            continue;

        // 绘制轮廓线
        cv::drawContours(outputImage, filteredContours, static_cast<int>(i),
                         contourColor, // 轮廓颜色
                         2,            // 线宽
                         CV_AA);       // 抗锯齿

        if (bool drawContourInfo = false) {
            // 可选：添加轮廓信息
            double area = cv::contourArea(filteredContours[i]);

            // 计算轮廓中心点
            cv::Moments m = cv::moments(filteredContours[i]);
            if (m.m00 != 0) {
                int centerX = static_cast<int>(m.m10 / m.m00);
                int centerY = static_cast<int>(m.m01 / m.m00);

                std::string info = "C" + std::to_string(i + 1) + " A:" + std::to_string(static_cast<int>(area));

                // 绘制中心点
                cv::circle(outputImage, cv::Point(centerX, centerY), 4, contourColor, -1);

                // 添加文本信息
                cv::Point textPos(centerX + 10, centerY);

                // 绘制文本背景
                int baseline = 0;
                cv::Size textSize = cv::getTextSize(info, cv::FONT_HERSHEY_SIMPLEX, 0.4, 1, &baseline);
                cv::rectangle(outputImage, textPos - cv::Point(2, textSize.height + 2), textPos + cv::Point(textSize.width + 2, 2), cv::Scalar(0, 0, 0), -1);

                cv::putText(outputImage, info, textPos, cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(125, 125, 125), 1);
            }
        }
    }

    std::string statusText;
    if (contourCount > 1) {
        statusText = "检测到 " + std::to_string(contourCount) + " 个轮廓 (红色)";
    } else if (contourCount == 1) {
        statusText = "检测到 1 个轮廓 (绿色)";
    } else {
        statusText = "未检测到轮廓";
    }

    // cv::putText(outputImage, statusText, cv::Point(10, 30),
    //             cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(125, 125, 125), 2);

    return std::make_tuple(contourCount, outputImage);
}

std::vector<std::vector<cv::Point>> ContourExtractor::extractWhiteBackgroundContour(const cv::Mat &inputImage) {
    std::vector<std::vector<cv::Point>> resultContours;

    if (inputImage.empty()) {
        qWarning() << "错误：输入图像为空！";
        return resultContours;
    }

    // 1. 预处理图像
    cv::Mat processed = preprocessForWhiteBackground(inputImage);

    // 2. 自动分割白色背景
    cv::Mat binary = segmentWhiteBackground(processed);

    // 3. 查找并过滤轮廓
    resultContours = findAndFilterContours(binary);

    // 4. 验证结果
    if (resultContours.empty()) {
        qWarning() << "警告：未找到符合条件的轮廓！";
    } else {
        qWarning() << "成功提取 " << resultContours.size() << " 个轮廓";
    }

    return resultContours;
}

std::tuple<std::vector<std::vector<cv::Point>>, cv::Mat> ContourExtractor::extractContourWithDebug(const cv::Mat &inputImage) {
    std::vector<std::vector<cv::Point>> contours;
    cv::Mat debugImage;

    if (inputImage.empty()) {
        return std::make_tuple(contours, debugImage);
    }

    // 1. 预处理
    cv::Mat processed = preprocessForWhiteBackground(inputImage);

    // 2. 分割
    cv::Mat binary = segmentWhiteBackground(processed);
    // cv::imshow("binary", binary);

    // 3. 查找轮廓
    contours = findAndFilterContours(binary);

    // 4. 创建调试图像
    debugImage = createDebugImage(inputImage, processed, binary, contours);

    return std::make_tuple(contours, debugImage);
}

cv::Mat ContourExtractor::preprocessForWhiteBackground(const cv::Mat &input) {
    cv::Mat result;

    // 转换为灰度图
    if (input.channels() == 3) {
        cv::cvtColor(input, result, cv::COLOR_BGR2GRAY);
    } else {
        result = input.clone();
    }

    // 增强对比度：CLAHE（限制对比度自适应直方图均衡化）
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    clahe->apply(result, result);

    // 高斯模糊去噪
    cv::GaussianBlur(result, result, cv::Size(5, 5), 0);

    return result;
}

cv::Mat ContourExtractor::segmentWhiteBackground(const cv::Mat &grayImage) {
    cv::Mat binary;

    // 方法1：自适应阈值（适合光照不均匀的情况）
    // cv::adaptiveThreshold(grayImage, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, m_adaptiveBlockSize, 2);
    // cv::threshold(grayImage, binary, 240, 255, cv::THRESH_BINARY_INV);

    // 方法2：Otsu阈值（适合直方图双峰明显的情况）
    // double otsuThresh = cv::threshold(grayImage, binary, 0, 255,
    //                                  cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
    // qDebug() << "Otsu阈值: " << otsuThresh;

    // cv::imshow("grayImage", grayImage);
    // 方法3：固定阈值（如果背景非常稳定）
    cv::threshold(grayImage, binary, 240, 255, cv::THRESH_BINARY_INV);

    // double minVal, maxVal;
    // cv::Point minLoc, maxLoc; // 位置信息如果不需要，可以忽略

    // cv::minMaxLoc(binary, &minVal, &maxVal, &minLoc, &maxLoc);

    // qDebug() << "Min value: " << minVal << ", Max value: " << maxVal;
    // qDebug() << "Binary image type: " << binary.type() << " (CV_8U is " << CV_8U << ")";

    // 去除小噪声
    // if (m_removeNoise && m_noiseKernelSize > 0) {
    //     cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(m_noiseKernelSize, m_noiseKernelSize));
    //     cv::morphologyEx(binary, binary, cv::MORPH_OPEN, kernel);
    // }

    return binary;
}

std::vector<std::vector<cv::Point>> ContourExtractor::findAndFilterContours(const cv::Mat &binary) {
    std::vector<std::vector<cv::Point>> allContours;
    std::vector<cv::Vec4i> hierarchy;

    auto binaryClone = binary.clone();

    cv::findContours(binaryClone, allContours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<std::vector<cv::Point>> filteredContours;

    for (const auto &contour : allContours) {
        if (contour.size() < 3) {
            continue; // 跳过点数太少的轮廓
        }

        double area = cv::contourArea(contour);

        if (area > m_minContourArea) {
            filteredContours.push_back(contour);

            // 可选：输出轮廓信息用于调试
            // cv::Rect bbox = cv::boundingRect(contour);
            // qDebug() << "轮廓面积: " << area
            //           << ", 边界框: " << bbox
            //           << ", 点数: " << contour.size();
        }
    }

    // 按面积从大到小排序（可选）
    std::sort(filteredContours.begin(), filteredContours.end(), [](const std::vector<cv::Point> &c1, const std::vector<cv::Point> &c2) { return cv::contourArea(c1) > cv::contourArea(c2); });

    return filteredContours;
}

cv::Mat ContourExtractor::createDebugImage(const cv::Mat &original, const cv::Mat &processed, const cv::Mat &binary, const std::vector<std::vector<cv::Point>> &contours) {
    cv::Mat debugImage;

    // 创建3行2列的调试图
    std::vector<cv::Mat> rows;

    // 第一行：原始图和灰度图
    cv::Mat row1;
    cv::hconcat(original, convertTo3Channel(processed), row1);
    cv::putText(row1, "original", cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 255), 2);
    cv::putText(row1, "processed", cv::Point(original.cols + 10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 255), 2);

    // 第二行：二值化图和轮廓图

    // double minVal, maxVal;
    // cv::Point minLoc, maxLoc; // 位置信息如果不需要，可以忽略

    // cv::minMaxLoc(binary, &minVal, &maxVal, &minLoc, &maxLoc);

    // qDebug() << "Min value: " << minVal << ", Max value: " << maxVal;
    // qDebug() << "Binary image type: " << binary.type() << " (CV_8U is " << CV_8U << ")";

    cv::Mat binaryColor;
    cv::cvtColor(binary, binaryColor, cv::COLOR_GRAY2BGR);

    cv::Mat contourImage = original.clone();
    if (contourImage.channels() == 1) {
        cv::cvtColor(contourImage, contourImage, cv::COLOR_GRAY2BGR);
    }

    cv::drawContours(contourImage, contours, -1, cv::Scalar(0, 255, 0), 2);

    // 在轮廓图上标注面积
    for (size_t i = 0; i < contours.size(); ++i) {
        double area = cv::contourArea(contours[i]);
        cv::Moments m = cv::moments(contours[i]);
        if (m.m00 != 0) {
            cv::Point center(m.m10 / m.m00, m.m01 / m.m00);
            std::string text = "Area: " + std::to_string(static_cast<int>(area));
            cv::putText(contourImage, text, center, cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);
        }
    }

    cv::Mat row2;
    cv::hconcat(binaryColor, contourImage, row2);
    cv::putText(row2, "binary", cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 255), 2);
    cv::putText(row2, "contours", cv::Point(binaryColor.cols + 10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 255), 2);

    // 垂直拼接
    cv::vconcat(row1, row2, debugImage);

    // 添加总结信息
    std::string summary = "detect " + std::to_string(contours.size()) + " contour (area > " + std::to_string(static_cast<int>(m_minContourArea)) + ")";
    cv::putText(debugImage, summary, cv::Point(10, debugImage.rows - 20), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 255), 2);

    return debugImage;
}

cv::Mat ContourExtractor::convertTo3Channel(const cv::Mat &gray) {
    cv::Mat result;
    if (gray.channels() == 1) {
        cv::cvtColor(gray, result, cv::COLOR_GRAY2BGR);
    } else {
        result = gray.clone();
    }
    return result;
}

// 核心：寻找最大轮廓
std::vector<cv::Point> ContourExtractor::findLargestContour(const cv::Mat &src, int whiteThreshold) {
    if (src.empty()) {
        qDebug() << "错误: 输入图像为空!";
        return {};
    }

    // 2. 检查图像通道数
    cv::Mat srcClone;
    if (src.channels() == 1) {
        // 如果已经是灰度图，直接使用
        srcClone = src.clone();
        // qDebug() << "输入图像已经是灰度图，跳过转换";
    } else if (src.channels() == 3 || src.channels() == 4) {
        // 如果是彩色图，转换为灰度图
        cv::cvtColor(src, srcClone, cv::COLOR_BGR2GRAY);
        // qDebug() << "已将彩色图转换为灰度图";
    } else {
        qDebug() << "错误: 不支持的图像通道数: " << src.channels();
        return {};
    }

    cv::Mat thr;
    // 背光图片：物体黑(0)，背景白(255)。
    // 使用 THRESH_BINARY_INV 将物体变成白色(255)，背景变成黑色(0)
    // 这样 findContours 才能正确找到物体
    cv::threshold(srcClone, thr, whiteThreshold, 255, cv::THRESH_BINARY_INV);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(thr, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if (contours.empty())
        return {};

    // 找到面积最大的轮廓
    double maxArea = 0;
    int maxIdx = -1;
    for (size_t i = 0; i < contours.size(); i++) {
        double area = cv::contourArea(contours[i]);
        if (area > maxArea) {
            maxArea = area;
            maxIdx = i;
        }
    }

    if (maxIdx != -1)
        return contours[maxIdx];
    return {};
}

std::vector<std::vector<cv::Point>> ContourExtractor::findContours(const cv::Mat &inputImage, int whiteThreshold, int areaThreshold) {
    if (inputImage.empty()) {
        // cv::Mat emptyResult(300, 400, CV_8UC3, cv::Scalar(0, 0, 0));
        // cv::putText(emptyResult, "输入图像为空", cv::Point(50, 150), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
        cv::Mat emptyResult;
        return std::vector<std::vector<cv::Point>>();
    }

    // 3. 转换为灰度图
    cv::Mat grayImage;
    if (inputImage.channels() == 3) {
        cv::cvtColor(inputImage, grayImage, cv::COLOR_BGR2GRAY);
    } else {
        grayImage = inputImage.clone();
    }

    // 4. 二值化处理
    cv::Mat binaryImage;
    // cv::adaptiveThreshold(grayImage, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, m_adaptiveBlockSize, 2);
    cv::threshold(grayImage, binaryImage, whiteThreshold, 255, cv::THRESH_BINARY_INV);

    // cv::imshow("binaryImage", binaryImage);

    // 5. 查找轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binaryImage, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 过滤小面积轮廓
    std::vector<std::vector<cv::Point>> filteredContours;
    for (const auto& contour : contours) {
        if (contour.empty() || contour.size() < 3) {
            continue;
        }

        double area = cv::contourArea(contour);
        if (area >= areaThreshold) {  // 只保留面积大于阈值的轮廓
            filteredContours.push_back(contour);
        }
    }

    // 按轮廓面积从大到小排序
    std::sort(filteredContours.begin(), filteredContours.end(),
              [](const std::vector<cv::Point>& contour1, const std::vector<cv::Point>& contour2) {
                  return cv::contourArea(contour1) > cv::contourArea(contour2);
              });

    return filteredContours;
}
