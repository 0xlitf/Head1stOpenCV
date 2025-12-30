#ifndef IMAGEGRIDITEM_H
#define IMAGEGRIDITEM_H

#pragma execution_character_set("utf-8")

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <opencv2/opencv.hpp>

class ImageGridItem : public QWidget
{
    Q_OBJECT
public:
    explicit ImageGridItem(const QString& imageName, const cv::Mat& imageData, QWidget *parent = nullptr);
    QString getImageName() const { return m_imageName; }

private:
    void setupUI();

    QString m_imageName;
    cv::Mat m_imageData;

    QLabel* m_imageLabel;
    QLabel* m_infoLabel;
};

#endif // IMAGEGRIDITEM_H
