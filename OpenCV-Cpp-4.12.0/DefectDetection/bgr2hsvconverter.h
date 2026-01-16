#ifndef BGR2HSVCONVERTER_H
#define BGR2HSVCONVERTER_H

#include <opencv2/opencv.hpp>
#include <QObject>
#include <QDebug>

class BGR2HSVConverter : public QObject {
    Q_OBJECT
public:
    explicit BGR2HSVConverter(QObject *parent = nullptr);

public:
    // 基本BGR到HSV转换
    static cv::Mat convertBGR2HSV(const cv::Mat& bgrImage) {
        cv::Mat hsvImage;
        cv::cvtColor(bgrImage, hsvImage, cv::COLOR_BGR2HSV);
        return hsvImage;
    }

    // 反向转换：HSV到BGR
    static cv::Mat convertHSV2BGR(const cv::Mat& hsvImage) {
        cv::Mat bgrImage;
        cv::cvtColor(hsvImage, bgrImage, cv::COLOR_HSV2BGR);
        return bgrImage;
    }

    // 转换单个像素值
    static cv::Vec3b convertPixel(const cv::Vec3b& bgrPixel) {
        cv::Mat pixelMat(1, 1, CV_8UC3, bgrPixel);
        cv::Mat hsvMat;
        cv::cvtColor(pixelMat, hsvMat, cv::COLOR_BGR2HSV);
        return hsvMat.at<cv::Vec3b>(0, 0);
    }


    void printHueChannelDetails(const cv::Mat& hsvImage) {
        if (hsvImage.empty()) {
            qDebug() << "错误：HSV图像为空！";
            return;
        }

        if (hsvImage.channels() != 3) {
            qDebug() << "错误：输入图像必须是3通道HSV格式";
            return;
        }

        // 分离HSV通道[6,7](@ref)
        std::vector<cv::Mat> hsvChannels;
        cv::split(hsvImage, hsvChannels);
        cv::Mat hueChannel = hsvChannels[0]; // H通道

        qDebug() << "=== HSV图像色相通道详细信息 ===";
        qDebug() << "图像尺寸:" << hueChannel.rows << "x" << hueChannel.cols;
        qDebug() << "总像素数:" << hueChannel.rows * hueChannel.cols;

        // 计算基本统计信息[3](@ref)
        double minVal, maxVal;
        cv::Point minLoc, maxLoc;
        cv::minMaxLoc(hueChannel, &minVal, &maxVal, &minLoc, &maxLoc);

        qDebug() << "色相统计信息:";
        qDebug() << "  最小值:" << minVal << "度 (位置: [" << minLoc.y << "," << minLoc.x << "])";
        qDebug() << "  最大值:" << maxVal << "度 (位置: [" << maxLoc.y << "," << maxLoc.x << "])";

        // 计算平均值和标准差
        cv::Scalar mean, stddev;
        cv::meanStdDev(hueChannel, mean, stddev);
        qDebug() << "  平均值:" << mean[0] << "度";
        qDebug() << "  标准差:" << stddev[0] << "度";

        // 计算色相值分布直方图[1](@ref)
        qDebug() << "色相值分布 (按颜色范围分类):";

        // 定义颜色范围 (OpenCV中H范围是0-179)[4](@ref)
        QVector<QPair<QString, QPair<int, int>>> colorRanges = {
            {"红色1", {0, 10}}, {"红色2", {170, 179}},
            {"橙色", {11, 25}}, {"黄色", {26, 35}},
            {"绿色", {36, 85}}, {"蓝色", {86, 135}},
            {"紫色", {136, 169}}
        };

        int totalPixels = hueChannel.rows * hueChannel.cols;

        for (const auto& color : colorRanges) {
            cv::Mat mask;
            int lower = color.second.first;
            int upper = color.second.second;

            if (lower < upper) {
                cv::inRange(hueChannel, lower, upper, mask);
            } else {
                // 处理红色分布在两端的情况[2](@ref)
                cv::Mat mask1, mask2;
                cv::inRange(hueChannel, lower, 179, mask1);
                cv::inRange(hueChannel, 0, upper, mask2);
                mask = mask1 | mask2;
            }

            int pixelCount = cv::countNonZero(mask);
            double percentage = (static_cast<double>(pixelCount) / totalPixels) * 100;

            qDebug() << "  " << color.first << "(" << lower << "-" << upper << "度):"
                     << pixelCount << "像素 (" << QString::number(percentage, 'f', 2) << "%)";
        }
        qDebug() << "";

        // 打印关键区域的色相值[3](@ref)
        qDebug() << "关键区域色相值:";
        QVector<QPair<cv::Point, QString>> regions = {
            {cv::Point(0, 0), "左上角"},
            {cv::Point(hueChannel.cols-1, 0), "右上角"},
            {cv::Point(0, hueChannel.rows-1), "左下角"},
            {cv::Point(hueChannel.cols-1, hueChannel.rows-1), "右下角"},
            {cv::Point(hueChannel.cols/2, hueChannel.rows/2), "中心点"}
        };

        for (const auto& region : regions) {
            if (region.first.x >= 0 && region.first.x < hueChannel.cols &&
                region.first.y >= 0 && region.first.y < hueChannel.rows) {
                uchar hueValue = hueChannel.at<uchar>(region.first);
                qDebug() << "  " << region.second << "[" << region.first.x << ","
                         << region.first.y << "]: " << hueValue << "度";
            }
        }
        qDebug() << "";

        // 打印前10x10区域的像素值样本[3](@ref)
        qDebug() << "色相通道像素值样本 (前10x10区域):";
        int sampleRows = qMin(10, hueChannel.rows);
        int sampleCols = qMin(10, hueChannel.cols);

        for (int y = 0; y < sampleRows; y++) {
            QString rowStr = "  行" + QString::number(y) + ": ";
            for (int x = 0; x < sampleCols; x++) {
                uchar hueValue = hueChannel.at<uchar>(y, x);
                rowStr += QString::number(hueValue).rightJustified(3, ' ') + " ";
            }
            qDebug() << rowStr;
        }
    }
};

#endif // BGR2HSVCONVERTER_H
