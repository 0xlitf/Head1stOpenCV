#ifndef MINIMUMBOUNDING_H
#define MINIMUMBOUNDING_H

#include <QObject>
#include <opencv2/opencv.hpp> // 包含OpenCV头文件

class MinimumBounding : public QObject
{
    Q_OBJECT
public:
    explicit MinimumBounding(QObject *parent = nullptr);

    // 修改后的公共接口：输入和输出均为cv::Mat
    cv::Mat findAndCropObject(const cv::Mat& inputImage);

signals:
    // 可以添加一个信号，用于在处理完成时通知其他对象
    // void processingFinished(const cv::Mat& result);
};

#endif // MINIMUMBOUNDING_H
