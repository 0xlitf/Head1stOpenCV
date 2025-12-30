#ifndef IMAGEGRIDWIDGET_H
#define IMAGEGRIDWIDGET_H

#pragma execution_character_set("utf-8")

#include "imagegriditem.h"
#include <QGridLayout>
#include <QMap>
#include <QScrollArea>
#include <QWidget>

class ImageGridWidget
    : public QScrollArea // 使用QScrollArea以支持大量图片的滚动浏览
{
    Q_OBJECT
public:
    explicit ImageGridWidget(QWidget *parent = nullptr);
    void addImage(const QString &name, const cv::Mat &image);
    void removeImage(const QString &name);
    void clearAllImages();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    int calculateIdealItemWidth(int parentWidth);
    int calculateIdealItemHeight(int parentHeight);
    void removeAll();

private:
    QWidget *m_containerWidget; // 滚动区域的容器部件
    QGridLayout *m_gridLayout;
    QMap<QString, ImageGridItem *> m_imageItems;
    int m_maxColumns;
};

#endif // IMAGEGRIDWIDGET_H
