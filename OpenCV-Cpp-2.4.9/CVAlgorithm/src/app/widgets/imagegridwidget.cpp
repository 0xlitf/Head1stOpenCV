#include "imagegridwidget.h"

ImageGridWidget::ImageGridWidget(QWidget *parent)
    : QScrollArea(parent), m_containerWidget(new QWidget), m_gridLayout(new QGridLayout(m_containerWidget)), m_maxColumns(2) // 默认每行4列
{
    setWidget(m_containerWidget);
    setWidgetResizable(true); // 允许容器随布局调整大小
    m_containerWidget->setLayout(m_gridLayout);
    m_gridLayout->setSpacing(10); // 设置网格项之间的间距
    m_gridLayout->setAlignment(Qt::AlignTop); // 对齐方式
}

void ImageGridWidget::addImage(const QString& name, const cv::Mat& image)
{
    // 如果已存在同名项，先移除
    if (m_imageItems.contains(name)) {
        removeImage(name);
    }

    // 创建新的网格项
    ImageGridItem* newItem = new ImageGridItem(name, image, m_containerWidget);
    m_imageItems[name] = newItem;

    // 计算新项应放置的位置
    int totalItems = m_imageItems.size() - 1; // 新项插入前的总数，即新索引
    int row = totalItems / m_maxColumns;
    int column = totalItems % m_maxColumns;

    // 将新项添加到网格布局中
    m_gridLayout->addWidget(newItem, row, column);
}

void ImageGridWidget::removeImage(const QString& name)
{
    if (m_imageItems.contains(name)) {
        ImageGridItem* itemToRemove = m_imageItems.value(name);
        // 从布局中移除控件
        m_gridLayout->removeWidget(itemToRemove);
        // 从管理Map中移除
        m_imageItems.remove(name);
        // 安全删除控件
        itemToRemove->deleteLater();

        // 可选：重新排列剩余项以获得更紧凑的布局（此操作可能较耗时，图片多时慎用）
        // 一个简单的思路是清空布局，然后按Map顺序重新添加所有项。
    }
}

void ImageGridWidget::clearAllImages()
{
    // 移除所有项
    QMapIterator<QString, ImageGridItem*> i(m_imageItems);
    while (i.hasNext()) {
        i.next();
        m_gridLayout->removeWidget(i.value());
        i.value()->deleteLater();
    }
    m_imageItems.clear();
}
