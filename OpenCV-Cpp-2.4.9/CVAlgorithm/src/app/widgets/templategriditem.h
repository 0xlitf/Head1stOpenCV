#ifndef TEMPLATEGRIDITEM_H
#define TEMPLATEGRIDITEM_H

#pragma execution_character_set("utf-8")

#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <opencv2/opencv.hpp>

class TemplateGridItem : public QWidget {
    Q_OBJECT
public:
    explicit TemplateGridItem(const QString &imageName, const cv::Mat &imageData, QWidget *parent = nullptr);
    QString getImageName() const { return m_imageName; }
    void setImageMat(const cv::Mat &imageData);

protected:
    void resizeEvent(QResizeEvent *event) override;

    void updatePixmap();

private:
    void setupUI();

    QString m_imageName;
    cv::Mat m_imageData;

    QLabel *m_imageLabel;
    QLabel *m_infoLabel;

    QPixmap m_originalPixmap;
    QTimer *m_resizeTimer = new QTimer(this);
};

#endif // TEMPLATEGRIDITEM_H
