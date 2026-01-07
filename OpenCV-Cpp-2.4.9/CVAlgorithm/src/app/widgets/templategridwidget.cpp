#include "templategridwidget.h"
#include <QDebug>

TemplateGridWidget::TemplateGridWidget(QWidget *parent)
    : QScrollArea(parent)
    , m_containerWidget(new QWidget)
    , m_gridLayout(new QGridLayout(m_containerWidget)) {
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setWidget(m_containerWidget);
    setWidgetResizable(true);
    m_containerWidget->setLayout(m_gridLayout);
    m_gridLayout->setSpacing(10);
    m_gridLayout->setAlignment(Qt::AlignTop);

    m_resizeTimer->setSingleShot(true);

    connect(m_resizeTimer, &QTimer::timeout, this, [=]() {
        qDebug() << "TemplateGridWidget::updateChildren";
        this->updateChildren();
    });
}

void TemplateGridWidget::addImage(const QString &name, const cv::Mat &image) {
    if (m_imageItems.contains(name)) {
        // removeImage(name);

        qDebug() << "m_imageItems.contains" << name;
        TemplateGridItem *item = m_imageItems[name];
        item->setImageMat(image);
        return;
    }

    TemplateGridItem *newItem = new TemplateGridItem(name, image, m_containerWidget);
    newItem->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    if (idealItemWidth > 0 && idealItemHeight > 0) {
        newItem->setFixedWidth(idealItemWidth);
        newItem->setFixedHeight(idealItemHeight);
    }
    m_imageItems[name] = newItem;

    int totalItems = m_imageItems.size() - 1; // 新项插入前的总数，即新索引
    int row = totalItems / m_maxColumns;
    int column = totalItems % m_maxColumns;

    m_gridLayout->addWidget(newItem, row, column);
}

void TemplateGridWidget::removeImage(const QString &name) {
    if (m_imageItems.contains(name)) {
        TemplateGridItem *itemToRemove = m_imageItems.value(name);
        m_gridLayout->removeWidget(itemToRemove);
        m_imageItems.remove(name);
        itemToRemove->deleteLater();
    }
}

void TemplateGridWidget::clearAllImages() {
    QMapIterator<QString, TemplateGridItem *> i(m_imageItems);
    while (i.hasNext()) {
        i.next();
        m_gridLayout->removeWidget(i.value());
        i.value()->deleteLater();
    }
    m_imageItems.clear();
}

void TemplateGridWidget::resizeEvent(QResizeEvent *event) {
    QScrollArea::resizeEvent(event);

    qDebug() << "TemplateGridWidget::resizeEvent";
    m_resizeTimer->start(100);
}

void TemplateGridWidget::updateChildren() {
    QSize viewportSize = viewport()->size();

    idealItemWidth = calculateIdealItemWidth(viewportSize.width());
    idealItemHeight = calculateIdealItemHeight(viewportSize.height());

    qDebug() << "idealItemWidth Height" << idealItemWidth << idealItemHeight;

    for (TemplateGridItem *item : m_imageItems) {
        item->setFixedWidth(idealItemWidth);
        item->setFixedHeight(idealItemHeight);
    }
}

int TemplateGridWidget::calculateIdealItemWidth(int parentWidth) {
    int spacing = 10;
    int totalSpacing = (m_maxColumns - 1) * spacing;

    int availableWidth = parentWidth - totalSpacing - 20;
    int idealWidth = availableWidth / m_maxColumns;

    idealWidth = std::max(120, idealWidth);
    return idealWidth;
}

int TemplateGridWidget::calculateIdealItemHeight(int parentHeight) {
    int availableHeight = parentHeight - 30;
    int idealHeight = availableHeight / 2;

    idealHeight = std::max(120, idealHeight);
    return idealHeight;
}

int TemplateGridWidget::maxColumns() const {
    return m_maxColumns;
}

void TemplateGridWidget::setMaxColumns(int newMaxColumns) {
    m_maxColumns = newMaxColumns;
}
