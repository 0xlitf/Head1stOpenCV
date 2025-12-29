#include "cutoutobject.h"

#include <QElapsedTimer>

CutOutObject::CutOutObject() {}

cv::Mat CutOutObject::eraseBlueBackground(cv::Mat inputImage,
                                          int colorThreshold,
                                          int blueThreshold) {
    // 图像预处理（与原有逻辑相同）
    cv::Mat cvImage = inputImage.clone();
    int rows = cvImage.rows;
    int cols = cvImage.cols;

    if (auto useBatch = true) {
        for (int i = 0; i < rows; ++i) {
            // 获取第i行的行首指针
            cv::Vec3b *ptr = cvImage.ptr<cv::Vec3b>(i);
            for (int j = 0; j < cols; ++j) {
                cv::Vec3b &pixel = ptr[j]; // 通过指针快速访问像素
                uchar blue = pixel[0];
                uchar green = pixel[1];
                uchar red = pixel[2];

                if ((blue - green > colorThreshold) && (blue - red > colorThreshold) &&
                    (blue > blueThreshold)) {
                    pixel = cv::Vec3b(255, 255, 255);
                } else {
                    pixel = cv::Vec3b(0, 0, 0);
                }
            }
        }
    } else {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                cv::Vec3b &pixel = cvImage.at<cv::Vec3b>(i, j);
                if ((pixel[0] - pixel[1] > colorThreshold) &&
                    (pixel[0] - pixel[2] > colorThreshold) &&
                    (pixel[0] > blueThreshold)) {
                    pixel[0] = 0;
                    pixel[1] = 0;
                    pixel[2] = 0;
                } else {
                    pixel[0] = 255;
                    pixel[1] = 255;
                    pixel[2] = 255;
                }
            }
        }
    }

    return cvImage;
}

// 新增：提取多个物体的核心函数
std::vector<ObjectDetectionResult> CutOutObject::extractMultipleObjects(
    const cv::Mat& inputImage,
    double minAreaThreshold,
    double maxAreaThreshold) {

    std::vector<ObjectDetectionResult> results;

    if (inputImage.empty()) {
        qWarning() << "错误, 输入图像为空";
        return results;
    }

    // cv::bitwise_not(cvImage, cvImage);

    // cv::imshow("cvImage after fillWhiteBorder", cvImage);

    cv::Mat gray;
    cv::cvtColor(inputImage, gray, cv::COLOR_BGR2GRAY);
    cv::imshow("gray", gray);

    // cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE,
    //                                            cv::Size(kernelSize, kernelSize));

    // cv::Mat morphResult;
    // cv::morphologyEx(gray, morphResult, cv::MORPH_OPEN, kernel);
    // cv::morphologyEx(morphResult, morphResult, cv::MORPH_CLOSE, kernel);
    // cv::dilate(morphResult, morphResult, kernel, cv::Point(-1, -1), 2);

    // 查找所有轮廓
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;

    // 白底黑物需要转化为黑底白物
    cv::bitwise_not(gray, gray);

    cv::findContours(gray, contours, hierarchy, cv::RETR_EXTERNAL,
                     cv::CHAIN_APPROX_SIMPLE);

    if (contours.empty()) {
        qDebug() << "未找到任何轮廓！";
        return results;
    } else {
        qDebug() << "轮廓数量: " << contours.size();
    }

    // 对轮廓按面积进行排序[8](@ref)
    std::sort(contours.begin(), contours.end(),
              [](const std::vector<cv::Point>& a, const std::vector<cv::Point>& b) {
                  return cv::contourArea(a) > cv::contourArea(b);
              });

    // 筛选符合面积阈值的轮廓[6,7](@ref)
    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);
        qDebug() << "contourArea" << area;

        // 应用面积阈值过滤
        if (area >= minAreaThreshold && area <= maxAreaThreshold) {
            ObjectDetectionResult result;
            result.contour = contour;
            result.area = area;
            result.boundingRect = cv::boundingRect(contour);

            if (contour.size() >= 5) {
                result.minRect = cv::minAreaRect(contour);
            } else {
                result.minRect = cv::RotatedRect();
            }

            results.push_back(result);
        }
    }

    qDebug() << "找到" << results.size() << "个符合面积阈值的物体";
    return results;
}

