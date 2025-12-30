#include "imagegridwidget.h"
#include <QDebug>

ImageGridWidget::ImageGridWidget(QWidget *parent)
    : QScrollArea(parent)
    , m_containerWidget(new QWidget)
    , m_gridLayout(new QGridLayout(m_containerWidget))
    , m_maxColumns(2)
{
    setWidget(m_containerWidget);
    setWidgetResizable(true);
    m_containerWidget->setLayout(m_gridLayout);
    m_gridLayout->setSpacing(10);             // 设置网格项之间的间距
    m_gridLayout->setAlignment(Qt::AlignTop); // 对齐方式
}

void ImageGridWidget::addImage(const QString &name, const cv::Mat &image) {
    if (m_imageItems.contains(name)) {
        removeImage(name);
    }

    ImageGridItem *newItem = new ImageGridItem(name, image, m_containerWidget);
    m_imageItems[name] = newItem;

    int totalItems = m_imageItems.size() - 1; // 新项插入前的总数，即新索引
    int row = totalItems / m_maxColumns;
    int column = totalItems % m_maxColumns;

    m_gridLayout->addWidget(newItem, row, column);
}

void ImageGridWidget::removeImage(const QString &name) {
    if (m_imageItems.contains(name)) {
        ImageGridItem *itemToRemove = m_imageItems.value(name);
        m_gridLayout->removeWidget(itemToRemove);
        m_imageItems.remove(name);
        itemToRemove->deleteLater();

        // 可选：重新排列剩余项以获得更紧凑的布局（此操作可能较耗时，图片多时慎用）
        // 一个简单的思路是清空布局，然后按Map顺序重新添加所有项。
    }
}

void ImageGridWidget::clearAllImages() {
    // 移除所有项
    QMapIterator<QString, ImageGridItem *> i(m_imageItems);
    while (i.hasNext()) {
        i.next();
        m_gridLayout->removeWidget(i.value());
        i.value()->deleteLater();
    }
    m_imageItems.clear();
}

void ImageGridWidget::resizeEvent(QResizeEvent *event) {
    QScrollArea::resizeEvent(event);

    QSize newSize = viewport()->size();

    int idealItemWidth = calculateIdealItemWidth(newSize.width());
    int idealItemHeight = calculateIdealItemHeight(newSize.height());

    qDebug() << "idealItemWidth Height" << idealItemWidth << idealItemHeight;

    for (ImageGridItem *item : m_imageItems) {
        item->setFixedWidth(idealItemWidth);
        item->setFixedHeight(idealItemHeight);
    }
}

int ImageGridWidget::calculateIdealItemWidth(int parentWidth) {
    int spacing = 10;
    int totalSpacing = (m_maxColumns - 1) * spacing;

    int availableWidth = parentWidth - totalSpacing - 20;
    int idealWidth = availableWidth / m_maxColumns;

    idealWidth = std::max(100, idealWidth);
    return idealWidth;
}

int ImageGridWidget::calculateIdealItemHeight(int parentHeight) {
    int spacing = 10;

    int availableHeight = parentHeight - 30;
    int idealHeight = availableHeight / m_maxColumns;

    idealHeight = std::max(100, idealHeight);
    return idealHeight;
}
