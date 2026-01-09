#ifndef TEMPLATEGRIDITEM_H
#define TEMPLATEGRIDITEM_H

#pragma execution_character_set("utf-8")

#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <opencv2/opencv.hpp>
#include "humomentsmatcher.h"

class TemplateGridItem : public QWidget {
    Q_OBJECT
public:
    explicit TemplateGridItem(const QString &imageName, const cv::Mat &imageData, const MatchResult &result, QWidget *parent = nullptr);
    QString getImageName() const { return m_imageName; }
    void setImageMat(const cv::Mat &imageData, const MatchResult &result);

    QString getMatchedTemplateName() const;
    void setMatchedTemplateName(const QString &newMatchedTemplateName);

    std::vector<cv::Point> getContour() const;
    void setContour(const std::vector<cv::Point> &newContour);

    cv::Point2f getCenter() const;
    void setCenter(cv::Point2f newCenter);

    double getScore() const;
    void setScore(double newScore);

    double getAreaDifferencePercent() const;
    void setAreaDifferencePercent(double newAreaDifferencePercent);

protected:
    void resizeEvent(QResizeEvent *event) override;

    void updatePixmap();

    void updateText();

private:
    void setupUI();

    QString m_imageName;
    cv::Mat m_imageData;

    QLabel *m_imageLabel = new QLabel;
    QLabel *m_infoLabel = new QLabel;

    QPixmap m_originalPixmap;
    QTimer *m_resizeTimer = new QTimer(this);

    QString m_matchedTemplateName{};             // 名称
    std::vector<cv::Point> m_contour;   // 轮廓
    cv::Point2f m_center;               // 中心点
    double m_score{-100.};               // 分数
    double m_areaDifferencePercent{-100.}; // 面积差值百分比
};

#endif // TEMPLATEGRIDITEM_H