// 保持原有的单个物体检测函数（向后兼容）
bool CutOutObject::extractLargestContour(const cv::Mat& inputImage,
                                         std::vector<cv::Point>& contour,
                                         double& area, cv::RotatedRect& minRect,
                                         int colorThreshold, int blueThreshold,
                                         int kernelSize) {

    auto results = extractMultipleObjects(inputImage, 0, 1000000);

    if (results.empty()) {
        return false;
    }

    // 返回面积最大的物体
    contour = results[0].contour;
    area = results[0].area;
    minRect = results[0].minRect;

    return true;
}

// 新增：获取多个物体的边界框结果
std::vector<cv::Mat> CutOutObject::getMultipleObjectsInBoundingRect(
    const cv::Mat& inputImage,
    double minAreaThreshold,
    double maxAreaThreshold) {

    // cv::imshow("inputImage", inputImage);

    std::vector<cv::Mat> resultImages;

    QElapsedTimer timer;
    timer.start();

    auto results =
        extractMultipleObjects(inputImage, minAreaThreshold, maxAreaThreshold);

    qDebug() << "extractMultipleObjects elapsed:" << timer.elapsed();

    for (const auto& result : results) {
        cv::Rect boundingRect = result.boundingRect;
        cv::Mat resultImg(boundingRect.height, boundingRect.width, CV_8UC3,
                          cv::Scalar(255, 255, 255));

        std::vector<cv::Point> shiftedContour;
        for (const auto& point : result.contour) {
            shiftedContour.push_back(
                cv::Point(point.x - boundingRect.x, point.y - boundingRect.y));
        }

        if (!shiftedContour.empty()) {
            std::vector<std::vector<cv::Point>> contoursToDraw = {shiftedContour};
            cv::drawContours(resultImg, contoursToDraw, 0, cv::Scalar(0, 0, 0),
                             CV_FILLED);
        }

        resultImages.push_back(resultImg);
    }

    return resultImages;
}

// 新增：获取多个物体的原图尺寸掩码
cv::Mat CutOutObject::getMultipleObjectsInOriginalSize(
    const cv::Mat& inputImage,
    double minAreaThreshold,
    double maxAreaThreshold) {

    auto results =
        extractMultipleObjects(inputImage, minAreaThreshold, maxAreaThreshold);

    cv::Mat resultImg(inputImage.size(), CV_8UC3, cv::Scalar(255, 255, 255));

    for (const auto &result : results) {
        if (!result.contour.empty()) {
            std::vector<std::vector<cv::Point>> contoursToDraw = {result.contour};
            cv::drawContours(resultImg, contoursToDraw, 0, cv::Scalar(0, 0, 0),
                             CV_FILLED);
        }
    }

    return resultImg;
}

// 新增：测试多物体检测功能
void CutOutObject::testExtractMultipleObjects(const cv::Mat& inputImage,
                                              double minAreaThreshold,
                                              double maxAreaThreshold) {
    auto results = extractMultipleObjects(inputImage, minAreaThreshold, maxAreaThreshold);

    if (results.empty()) {
        qDebug() << "未找到符合面积阈值的物体！";
        return;
    }

    cv::Mat resultImage = inputImage.clone();

    // 用不同颜色绘制每个检测到的物体
    std::vector<cv::Scalar> colors = {
        cv::Scalar(0, 255, 0),   // 绿色
        cv::Scalar(0, 0, 255),   // 红色
        cv::Scalar(255, 0, 0),   // 蓝色
        cv::Scalar(0, 255, 255), // 黄色
        cv::Scalar(255, 0, 255), // 粉色
        cv::Scalar(255, 255, 0)  // 青色
    };

    for (size_t i = 0; i < results.size(); ++i) {
        const auto& result = results[i];
        cv::Scalar color = colors[i % colors.size()];

        // 绘制轮廓
        std::vector<std::vector<cv::Point>> contoursToDraw = {result.contour};
        cv::drawContours(resultImage, contoursToDraw, 0, color, 3);

        // 绘制最小外接矩形
        cv::Point2f rectPoints[4];
        result.minRect.points(rectPoints);
        for (int j = 0; j < 4; j++) {
            cv::line(resultImage, rectPoints[j], rectPoints[(j + 1) % 4],
                     color, 2);
        }

        // 标注面积信息
        std::string areaText = "Area: " + std::to_string((int)result.area);
        cv::putText(resultImage, areaText,
                    cv::Point(result.boundingRect.x, result.boundingRect.y - 10),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, color, 2);

        qDebug() << "物体" << i + 1 << ": 面积=" << result.area
                 << ", 中心点=(" << result.minRect.center.x
                 << "," << result.minRect.center.y << ")";
    }

    cv::imshow("Multiple Objects Detection", resultImage);
}
