#ifndef DEFECTDETECTOR_H
#define DEFECTDETECTOR_H

#pragma execution_character_set("utf-8")

#include <QObject>
#include <QDebug>
#include <QMessageBox>
#include <opencv2/opencv.hpp>

class DefectDetector : public QObject
{
    Q_OBJECT

public:
    enum ErrorCode {
        SUCCESS = 0,           // 成功
        FILE_NOT_FOUND = 1,    // 文件不存在
        IMAGE_LOAD_FAILED = 2, // 图像加载失败
        INVALID_IMAGE = 3,     // 无效图像
        NO_CONTOURS_FOUND = 4, // 未找到轮廓
        TEMPLATE_NOT_SET = 5,  // 模板未设置
        MATCH_FAILED = 6       // 匹配失败
    };
    Q_ENUM(ErrorCode) // 启用Qt元对象系统

signals:
    void sendLog(const QString &logStr);
    void errorOccured(ErrorCode errorCode, const QString &errorStr);

public:
    explicit DefectDetector(QObject *parent = nullptr);

    static std::vector<std::vector<cv::Point>> findContours(const cv::Mat& inputImage, int whiteThreshold = 240, int areaThreshold = 2000);
    // 创建外边缘掩膜（向内6个像素的环形区域）
    cv::Mat createOuterEdgeMask(const cv::Mat& inputImage,
                                const std::vector<std::vector<cv::Point>>& contours,
                                int edgeWidth = 6) {

        if (contours.empty()) {
            return cv::Mat::zeros(inputImage.size(), CV_8UC1);
        }

        cv::Mat mask = cv::Mat::zeros(inputImage.size(), CV_8UC1);
        cv::Mat edgeMask = cv::Mat::zeros(inputImage.size(), CV_8UC1);

        // 1. 绘制原始轮廓（填充）
        cv::drawContours(mask, contours, -1, cv::Scalar(255), CV_FILLED);

        // 2. 创建腐蚀后的内部区域
        cv::Mat innerMask = mask.clone();
        if (edgeWidth > 0) {
            cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE,
                                                       cv::Size(2*edgeWidth + 1,
                                                                2*edgeWidth + 1));
            cv::erode(mask, innerMask, kernel);
        }

        // 3. 外边缘掩膜 = 原始掩膜 - 内部掩膜（环形区域）
        cv::subtract(mask, innerMask, edgeMask);

