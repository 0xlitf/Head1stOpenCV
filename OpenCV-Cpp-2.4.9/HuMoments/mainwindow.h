#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma execution_character_set("utf-8")

#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include "humomentsmatcher.h"
#include "fileutils.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
    void createComponents();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoadTemplateFolder();

    void onLoadTemplate();
    void onLoadScene();
    void onRunMatching();

private:
    // UI 组件
    QLabel *m_templateFolderLabel;
    QLabel *m_templateDescLabel;

    QLabel *m_templateLabel;
    QLabel *m_contourLabel;
    QLabel *m_sceneLabel;
    QTextEdit *m_logTextEdit;
    QPushButton *m_loadTemplateButton;
    QPushButton *m_loadSceneButton;
    QPushButton *m_matchButton;

    std::vector<cv::Point> m_templateContour; // 存储提取出的模板轮廓

    // 图像数据
    cv::Mat m_templateImg;                    // 原始模板图
    cv::Mat m_sceneImg;                       // 原始场景图

    HuMomentsMatcher m_matcher;

    QString m_sceneFileName;
};

#endif // MAINWINDOW_H
