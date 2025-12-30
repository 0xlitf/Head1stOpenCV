#ifndef GROUPBOX_H
#define GROUPBOX_H

#include <QGroupBox>
#include <QPainter>
#include <QStyleOptionGroupBox>

class GroupBox : public QGroupBox
{
    Q_OBJECT
public:
    explicit GroupBox(QWidget *parent = nullptr) : QGroupBox(parent) {}
    explicit GroupBox(const QString &title, QWidget *parent = nullptr) : QGroupBox(title, parent) {}

protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // GROUPBOX_H