        return edgeMask;
    }

    // 显示外边缘掩膜
    cv::Mat processOuterEdge(const cv::Mat& inputImage,
                          const std::vector<std::vector<cv::Point>>& contours,
                          int edgeWidth = 6) {

        if (inputImage.empty() || contours.empty()) {
            return cv::Mat();
        }

        // 克隆原图用于显示
        cv::Mat displayMat = inputImage.clone();
        if (displayMat.channels() == 1) {
            cv::cvtColor(displayMat, displayMat, cv::COLOR_GRAY2BGR);
        }

        // 创建外边缘掩膜
        cv::Mat edgeMask = createOuterEdgeMask(inputImage, contours, edgeWidth);
        cv::imshow("edgeMask", edgeMask);

        // 创建彩色掩膜（红色边缘）
        cv::Mat colorEdge;
        cv::cvtColor(edgeMask, colorEdge, cv::COLOR_GRAY2BGR);
        colorEdge.setTo(cv::Scalar(0, 0, 0), edgeMask);  // BGR: 红色

        // 可选：创建绿色原始轮廓线
        cv::Mat contourImage = inputImage.clone();
        if (contourImage.channels() == 1) {
            cv::cvtColor(contourImage, contourImage, cv::COLOR_GRAY2BGR);
        }
        cv::drawContours(contourImage, contours, -1, cv::Scalar(0, 255, 0), 2);

        // 将边缘掩膜叠加到原图
        // cv::addWeighted(displayMat, 0.5, colorEdge, 0.5, 0, displayMat);

        cv::Mat edgeMask_8u;
        edgeMask.convertTo(edgeMask_8u, CV_8UC1);

        // cv::Mat whiteBackground = cv::Mat::ones(displayMat.size(), displayMat.type()) * 255;
        cv::Mat whiteBackground = cv::Mat(displayMat.size(), displayMat.type(), cv::Scalar(255, 255, 255));
        cv::Mat result;
        displayMat.copyTo(result, edgeMask); // 将displayMat中边缘掩膜对应的区域复制到result
        whiteBackground.copyTo(result, ~edgeMask); // 将非边缘区域设置为白色

        return result;
    }

    // 显示外边缘掩膜
    cv::Mat displayOuterEdge(const cv::Mat& inputImage,
                          const std::vector<std::vector<cv::Point>>& contours,
                          int edgeWidth = 6) {

        if (inputImage.empty() || contours.empty()) {
            return cv::Mat();
        }

        // 克隆原图用于显示
        cv::Mat displayMat = inputImage.clone();
        if (displayMat.channels() == 1) {
            cv::cvtColor(displayMat, displayMat, cv::COLOR_GRAY2BGR);
        }

        // 创建外边缘掩膜
        cv::Mat edgeMask = createOuterEdgeMask(inputImage, contours, edgeWidth);
        cv::imshow("edgeMask", edgeMask);

        // 创建彩色掩膜（红色边缘）
        cv::Mat colorEdge;
        cv::cvtColor(edgeMask, colorEdge, cv::COLOR_GRAY2BGR);
        colorEdge.setTo(cv::Scalar(0, 0, 0), edgeMask);  // BGR: 红色

        // 可选：创建绿色原始轮廓线
        cv::Mat contourImage = inputImage.clone();
        if (contourImage.channels() == 1) {
            cv::cvtColor(contourImage, contourImage, cv::COLOR_GRAY2BGR);
        }
        cv::drawContours(contourImage, contours, -1, cv::Scalar(0, 255, 0), 2);

        // 将边缘掩膜叠加到原图
        // cv::addWeighted(displayMat, 0.5, colorEdge, 0.5, 0, displayMat);

        cv::Mat edgeMask_8u;
        edgeMask.convertTo(edgeMask_8u, CV_8UC1);

        // cv::Mat whiteBackground = cv::Mat::ones(displayMat.size(), displayMat.type()) * 255;
        cv::Mat whiteBackground = cv::Mat(displayMat.size(), displayMat.type(), cv::Scalar(255, 255, 255));
        cv::Mat result;
        displayMat.copyTo(result, edgeMask); // 将displayMat中边缘掩膜对应的区域复制到result
        whiteBackground.copyTo(result, ~edgeMask); // 将非边缘区域设置为白色

        cv::imshow("Result", result);


        // 显示对比图
        cv::Mat comparison;
        qDebug() << "contourImage" << contourImage.cols << contourImage.rows;
        qDebug() << "displayMat" << displayMat.cols << displayMat.rows;
        cv::hconcat(contourImage, result, comparison);

        // 添加文字说明
        std::string edgeText = "Edge Width: " + std::to_string(edgeWidth) + " pixels";
        cv::putText(comparison, "Original Contour (Green)",
                    cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX,
                    0.7, cv::Scalar(0, 255, 0), 2);
        cv::putText(comparison, "Edge Zone (Red)",
                    cv::Point(contourImage.cols + 10, 30),
                    cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
        cv::putText(comparison, edgeText,
                    cv::Point(10, comparison.rows - 20),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 0), 2);

        cv::imshow("Outer Edge Detection Zone", comparison);

        return result;
    }

    // 首先调用这个接口，分析图片中有几个物料，多的直接吹掉
    static std::tuple<int, cv::Mat> analyzeAndDrawContour(const cv::Mat& inputImage, int whiteThreshold = 240, int areaThreshold = 2000);

    void setTemplateFolder(const QStringList &descStrs, const QStringList &folderName);

    double fullMatchImage(const QString &fileName);

    double fullMatchMat(cv::Mat sceneImg);

    double scoreThreshold() const;
    void setScoreThreshold(double newScoreThreshold);

    int whiteThreshold() const;
    void setWhiteThreshold(int newWhiteThreshold);

    int detectThickness() const;
    void setDetectThickness(int newDetectThickness);

    int removeOuterBorderThickness() const;
    void setRemoveOuterBorderThickness(int newRemoveOuterBorderThickness);

    bool hasDefect(double scoreThreshold){
        return scoreThreshold > m_scoreThreshold;
    }

    int precision() const;
    void setPrecision(int newPrecision);

    bool debugImageFlag() const;
    void setDebugImageFlag(bool newDebugImageFlag);

    double matchMat(cv::Mat templateInput, cv::Mat defectInput);

private:
    void addTemplate(const QString &desc, const QString &fileName);

    void addTemplateIntoMap(const QString &desc, const QString &fileName, cv::Mat tInput);

private:
    QList<std::tuple<QString, QString, cv::Mat>> m_templateList;
    int m_removeOuterBorderThickness{3}; // 比对时忽略的边缘厚度
    int m_detectThickness{6}; // 比对时检测的边缘厚度
    int m_whiteThreshold{35}; // 差值结果阈值，大于这个值被认为是缺陷点，一般设置为30-40
    double m_scoreThreshold{15}; // 缺陷点的个数，根据下采样的次数决定，m_precision为2时，此数值一般为10-20

    // 以下参数不改
    int m_precision{2}; // 取决于进行几次下采样，暂时不可更改
    bool m_useHSV{false}; // false true

    bool m_debugImageFlag{true}; // false true 是否输出调试结果图片
};

#endif // DEFECTDETECTOR_H
