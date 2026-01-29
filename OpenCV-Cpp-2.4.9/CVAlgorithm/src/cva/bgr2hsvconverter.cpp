#include "bgr2hsvconverter.h"

BGR2HSVConverter::BGR2HSVConverter(QObject *parent)
    : QObject{parent} {}

cv::Mat BGR2HSVConverter::convertBGR2HSV(const cv::Mat &bgrImage) {
    cv::Mat hsvImage;
    cv::cvtColor(bgrImage, hsvImage, cv::COLOR_BGR2HSV);
    return hsvImage;
}

cv::Mat BGR2HSVConverter::convertHSV2BGR(const cv::Mat &hsvImage) {
    cv::Mat bgrImage;
    cv::cvtColor(hsvImage, bgrImage, cv::COLOR_HSV2BGR);
    return bgrImage;
}
