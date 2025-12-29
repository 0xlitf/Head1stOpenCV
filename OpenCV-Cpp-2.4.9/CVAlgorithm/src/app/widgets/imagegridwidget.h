#ifndef IMAGEGRIDWIDGET_H
#define IMAGEGRIDWIDGET_H

#include "controls/controls.h"

class ImageGridWidget : public WidgetBase {
    Q_OBJECT
public:
    explicit ImageGridWidget(QWidget *parent = nullptr);

    void createComponents();

signals:
};

#endif // IMAGEGRIDWIDGET_H
