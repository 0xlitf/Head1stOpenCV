#ifndef SELECTFILEWIDGET_H
#define SELECTFILEWIDGET_H

#include "controls/controls.h"

class SelectFileWidget: public WidgetBase {
    Q_OBJECT
public:
    explicit SelectFileWidget(QWidget *parent = nullptr);

private:
    void createComponents();
};

#endif // SELECTFILEWIDGET_H
