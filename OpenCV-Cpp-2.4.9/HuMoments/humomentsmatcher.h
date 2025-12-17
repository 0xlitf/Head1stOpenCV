#ifndef HUMOMENTSMATCHER_H
#define HUMOMENTSMATCHER_H

#include <QPixmap>
#include <QString>
#include <QMap>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <opencv2/opencv.hpp>

class HuMomentsMatcher
{
public:
    HuMomentsMatcher();
    void addTemplateIntoMap(const QString& name, cv::Mat mat);

    // 辅助函数：将 cv::Mat 转换为 QPixmap 用于显示
    QPixmap cvMatToQPixmap(const cv::Mat &inMat);

    // 辅助函数：获取最大的轮廓
    std::vector<cv::Point> findLargestContour(const cv::Mat &srcInfo,
                                              bool isTemplate);

    cv::Mat addTemplate(const QString& fileName);

    QString calcHuMoments(std::vector<cv::Point> contour) {
        cv::Moments moms = cv::moments(contour);
        double hu[7];
        cv::HuMoments(moms, hu);

        QString huStr;
        for (int i = 0; i < 7; i++) {
            double value = hu[i];
            double result;
            // 使用 Log 变换方便查看数量级
            // 取Hu矩的绝对值
            // 以10为底的对数
            // 保存原始hu[i]的符号
            if (fabs(value) < 1e-20) {  // 接近0的处理
                result = 0.0;
            } else {
                result = -1 * copysign(1.0, value) * log10(fabs(value));
            }
            // huStr += QString::number(-1 * copysign(1.0, hu[i]) * log10(abs(hu[i])),
            //                          'f', 2) +
            //          " ";
            huStr += QString::number(result, 'f', 2) + " ";
        }
        return huStr;
    }

    cv::Mat croppedCanvas(cv::Mat templateImg, std::vector<cv::Point> contour) {
        cv::Mat canvas;
        if (templateImg.channels() == 1) {
            cv::cvtColor(templateImg, canvas, cv::COLOR_GRAY2BGR);
        } else {
            canvas = templateImg.clone();  // 深拷贝
        }

        std::vector<std::vector<cv::Point>> contoursToDraw = { contour };
        cv::drawContours(canvas, contoursToDraw, 0, cv::Scalar(0, 255, 0), 2); // 绿色线条

        // 3. 计算轮廓的包围盒 (Bounding Rect)
        cv::Rect boundRect = cv::boundingRect(contour);

        // 4. 增加一点 padding (边距)，防止轮廓紧贴着边缘不好看
        int padding = 10;
        boundRect.x = std::max(0, boundRect.x - padding);
        boundRect.y = std::max(0, boundRect.y - padding);
        boundRect.width = std::min(canvas.cols - boundRect.x, boundRect.width + 2 * padding);
        boundRect.height = std::min(canvas.rows - boundRect.y, boundRect.height + 2 * padding);

        // 5. 裁剪图像 (ROI - Region of Interest)
        cv::Mat croppedCanvas = canvas(boundRect);

        return croppedCanvas;
    }

private:

    std::vector<cv::Point> m_templateContour; // 存储提取出的模板轮廓

    QMap<QString, QList<cv::Mat>> m_matMap;
};

#endif // HUMOMENTSMATCHER_H
