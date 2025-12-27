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

class ImageListWidget : public QWidget
{
    Q_OBJECT

public:
    ImageListWidget(QWidget *parent = nullptr);

    void loadImagesFromFolder(const QString& folderPath);

private:
    QLineEdit* m_pathEdit;
    QPushButton* m_browseButton;
    QListWidget* m_listWidget;
};

#endif // IMAGELISTWIDGET_H
