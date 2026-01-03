#include "imagelistviewwidget.h"

#include <utils/fileutils.h>

ImageListViewWidget::ImageListViewWidget(QWidget *parent)
    : QWidget{parent} {
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    this->setMinimumWidth(300);
    this->setMaximumWidth(300);

    // 创建模型、委托和视图
    m_model = new ImageListViewModel(this);
    m_delegate = new ImageListViewDelegate(this);
    m_filterModel = new ImageListViewFilterModel(this);
    m_listView = new ImageListView(this);

    m_filterModel->setSourceModel(m_model);
    m_filterModel->setFilterCaseSensitivity(Qt::CaseInsensitive); // 不区分大小写

    m_listView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_listView->setFrameShape(QFrame::NoFrame);

    m_listView->setModel(m_filterModel);
    m_listView->setItemDelegate(m_delegate);

    // 保持你原有的样式设置
    m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 性能关键设置：如果所有项高度一致，强烈建议开启
    m_listView->setUniformItemSizes(true);

    // 连接新的信号槽
    // 注意：QListView 的当前项变化信号是 clicked() 或 activated()
    connect(m_listView, &QListView::clicked, this, [=](const QModelIndex &index){
        QModelIndex sourceIndex = m_filterModel->mapToSource(index);
        this->onListItemClicked(sourceIndex);
    });

    m_searchLineEdit = new LineEdit;
    m_searchLineEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_searchLineEdit->setPlaceholderText("搜索图片...");
    connect(m_searchLineEdit, &LineEdit::textChanged, this, [=](const QString &searchText) {
        m_filterModel->setFilterFixedString(searchText.trimmed());
    });


    Layouting::Column{m_searchLineEdit, m_listView}.attachTo(this);

}

void ImageListViewWidget::loadImagesFromFolder(const QString &folderPath) {
    QStringList imageFilesList = FileUtils::findAllImageFiles(folderPath);
    m_model->setImageFiles(imageFilesList); // 一次性设置数据到模型
}

void ImageListViewWidget::onListItemClicked(const QModelIndex &index) {
    QString imagePath = m_model->filePath(index);
    if (!imagePath.isEmpty()) {
        emit imageSelected(imagePath);
    }
}
