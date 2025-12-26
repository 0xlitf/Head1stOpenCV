#include "normalbutton.h"
#include <QPainterPath>

NormalButton::NormalButton(QWidget *parent)
    : QPushButton{parent} {
    // this->setFlat(true);

    this->setMinimumSize(50, 35);

    this->setAttribute(Qt::WA_Hover);

    this->updateAppearance();
}

NormalButton::NormalButton(const QString& str, QWidget *parent)
    : NormalButton{parent} {
    this->setText(str);
}

void NormalButton::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor fillColor = getCurrentColor();

    QRect rect = this->rect();
    this->drawOuterBorder(painter, rect);
    this->drawContentArea(painter, rect, fillColor);
    this->drawText(painter, rect);
}

void NormalButton::drawOuterBorder(QPainter &painter, const QRect &rect) {
    QRect innerRect = rect.adjusted(m_spacing, m_spacing, -m_spacing, -m_spacing);

    painter.setPen(QPen(m_borderColor, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(innerRect, m_cornerRadius, m_cornerRadius);
    return;
    QPainterPath outerPath;

    QRect outerRect = rect.adjusted(0, 0, 0, 0);

    outerPath.moveTo(outerRect.left(), outerRect.top() + m_cornerRadius);
    outerPath.arcTo(outerRect.left(), outerRect.top(),
                    m_cornerRadius * 2, m_cornerRadius * 2, 180, -90);
    outerPath.lineTo(outerRect.right() - m_cornerRadius, outerRect.top());
    outerPath.arcTo(outerRect.right() - m_cornerRadius * 2, outerRect.top(),
                    m_cornerRadius * 2, m_cornerRadius * 2, 90, -90);
    outerPath.lineTo(outerRect.right(), outerRect.bottom());
    outerPath.lineTo(outerRect.left(), outerRect.bottom());
    outerPath.closeSubpath();

    painter.setPen(QPen(m_borderColor, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(outerPath);
}

void NormalButton::drawContentArea(QPainter &painter, const QRect &rect, const QColor &fillColor) {
    QRect innerRect = rect.adjusted(m_spacing, m_spacing, -m_spacing, -m_spacing);

    painter.setPen(Qt::NoPen);
    painter.setBrush(fillColor);
    painter.drawRoundedRect(innerRect, m_cornerRadius, m_cornerRadius);
}

void NormalButton::drawText(QPainter &painter, const QRect &rect) {
    QRect textRect = rect.adjusted(m_spacing * 2, m_spacing, -m_spacing * 2,  -m_spacing);

    QPen pen;
    pen.setColor(this->getTextColor());

    painter.setPen(pen);
    painter.drawText(textRect, Qt::AlignCenter, text());
}
