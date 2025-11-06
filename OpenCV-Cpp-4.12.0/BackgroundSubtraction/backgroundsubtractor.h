#ifndef BACKGROUNDSUBTRACTOR_H
#define BACKGROUNDSUBTRACTOR_H

#include <QWidget>
#include <QImage>
#include <QPixmap>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QGroupBox>
#include <QComboBox>

#include <opencv2/opencv.hpp>


class BackgroundSubtractor : public QWidget
{
    Q_OBJECT

public:
    BackgroundSubtractor(QWidget *parent = nullptr);
    ~BackgroundSubtractor();

    cv::Mat qImageToCvMat(const QImage &qimage);

    QImage cvMatToQImage(const cv::Mat &mat);
private slots:
    void loadImageA();
    void loadImageB();
    void subtractBackground();
    void onThresholdChanged(int value);
    void onMethodChanged(int index);
    void saveResult();

private:
    void setupUI();
    void updateDisplays();
    std::tuple<QImage, QImage> subtractSimple(const QImage &imgA, const QImage &imgB);
    std::tuple<QImage, QImage> subtractAdvanced(const QImage &imgA, const QImage &imgB);
    QImage applyMorphologicalOperations(const QImage &binaryImage);

    // 新增函数：提取大物体并画红框
    QImage extractLargeObjectsWithBoundingBox(const QImage &mask, const QImage &original);

    // 辅助函数：查找轮廓
    std::vector<std::vector<cv::Point>> findContours(const cv::Mat &maskMat);

    // 辅助函数：过滤小轮廓
    std::vector<std::vector<cv::Point>> filterLargeContours(
        const std::vector<std::vector<cv::Point>> &contours,
        double minArea = 1000.0  // 最小面积阈值
        );

private:
    // UI 组件
    QLabel *m_imageALabel;
    QLabel *m_imageBLabel;
    QLabel *m_resultLabel;
    QLabel *m_resultLabelWithRect;

    QLabel *m_maskLabelBefore;
    QLabel *m_maskLabel;
    QPushButton *m_loadButtonA;
    QPushButton *m_loadButtonB;
    QPushButton *m_subtractButton;
    QPushButton *m_saveButton;
    QSlider *m_thresholdSlider;
    QLabel *m_thresholdValueLabel;
    QComboBox *m_methodComboBox;

    // 图像数据
    QImage m_imageA;
    QImage m_imageB;
    QImage m_resultImage;
    QImage m_resultImageWithRect;
    QImage m_resultImageBefore;
    QImage m_maskImage;
    QImage m_maskImageBefore;

    QString m_fileNameA;
    QString m_fileNameB;

    // 参数
    int threshold;
};

#endif // BACKGROUNDSUBTRACTOR_H
