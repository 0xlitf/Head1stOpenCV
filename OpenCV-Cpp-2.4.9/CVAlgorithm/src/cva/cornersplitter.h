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
    std::tuple<cv::Mat, cv::Mat, cv::Mat, cv::Mat> splitCorners(const cv::Mat &inputImage) {
        cv::Mat topLeft, topRight, bottomLeft, bottomRight;

        // 安全检查：输入图像不能为空
        if (inputImage.empty()) {
            qWarning() << "输入图像为空，无法截取角点。";
            return std::make_tuple(topLeft, topRight, bottomLeft, bottomRight);
        }

        int imgHeight = inputImage.rows;
        int imgWidth = inputImage.cols;
        int s = m_cornerSideLength;

        // 安全检查：确保要截取的正方形区域在图像范围内
        if (s > imgHeight || s > imgWidth) {
            qWarning() << "设定的边长超过图像范围。图像尺寸: (" << imgWidth << "x" << imgHeight << "), 请求的边长: " << s;
            // 可以选择将s调整为最大可能值，这里直接返回空矩阵
            // s = std::min(imgHeight, imgWidth);
            return std::make_tuple(topLeft, topRight, bottomLeft, bottomRight);
        }

        // 使用try-catch防止ROI越界导致程序崩溃
        try {
            // 1. 左上角区域：从(0,0)开始，截取sxs的区域
            cv::Rect roiTL(0, 0, s, s);
            topLeft = inputImage(roiTL).clone(); // 使用clone()获取深拷贝，与原图数据独立

            // 2. 右上角区域：从(宽度-s, 0)开始，截取sxs的区域
            cv::Rect roiTR(imgWidth - s, 0, s, s);
            topRight = inputImage(roiTR).clone();

            // 3. 左下角区域：从(0, 高度-s)开始，截取sxs的区域
            cv::Rect roiBL(0, imgHeight - s, s, s);
            bottomLeft = inputImage(roiBL).clone();

            // 4. 右下角区域：从(宽度-s, 高度-s)开始，截取sxs的区域
            cv::Rect roiBR(imgWidth - s, imgHeight - s, s, s);
            bottomRight = inputImage(roiBR).clone();

        } catch (const cv::Exception &e) {
            qWarning() << "在截取角点区域时发生OpenCV异常：" << e.what();
        }

        return std::make_tuple(topLeft, topRight, bottomLeft, bottomRight);
    }

private:
    int m_cornerSideLength{100};
};
#endif // CORNERSPLITTER_H
