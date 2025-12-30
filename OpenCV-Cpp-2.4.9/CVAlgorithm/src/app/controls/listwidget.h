#ifndef LISTWIDGET_H
#define LISTWIDGET_H

#include <QListWidget>

class ListWidget : public QListWidget {
    Q_OBJECT
public:
    ListWidget();

protected:
    void focusOutEvent(QFocusEvent *event);
};

#endif // LISTWIDGET_H
