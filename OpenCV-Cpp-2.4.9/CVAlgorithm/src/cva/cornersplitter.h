#ifndef CORNERSPLITTER_H
#define CORNERSPLITTER_H

#include <QDebug>
#include <QObject>
#include <opencv2/opencv.hpp>
#include <tuple>

class CornerSplitter : public QObject {
    Q_OBJECT

public:
    explicit CornerSplitter(QObject *parent = nullptr);

    // 设置角点正方形边长
    void setCornerSideLength(int length) {
        // 确保边长不超过图像高度的一半，这是一个合理的约束
        m_cornerSideLength = std::max(1, length); // 最小为1像素
    }

    // 获取角点正方形边长
    int cornerSideLength() const { return m_cornerSideLength; }

    std::tuple<cv::Mat, cv::Mat, cv::Mat, cv::Mat> splitCorners(const cv::Mat &inputImage);

private:
    int m_cornerSideLength{100};
};
#endif // CORNERSPLITTER_H
