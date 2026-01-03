#ifndef IMAGELISTVIEW_H
#define IMAGELISTVIEW_H

#pragma execution_character_set("utf-8")

#include <QListView>

class ImageListView : public QListView {
    Q_OBJECT
public:
    ImageListView(QWidget *parent = 0);
};

#endif // IMAGELISTVIEW_H
