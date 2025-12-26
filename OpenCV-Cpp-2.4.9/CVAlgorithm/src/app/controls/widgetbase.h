#pragma once

#pragma execution_character_set("utf-8")

#include <QApplication>
#include <QElapsedTimer>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>

#include <QWidget>


class WidgetBase : public QWidget {
    Q_OBJECT
public:
    explicit WidgetBase(QWidget* parent = nullptr);
    explicit WidgetBase(const QColor& color, QWidget* parent = nullptr);

    static WidgetBase* create();

    static WidgetBase* randomColorWidget();

    static int generateRandomNumber();

    virtual ~WidgetBase();

    virtual QString realClassName();

    void setBackgroundColor(const QColor& color);

    void setRandomColor();

protected:
    // void mouseDoubleClickEvent(QMouseEvent* event) override;

    // void mouseMoveEvent(QMouseEvent* event) override;

    // void mousePressEvent(QMouseEvent* event) override;

    // void mouseReleaseEvent(QMouseEvent* event) override;

    void paintEvent(QPaintEvent* event) override;

signals:

private:
    QColor m_backgroundColor{QColor(247, 247, 247)};
};

class HSeperateLine : public WidgetBase {
public:
    explicit HSeperateLine(QWidget* parent = nullptr);
};

class VSeperateLine : public WidgetBase {
public:
    explicit VSeperateLine(QWidget* parent = nullptr);
};

