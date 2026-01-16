#ifndef MINIMUMBOUNDING_H
#define MINIMUMBOUNDING_H

#include <QObject>
#include <opencv2/opencv.hpp>

class MinimumBounding : public QObject
{
    Q_OBJECT
public:
    explicit MinimumBounding(QObject *parent = nullptr);

    // 修改后的接口：输入和输出均为cv::Mat，底色为纯黑色，短边纵向、长边横向
    cv::Mat findAndCropObject(const cv::Mat& inputImage);

signals:
    // 可以添加处理进度或完成信号
    // void processingFinished(const cv::Mat& result);
};

#endif // MINIMUMBOUNDING_H
