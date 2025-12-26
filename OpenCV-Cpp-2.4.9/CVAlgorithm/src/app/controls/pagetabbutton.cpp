#include "pagetabbutton.h"
#include <QPainterPath>

PageTabButton::PageTabButton(QWidget *parent)
    : QPushButton{parent} {
    this->setFlat(true);

    this->setAttribute(Qt::WA_Hover);

    this->updateAppearance();
}

void PageTabButton::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor fillColor = getCurrentColor();

    QRect rect = this->rect();
    this->drawContentArea(painter, rect, fillColor);
    this->drawText(painter, rect);
}

void PageTabButton::drawContentArea(QPainter &painter, const QRect &rect, const QColor &fillColor) {
    QRect innerRect = rect.adjusted(m_spacing, m_spacing, -m_spacing, 1);

    QPainterPath innerPath;

    if (innerRect.height() > m_cornerRadius) {
        innerPath.moveTo(innerRect.left(), innerRect.top() + m_cornerRadius);
        innerPath.arcTo(innerRect.left(), innerRect.top(),
                        m_cornerRadius * 2, m_cornerRadius * 2, 180, -90);
        innerPath.lineTo(innerRect.right() - m_cornerRadius, innerRect.top());
        innerPath.arcTo(innerRect.right() - m_cornerRadius * 2, innerRect.top(),
                        m_cornerRadius * 2, m_cornerRadius * 2, 90, -90);
        innerPath.lineTo(innerRect.right(), innerRect.bottom() - m_cornerRadius);
        innerPath.arcTo(innerRect.right(), innerRect.bottom() - m_cornerRadius,
                        m_cornerRadius, m_cornerRadius, 180, 90);
        innerPath.lineTo(innerRect.left() - m_cornerRadius, innerRect.bottom());
        innerPath.arcTo(innerRect.left() - m_cornerRadius, innerRect.bottom() - m_cornerRadius,
                        m_cornerRadius, m_cornerRadius, 270, 90);
        innerPath.closeSubpath();
    } else {
        innerPath.addRect(innerRect);
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(fillColor);
    painter.drawPath(innerPath);

    if (underMouse() && isEnabled()) {
        QLinearGradient gradient(innerRect.topLeft(), innerRect.bottomLeft());
        gradient.setColorAt(0, fillColor.lighter(120));
        gradient.setColorAt(1, fillColor.darker(120));
        painter.setBrush(gradient);
        painter.drawPath(innerPath);
    }
}

void PageTabButton::drawText(QPainter &painter, const QRect &rect) {
    QRect textRect = rect.adjusted(m_spacing, m_spacing, -m_spacing, 1);

    QFont font = painter.font();
    font.setPixelSize(20);
    font.setBold(true);
    painter.setFont(font);

    QPen pen;
    pen.setColor(this->getTextColor());

    painter.setPen(pen);
    painter.drawText(textRect, Qt::AlignCenter, text());

    if (isEnabled()) {
        painter.setPen(QColor(0, 0, 0, 30));
        painter.drawText(textRect.translated(0, 1), Qt::AlignCenter, text());
        painter.setPen(this->getTextColor());
        painter.drawText(textRect, Qt::AlignCenter, text());
    }
}
