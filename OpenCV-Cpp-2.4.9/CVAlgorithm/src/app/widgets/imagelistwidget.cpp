#include "imagelistwidget.h"
#include <QDebug>
#include <utils/fileutils.h>

ImageListWidget::ImageListWidget(QWidget *parent)
    : QWidget(parent)
    , m_browseButton(new QPushButton("浏览文件夹"))
    , m_listWidget(new ListWidget) {
    m_listWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_listWidget->setFrameShape(QFrame::NoFrame);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    this->setMinimumWidth(300);
    this->setMaximumWidth(300);

    setWindowTitle("图片列表浏览器");

    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection); // 单行选择
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_searchLineEdit = new LineEdit;
    m_searchLineEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(m_searchLineEdit, &LineEdit::textChanged, this, [=](const QString &searchText) {
        QString filterText = searchText.trimmed();

        for (int i = 0; i < m_listWidget->count(); ++i) {
            QListWidgetItem *item = m_listWidget->item(i);
            QString itemText = item->text();

            if (filterText.isEmpty() || itemText.contains(filterText, Qt::CaseInsensitive)) {
                item->setHidden(false);
            } else {
                item->setHidden(true);
            }
        }
    });

    Layouting::Column{m_searchLineEdit, m_listWidget}.attachTo(this);

    connect(m_listWidget, &QListWidget::itemSelectionChanged, this, [=]() {
        qDebug() << "itemSelectionChanged:";
        this->onListItemSelectionChanged();
    });
}

void ImageListWidget::loadImagesFromFolder(const QString &folderPath) {
    m_listWidget->clear();

    QDir dir(folderPath);
    if (!dir.exists()) {
        QMessageBox::warning(this, "警告", "指定的文件夹不存在！");
        return;
    }

    // QStringList nameFilters;
    // for (const QByteArray& format : QImageReader::supportedImageFormats()) {
    //     nameFilters << QString("*.") + QString(format).toLower();
    // }
    // QFileInfoList imageFiles = dir.entryInfoList(nameFilters, QDir::Files |
    // QDir::Readable, QDir::Name);

    QStringList imageFilesList = FileUtils::findAllImageFiles(folderPath);

    if (imageFilesList.isEmpty()) {
        QMessageBox::information(this, "提示", "该文件夹下未找到支持的图片文件。");
        return;
    }

    for (auto &file : imageFilesList) {
        QFileInfo fileInfo(file);
        ImageListItem *itemWidget = new ImageListItem(fileInfo);

        QListWidgetItem *listItem = new QListWidgetItem(m_listWidget);
        listItem->setSizeHint(itemWidget->sizeHint());

        listItem->setText(fileInfo.fileName());

        listItem->setData(Qt::UserRole, QVariant(fileInfo.absoluteFilePath()));

        m_listWidget->setItemWidget(listItem, itemWidget);
    }
}

void ImageListWidget::onListItemSelectionChanged() {
    QListWidgetItem *currentItem = m_listWidget->currentItem();
    qDebug() << "onListItemSelectionChanged currentItem:" << currentItem;
    if (currentItem) {
        QString imagePath = currentItem->data(Qt::UserRole).toString();
        qDebug() << "onListItemSelectionChanged imagePath:" << imagePath;
        // QString imagePath = m_itemToPathMap.value(currentItem);

        if (!imagePath.isEmpty()) {
            emit imageSelected(imagePath);
        }
    }
}
