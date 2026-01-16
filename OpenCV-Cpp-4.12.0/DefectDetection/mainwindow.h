#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <opencv2/opencv.hpp>
#include "DefectDetector.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadDefaultImages();
    void onDetectDefect();
    void onLoadNormalImage();
    void onLoadDefectImage();

private:
    // 图像显示相关
    QLabel *m_normalImageLabel;
    QLabel *m_defectImageLabel;
    QLabel *m_resultLabel;

    // 按钮
    QPushButton *m_loadNormalButton;
    QPushButton *m_loadDefectButton;
    QPushButton *m_detectButton;

    // 布局
    QWidget *m_centralWidget;
    QHBoxLayout *m_imagesLayout;
    QVBoxLayout *m_mainLayout;

    // 图像数据
    cv::Mat m_normalImage;
    cv::Mat m_defectImage;

    // 检测器
    DefectDetector *m_detector;

    // 图像显示方法
    void displayImageOnLabel(QLabel *label, const cv::Mat &image);
    cv::Mat resizeImageForDisplay(const cv::Mat &image, QLabel *label);
};

#endif // MAINWINDOW_H
