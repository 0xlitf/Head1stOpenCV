#ifndef IMAGELISTVIEWWIDGET_H
#define IMAGELISTVIEWWIDGET_H

#pragma execution_character_set("utf-8")

#include "controls/controls.h"
#include "imagelistview.h"
#include "imagelistviewmodel.h"
#include "imagelistviewdelegate.h"
#include "imagelistviewfiltermodel.h"
#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QImageReader>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

class ImageListViewWidget : public QWidget {
    Q_OBJECT
public:
    explicit ImageListViewWidget(QWidget *parent = nullptr);

signals:
    // 声明自定义信号，参数为图片的完整路径[1,2](@ref)
    void imageSelected(const QString &imagePath);

public:

    void loadImagesFromFolder(const QString &folderPath);

public slots:
    void onListItemClicked(const QModelIndex &index);

private:
    LineEdit *m_searchLineEdit = nullptr;
    QPushButton *m_browseButton = nullptr;
    ImageListView* m_listView = nullptr;
    ImageListViewModel* m_model = nullptr;
    ImageListViewDelegate* m_delegate = nullptr;
    ImageListViewFilterModel* m_filterModel = nullptr;
};

#endif // IMAGELISTVIEWWIDGET_H
