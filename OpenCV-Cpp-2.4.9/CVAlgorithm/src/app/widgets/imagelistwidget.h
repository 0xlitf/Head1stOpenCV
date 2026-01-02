#ifndef IMAGELISTWIDGET_H
#define IMAGELISTWIDGET_H

#pragma execution_character_set("utf-8")

#include "controls/listwidget.h"
#include "imagelistitem.h"
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

class ImageListWidget : public QWidget {
    Q_OBJECT

signals:
    // 声明自定义信号，参数为图片的完整路径[1,2](@ref)
    void imageSelected(const QString &imagePath);

public:
    ImageListWidget(QWidget *parent = nullptr);

    void loadImagesFromFolder(const QString &folderPath);

    void onListItemSelectionChanged();

private:
    LineEdit *m_searchLineEdit = nullptr;
    QPushButton *m_browseButton;
    ListWidget *m_listWidget;
};

#endif // IMAGELISTWIDGET_H
