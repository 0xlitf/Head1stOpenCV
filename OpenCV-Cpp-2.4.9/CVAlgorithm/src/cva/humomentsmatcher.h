#ifndef HUMOMENTSMATCHER_H
#define HUMOMENTSMATCHER_H

#pragma execution_character_set("utf-8")

#include "cva_global.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <opencv2/opencv.hpp>

using MatchResult =
    std::tuple<QString, std::vector<cv::Point>, cv::Point2f, double, double>;

class CVASHARED_EXPORT HuMomentsMatcher : public QObject {
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

// 外部调用的static接口
public:
    static std::tuple<int, cv::Mat> analyzeAndDrawContour(const cv::Mat& inputImage);

    static cv::Mat drawResultsOnImage(const cv::Mat &inputImage,
                                      const QList<MatchResult> &resultList);

    static void drawLabel(cv::Mat &image, const QString &label,
                          const cv::Point &position, const cv::Scalar &color);

// 外部调用的算法接口
public:
    HuMomentsMatcher(QObject *parent = nullptr);

    // C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/HuMoments/dataset_folder
    // 设置模版文件夹，传入描述的QStringList和文件夹路径的QStringList
    void setTemplateFolder(const QStringList &descStrs, const QStringList &folderName);

    int getTemplateSize() {
        return m_huMomentsList.size();
    }

    QList<MatchResult> matchImage(const QString &fileName);

    QList<MatchResult> matchMat(cv::Mat sceneImg);

    // 辅助函数：获取最大的轮廓
    std::vector<cv::Point> findLargestContour(const cv::Mat &srcInfo,
                                              bool isTemplate);

public:
    void addTemplateIntoMap(const QString &desc, const QString &fileName,
                            const QString &huStr, std::vector<cv::Point> contour);

    // 辅助函数：将 cv::Mat 转换为 QPixmap 用于显示
    QPixmap cvMatToQPixmap(const cv::Mat &inMat);

    QString calcHuMoments(std::vector<cv::Point> contour);

    cv::Mat croppedCanvas(cv::Mat templateImg, std::vector<cv::Point> contour);

    double scoreThreshold() const;
    void setScoreThreshold(double newScoreThreshold);

    int whiteThreshold() const;
    void setWhiteThreshold(int thres);

    double areaThreshold() const;
    void setAreaThreshold(double newAreaThreshold);

private:
    void addTemplate(const QString &desc, const QString &fileName);

private:
    QList<std::tuple<QString, QString, QString, std::vector<cv::Point>>>
        m_huMomentsList;
    double m_scoreThreshold{0.1};
    int m_whiteThreshold{240};

    double m_areaThreshold{0.2};

};

#endif // HUMOMENTSMATCHER_H
