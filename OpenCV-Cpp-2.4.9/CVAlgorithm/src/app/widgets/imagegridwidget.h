#ifndef IMAGEGRIDWIDGET_H
#define IMAGEGRIDWIDGET_H

#pragma execution_character_set("utf-8")

#include "imagegriditem.h"
#include <QGridLayout>
#include <QMap>
#include <QScrollArea>
#include <QWidget>

class ImageGridWidget : public QScrollArea {
    Q_OBJECT
public:
    explicit ImageGridWidget(QWidget *parent = nullptr);
    void addImage(const QString &name, const cv::Mat &image);
    void removeImage(const QString &name);
    void clearAllImages();

    int maxColumns() const;
    void setMaxColumns(int newMaxColumns);

protected:
    void resizeEvent(QResizeEvent *event) override;

    void updateChildren();

private:
    int calculateIdealItemWidth(int parentWidth);
    int calculateIdealItemHeight(int parentHeight);

private:
    QWidget *m_containerWidget;
    QGridLayout *m_gridLayout;
    QMap<QString, ImageGridItem *> m_imageItems;
    int m_maxColumns{3};

    QTimer *m_resizeTimer = new QTimer(this);
    double idealItemWidth{-1};
    double idealItemHeight{-1};
};

#endif // IMAGEGRIDWIDGET_H
