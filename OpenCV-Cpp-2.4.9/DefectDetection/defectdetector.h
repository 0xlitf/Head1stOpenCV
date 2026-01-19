#ifndef DEFECTDETECTOR_H
#define DEFECTDETECTOR_H

#pragma execution_character_set("utf-8")

#include <QObject>
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

    // 首先调用这个接口，分析图片中有几个物料，多的直接吹掉
    static std::tuple<int, cv::Mat> analyzeAndDrawContour(const cv::Mat& inputImage);

    void setTemplateFolder(const QStringList &descStrs, const QStringList &folderName);

    double fullMatchImage(const QString &fileName);

    double fullMatchMat(cv::Mat sceneImg);

    double scoreThreshold() const;
    void setScoreThreshold(double newScoreThreshold);

    int whiteThreshold() const;
    void setWhiteThreshold(int newWhiteThreshold);

    bool hasDefect(double scoreThreshold){
        return scoreThreshold > m_scoreThreshold;
    }

private:
    void addTemplate(const QString &desc, const QString &fileName);

    void addTemplateIntoMap(const QString &desc, const QString &fileName, cv::Mat tInput);

    double matchMat(cv::Mat templateInput, cv::Mat defectInput);

private:
    QList<std::tuple<QString, QString, cv::Mat>> m_huMomentsList;
    int m_removeOuterBorderThickness{3}; // 比对时忽略的边缘厚度
    int m_detectThickness{6}; // 比对时检测的边缘厚度
    int m_whiteThreshold{35}; // 差值结果阈值，大于这个值被认为是缺陷点，一般设置为30-40
    double m_scoreThreshold{15}; // 缺陷点的个数，根据下采样的次数决定，m_precision为2时，此数值一般为10-20

    // 以下参数不改
    int m_precision{2}; // 取决于进行几次下采样，暂时不可更改
    bool m_useHSV{true};

    bool m_debugImageFlag{false}; // false true 是否输出调试结果图片
};

#endif // DEFECTDETECTOR_H
