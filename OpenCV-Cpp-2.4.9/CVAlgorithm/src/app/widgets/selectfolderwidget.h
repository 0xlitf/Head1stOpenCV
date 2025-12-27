#ifndef SELECTFOLDERWIDGET_H
#define SELECTFOLDERWIDGET_H

#include "controls/controls.h"

class SelectFolderWidget: public WidgetBase {
    Q_OBJECT
public:
    explicit SelectFolderWidget(QWidget *parent = nullptr);

private:
    void createComponents();
};

#endif // SELECTFOLDERWIDGET_H
