#ifndef HUMOMENTSMATCHER_H
#define HUMOMENTSMATCHER_H

#pragma execution_character_set("utf-8")

#include "fileutils.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QPixmap>
#include <QString>
#include <opencv2/opencv.hpp>

class HuMomentsMatcher {
public:
    HuMomentsMatcher();

    void addTemplateIntoMap(const QString &name, const QString &huStr,
                            std::vector<cv::Point> contour);

    // 辅助函数：将 cv::Mat 转换为 QPixmap 用于显示
    QPixmap cvMatToQPixmap(const cv::Mat &inMat);

    // 辅助函数：获取最大的轮廓
    std::vector<cv::Point> findLargestContour(const cv::Mat &srcInfo,
                                              bool isTemplate);

    cv::Mat addTemplate(const QString &fileName);

    QString calcHuMoments(std::vector<cv::Point> contour);

    cv::Mat croppedCanvas(cv::Mat templateImg, std::vector<cv::Point> contour);

    // C:\GitHub\Head1stOpenCV\OpenCV-Cpp-2.4.9\HuMoments\dataset_foler
    void setTemplateFolder(const QString &folderName);

    void matchImage(const QString &fileName);

    void matchMat(cv::Mat sceneImg);

private:
    QList<std::tuple<QString, QString, std::vector<cv::Point>>> m_humomentsList;
};

#endif // HUMOMENTSMATCHER_H
