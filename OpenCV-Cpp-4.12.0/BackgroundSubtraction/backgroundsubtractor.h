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

class BackgroundSubtractor : public QWidget
{
    Q_OBJECT

public:
    BackgroundSubtractor(QWidget *parent = nullptr);
    ~BackgroundSubtractor();

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
    QImage subtractSimple(const QImage &imgA, const QImage &imgB);
    QImage subtractAdvanced(const QImage &imgA, const QImage &imgB);
    QImage applyMorphologicalOperations(const QImage &binaryImage);

private:
    // UI 组件
    QLabel *imageALabel;
    QLabel *imageBLabel;
    QLabel *resultLabel;
    QLabel *maskLabel;
    QPushButton *loadButtonA;
    QPushButton *loadButtonB;
    QPushButton *subtractButton;
    QPushButton *saveButton;
    QSlider *thresholdSlider;
    QLabel *thresholdValueLabel;
    QComboBox *methodComboBox;

    // 图像数据
    QImage imageA;
    QImage imageB;
    QImage resultImage;
    QImage maskImage;

    // 参数
    int threshold;
};

#endif // BACKGROUNDSUBTRACTOR_H
