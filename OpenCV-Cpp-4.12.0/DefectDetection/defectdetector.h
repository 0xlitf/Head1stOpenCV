#ifndef DEFECTDETECTOR_H
#define DEFECTDETECTOR_H

#include <QObject>
#include <opencv2/opencv.hpp>

class DefectDetector : public QObject
{
    Q_OBJECT

public:
    explicit DefectDetector(QObject *parent = nullptr);

    // 首先调用这个接口，分析图片中有几个物料，多的直接吹掉
    static std::tuple<int, cv::Mat> analyzeAndDrawContour(const cv::Mat& inputImage);

    void setTemplateFolder(const QStringList &descStrs, const QStringList &folderName);

private:
    void addTemplate(const QString &desc, const QString &fileName);

private:
    QList<std::tuple<QString, QString, QString, std::vector<cv::Point>>>
        m_huMomentsList;
};

#endif // DEFECTDETECTOR_H
