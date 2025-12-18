#ifndef HUMOMENTSMATCHER_H
#define HUMOMENTSMATCHER_H

#include <QPixmap>
#include <QString>
#include <QMap>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <opencv2/opencv.hpp>
#include "fileutils.h"

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

    QString calcHuMoments(std::vector<cv::Point> contour);

    cv::Mat croppedCanvas(cv::Mat templateImg, std::vector<cv::Point> contour);

    // C:\GitHub\Head1stOpenCV\OpenCV-Cpp-2.4.9\HuMoments\dataset_foler
    void setTemplateFolder(const QString& folderName) {
        auto imageFilenames = FileUtils::findAllImageFiles(folderName);

    }

private:

    std::vector<cv::Point> m_templateContour; // 存储提取出的模板轮廓

    QMap<QString, QList<cv::Mat>> m_matMap;
};

#endif // HUMOMENTSMATCHER_H
