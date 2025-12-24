#include "cutoutobject.h"

CutOutObject::CutOutObject() {}
bool CutOutObject::extractLargestContour(const cv::Mat &inputImage,
                                         std::vector<cv::Point> &contour,
                                         double &area, cv::RotatedRect &minRect,
                                         int colorThreshold, int blueThreshold,
                                         int kernelSize) {

    if (inputImage.empty()) {
        qWarning() << "错误, 输入图像为空";
        return false;
    }

    cv::Mat cvImage = inputImage.clone();
    for (int i = 0; i < cvImage.rows; ++i) {
        for (int j = 0; j < cvImage.cols; ++j) {
            cv::Vec3b &pixel = cvImage.at<cv::Vec3b>(i, j);
            if ((pixel[0] - pixel[1] > colorThreshold) &&
                (pixel[0] - pixel[2] > colorThreshold) &&
                (pixel[0] > blueThreshold)) {
                pixel[0] = 255;
                pixel[1] = 255;
                pixel[2] = 255;
            } else {
                pixel[0] = 0;
                pixel[1] = 0;
                pixel[2] = 0;
            }
        }
    }

    cv::bitwise_not(cvImage, cvImage);

    // cv::imshow("threshold (30,50)", cvImage);

    cv::Mat gray;
    cv::cvtColor(cvImage, gray, cv::COLOR_BGR2GRAY);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE,
                                               cv::Size(kernelSize, kernelSize));

    cv::Mat morphResult;
    cv::morphologyEx(gray, morphResult, cv::MORPH_OPEN, kernel);

    cv::morphologyEx(morphResult, morphResult, cv::MORPH_CLOSE, kernel);

    cv::dilate(morphResult, morphResult, kernel, cv::Point(-1, -1), 2);

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(morphResult, contours, hierarchy, cv::RETR_EXTERNAL,
                     cv::CHAIN_APPROX_SIMPLE);

    if (contours.empty()) {
        qDebug() << "未找到任何轮廓！";
        return false;
    }

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

    if (contour.size() >= 5) {
        minRect = cv::minAreaRect(contour);
    } else {
        minRect = cv::RotatedRect(cv::minAreaRect(contour));
    }

    return true;
}
cv::Mat CutOutObject::getObjectInBoundingRect(const cv::Mat &inputImage,
                                              int colorThreshold,
                                              int blueThreshold,
                                              int kernelSize) {
    std::vector<cv::Point> contour;
    double area;
    cv::RotatedRect minRect;

    if (!extractLargestContour(inputImage, contour, area, minRect, colorThreshold,
                               blueThreshold, kernelSize)) {
        qWarning() << "无法提取轮廓，返回空矩阵";
        return cv::Mat();
    }

    cv::Rect boundingRect = cv::boundingRect(contour);

    cv::Mat result(boundingRect.height, boundingRect.width, CV_8UC3,
                   cv::Scalar(255, 255, 255));

    std::vector<cv::Point> shiftedContour;
    for (const auto &point : contour) {
        shiftedContour.push_back(
            cv::Point(point.x - boundingRect.x, point.y - boundingRect.y));
    }

    if (!shiftedContour.empty()) {
        std::vector<std::vector<cv::Point>> contoursToDraw = {shiftedContour};
        cv::drawContours(result, contoursToDraw, 0, cv::Scalar(0, 0, 0),
                         CV_FILLED); // 注意：OpenCV 2.4.9中使用CV_FILLED
    }

    return result;
}

cv::Mat CutOutObject::getObjectInOriginalSize(const cv::Mat &inputImage,
                                              int colorThreshold,
                                              int blueThreshold,
                                              int kernelSize) {
    std::vector<cv::Point> contour;
    double area;
    cv::RotatedRect minRect;

    if (!extractLargestContour(inputImage, contour, area, minRect, colorThreshold,
                               blueThreshold, kernelSize)) {
        qWarning() << "无法提取轮廓，返回空矩阵";
        return cv::Mat();
    }

    cv::Mat result(inputImage.size(), CV_8UC3, cv::Scalar(255, 255, 255));

    if (!contour.empty()) {
        std::vector<std::vector<cv::Point>> contoursToDraw = {contour};
        cv::drawContours(result, contoursToDraw, 0, cv::Scalar(0, 0, 0),
                         CV_FILLED); // 注意：OpenCV 2.4.9中使用CV_FILLED
    }

    return result;
}

void CutOutObject::testExtractLargestContour(const QString &imageFilename) {
    cv::Mat image = cv::imread(imageFilename.toStdString());
    if (image.empty()) {
        qDebug() << "无法读取图像";
        return;
    }

    std::vector<cv::Point> largestContour;
    double contourArea;
    cv::RotatedRect minRect;

    if (extractLargestContour(image, largestContour, contourArea, minRect, 30, 50,
                              3)) {
        qDebug() << "找到最大轮廓，面积: " << contourArea << " 像素";

        qDebug() << "最小外接矩形信息:";
        qDebug() << "中心点: (" << minRect.center.x << ", " << minRect.center.y
                 << ")";
        qDebug() << "尺寸: " << minRect.size.width << " x " << minRect.size.height;
        qDebug() << "旋转角度: " << minRect.angle << " 度";

        cv::Mat result = image.clone();

        cv::drawContours(result,
                         std::vector<std::vector<cv::Point>>{largestContour}, -1,
                         cv::Scalar(0, 255, 0), 3);

        cv::Point2f rectPoints[4];
        minRect.points(rectPoints);
        for (int i = 0; i < 4; i++) {
            cv::line(result, rectPoints[i], rectPoints[(i + 1) % 4],
                     cv::Scalar(0, 0, 255), 2);
        }

        cv::circle(result, minRect.center, 5, cv::Scalar(255, 0, 0), -1);

        cv::imshow("Original Image", image);
        cv::imshow("Contour with Min Area Rect", result);
        cv::waitKey(0);
    } else {
        qDebug() << "未找到有效轮廓！";
    }
}
