#include "lineedit.h"
#include <QPainter>
#include <QAction>
#include <QIcon>
#include <QDebug>
#include <QPainterPath>

LineEdit::LineEdit(QWidget* parent)
    : QLineEdit(parent) {
    this->setMinimumSize(80, 35);

    QMargins margins;
    margins.setLeft(m_spacing * 2);
    this->setTextMargins(margins);

    QPalette palette = this->palette();
    palette.setColor(QPalette::Base, Qt::transparent);
    palette.setColor(QPalette::Text, QColor("#222222"));

    this->setPalette(palette);
    this->setFrame(false);

    this->setupActions();
}

LineEdit::LineEdit(const QString& str, QWidget* parent): LineEdit(parent) {
    this->setText(str);
}

void LineEdit::setBorderColor(const QColor& color) {
    m_borderColor = color;
    update();
}

void LineEdit::setBorderWidth(int width) {
    m_borderWidth = width;
    update();
}

void LineEdit::setCornerRadius(int radius) {
    m_cornerRadius = radius;
    update();
}

void LineEdit::setupActions() {
    auto createClearIcon = []() {
        QPixmap pixmap(16, 16);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);

        painter.setPen(QPen(Qt::gray, 2));
        painter.drawLine(2, 2, 14, 14);
        painter.drawLine(14, 2, 2, 14);

        return QIcon(pixmap);
    };
    m_settingsAction = new QAction(createClearIcon(), "", this);
    connect(m_settingsAction, &QAction::triggered, this, [this]() { this->setText(""); });

    updateActionVisibility();
}

void LineEdit::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect innerRect = this->rect().adjusted(m_spacing, m_spacing, -m_spacing, -m_spacing);
    QRect bgRect = innerRect.adjusted(m_borderWidth, m_borderWidth, -m_borderWidth, -m_borderWidth);
    painter.setBrush(palette().base());
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(bgRect, m_cornerRadius, m_cornerRadius);

    QPen borderPen(m_borderColor, m_borderWidth);
    painter.setPen(borderPen);
    if (this->hasFocus()) {
        painter.setBrush(Qt::white);
    } else {
        painter.setBrush(QColor("#ffffff").darker(105));
    }
    painter.drawRoundedRect(innerRect.adjusted(0, 0, 0, 0), m_cornerRadius, m_cornerRadius);

    drawFocusIndicator(painter);

    QRect textRect = rect().adjusted(2, 2, -25, -2);
    painter.setClipRect(textRect);

    QLineEdit::paintEvent(event);
}

void LineEdit::drawFocusIndicator(QPainter& painter) {
    QRect innerRect = this->rect().adjusted(m_spacing, m_spacing, -m_spacing, 0);
    painter.save();

    QPainterPath maskPath;
    QRect clipRect = this->rect().adjusted(m_spacing, m_spacing, -m_spacing, -m_spacing);
    maskPath.addRoundedRect(clipRect, m_cornerRadius, m_cornerRadius);

    painter.setClipPath(maskPath);

    QColor lineColor = this->hasFocus() ? QColor("#005a9e") : QColor("#999999");
    int lineHeight = this->hasFocus() ? 2 : 1;

    int lineY = innerRect.height() - lineHeight;
    painter.setPen(Qt::NoPen);
    painter.setBrush(lineColor);
    painter.drawRect(m_spacing, lineY, innerRect.width(), lineHeight);

    painter.restore();
}
