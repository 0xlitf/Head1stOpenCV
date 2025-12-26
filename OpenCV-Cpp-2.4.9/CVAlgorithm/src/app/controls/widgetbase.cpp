#include "widgetbase.h"

#include <QDebug>
#include <QTime>
#include <random>

WidgetBase::WidgetBase(QWidget* parent) : QWidget{parent} {
    this->setContentsMargins(QMargins(0, 0, 0, 0));
}

WidgetBase::WidgetBase(const QColor& color, QWidget* parent) : WidgetBase(parent) {
    this->setBackgroundColor(color);
}

WidgetBase* WidgetBase::create() {
    return new WidgetBase;
}

WidgetBase* WidgetBase::randomColorWidget() {
    auto w = new WidgetBase;
    w->setRandomColor();
    return w;
}

int WidgetBase::generateRandomNumber() {
    std::random_device rd;
    std::mt19937 gen(rd());  // Mersenne Twister
    std::uniform_int_distribution<> dis(1, 255);

    return dis(gen);
}

WidgetBase::~WidgetBase() {
}

QString WidgetBase::realClassName() {
    return this->metaObject()->className();
}

void WidgetBase::setBackgroundColor(const QColor& color) {
    m_backgroundColor = color;
}

void WidgetBase::setRandomColor() {
    auto randInt = generateRandomNumber();
    QColor randomColor = QColor::fromRgb(randInt, randInt, randInt);
    this->setBackgroundColor(randomColor);
}

void WidgetBase::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.fillRect(rect(), m_backgroundColor);

    QWidget::paintEvent(event);
}

HSeperateLine::HSeperateLine(QWidget* parent) {
    this->setFixedHeight(1);
    this->setBackgroundColor(QColor("#404040"));
}

VSeperateLine::VSeperateLine(QWidget* parent) {
    this->setFixedWidth(1);
    this->setBackgroundColor(QColor("#404040"));
}

