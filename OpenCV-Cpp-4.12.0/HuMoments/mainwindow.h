#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <opencv2/opencv.hpp>

class MainWindow : public QMainWindow
{
    Q_OBJECT
    void createComponents();
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoadTemplate();
    void onLoadScene();
    void onRunMatching();

private:
    // UI 组件
    QLabel *lblTemplateDisplay;
    QLabel *lblSceneDisplay;
    QTextEdit *txtLog;
    QPushButton *btnLoadTemplate;
    QPushButton *btnLoadScene;
    QPushButton *btnMatch;

    // 图像数据
    cv::Mat m_templateImg; // 原始模板图
    cv::Mat m_sceneImg;    // 原始场景图
    std::vector<cv::Point> m_templateContour; // 存储提取出的模板轮廓

    // 辅助函数：将 cv::Mat 转换为 QPixmap 用于显示
    QPixmap cvMatToQPixmap(const cv::Mat &inMat);

    // 辅助函数：获取最大的轮廓
    std::vector<cv::Point> findLargestContour(const cv::Mat &srcInfo, bool isTemplate);
};

#endif // MAINWINDOW_H
