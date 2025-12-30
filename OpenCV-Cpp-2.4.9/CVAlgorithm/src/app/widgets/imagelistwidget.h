#ifndef IMAGELISTWIDGET_H
#define IMAGELISTWIDGET_H

#pragma execution_character_set("utf-8")

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QImageReader>
#include "imagelistitem.h"
#include "controls/listwidget.h"

class ImageListWidget : public QWidget
{
    Q_OBJECT

signals:
    // 声明自定义信号，参数为图片的完整路径[1,2](@ref)
    void imageSelected(const QString& imagePath);

public:
    ImageListWidget(QWidget *parent = nullptr);

    void loadImagesFromFolder(const QString& folderPath);

    void onListItemSelectionChanged();

private:
    QLineEdit* m_pathEdit;
    QPushButton* m_browseButton;
    ListWidget* m_listWidget;
};

#endif // IMAGELISTWIDGET_H
