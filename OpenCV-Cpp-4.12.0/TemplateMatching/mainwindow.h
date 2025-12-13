#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QStatusBar>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QMessageBox>
#include <QImage>
#include <QPixmap>
#include <QScrollArea>
#include <QFrame>
#include <QTextEdit>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QSplitter>
#include <QComboBox>

#include <opencv2/opencv.hpp>
#include "GeoMatch.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 菜单栏动作
    void openTemplateImage();
    void openSearchImage();
    void saveResultImage();
    void exitApplication();
    void aboutApplication();

    // 按钮点击事件
    void startMatching();
    void resetParameters();

    // 参数变化事件
    void lowThresholdChanged(int value);
    void highThresholdChanged(int value);
    void minScoreChanged(double value);
    void greedinessChanged(double value);
    void edgeMethodChanged(int index);

    // 实时预览
    void updateTemplatePreview();
    void updateSearchPreview();

private:
    // 初始化函数
    void setupUI();
    void setupMenuBar();
    void setupTemplatePanel();
    void setupSearchPanel();
    void setupControlPanel();
    void setupStatusBar();
    void setupConnections();

    // 图像处理函数
    QImage matToQImage(const cv::Mat &mat);
    void displayImage(const cv::Mat &mat, QGraphicsView *view, const QString &title = "");
    void updateTemplateDisplay();
    void updateSearchDisplay();

    // 工具函数
    void showError(const QString &message);
    void showInfo(const QString &message);
    void updateStatus(const QString &message);

    // 核心匹配函数
    void performMatching();

private:
    // UI组件
    QWidget *centralWidget;
    QSplitter *mainSplitter;

    // 菜单栏
    QMenuBar *m_menuBar;
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *helpMenu;
    QAction *openTemplateAction;
    QAction *openSearchAction;
    QAction *saveResultAction;
    QAction *exitAction;
    QAction *resetAction;
    QAction *aboutAction;

    // 左侧模板面板
    QGroupBox *templateGroup;
    QLabel *templateLabel;
    QPushButton *selectTemplateBtn;
    QGraphicsView *templateView;
    QGraphicsScene *templateScene;
    QLabel *lowThreshLabel;
    QSlider *lowThreshSlider;
    QLabel *highThreshLabel;
    QSlider *highThreshSlider;
    QLabel *templateInfoLabel;
    QComboBox *edgeMethodCombo;

    // 右侧搜索面板
    QGroupBox *searchGroup;
    QLabel *searchLabel;
    QPushButton *selectSearchBtn;
    QGraphicsView *searchView;
    QGraphicsScene *searchScene;
    QLabel *searchInfoLabel;
    QLabel *resultLabel;
    QTextEdit *logTextEdit;

    // 底部控制面板
    QGroupBox *controlGroup;
    QLabel *minScoreLabel;
    QDoubleSpinBox *minScoreSpin;
    QLabel *greedinessLabel;
    QDoubleSpinBox *greedinessSpin;
    QPushButton *matchBtn;
    QPushButton *resetBtn;
    QLabel *matchResultLabel;

    // 状态栏
    QStatusBar *m_statusBar;
    QLabel *statusLabel;

    // 图像数据
    cv::Mat templateImage;
    cv::Mat searchImage;
    cv::Mat resultImage;
    cv::Mat templatePreview;
    cv::Mat searchPreview;

    // 算法对象
    GeoMatch geoMatcher;

    // 算法参数
    int lowThreshold = 10;
    int highThreshold = 100;
    double minScore = 0.7;
    double greediness = 0.8;
    int edgeMethod = 0;  // 0: Canny, 1: Sobel, 2: Laplacian

    // 匹配结果
    cv::Point matchPoint;
    double matchScore = 0.0;
    double matchTime = 0.0;
    bool isMatched = false;

    // 图像路径
    QString templateImagePath;
    QString searchImagePath;
};
#endif // MAINWINDOW_H
