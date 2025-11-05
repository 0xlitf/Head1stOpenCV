#ifndef IMAGEBINARIZER_H
#define IMAGEBINARIZER_H

#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QImage>
#include <QSlider>
#include <QHBoxLayout>

class ImageBinarizer : public QWidget {
    Q_OBJECT

public:
    ImageBinarizer(QWidget *parent = nullptr) : QWidget(parent) {
        setupUI();
    }

private slots:
    void loadImage();

    void onThresholdChanged(int value);

private:
    void setupUI();

    void displayOriginalImage();

    void binarizeImage();

    QImage originalImage;
    QLabel *originalLabel;
    QLabel *binaryLabel;
    QLabel *thresholdLabel;
    int thresholdValue = 128;
};

#endif // IMAGEBINARIZER_H
