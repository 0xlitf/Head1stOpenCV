#include "checkbox.h"
#include <QPainterPath>

CheckBox::CheckBox(QWidget *parent)
    : QCheckBox{parent} {}

CheckBox::CheckBox(const QString &text, QWidget *parent)
    : QCheckBox{text, parent} {}

void CheckBox::drawCheckBoxBackground(QPainter &painter) {
    int boxSize = 18;
    int margin = 2;
    QRect boxRect(margin, (height() - boxSize) / 2, boxSize, boxSize);

    QColor borderColor = isEnabled() ? QColor("#666666") : QColor("#cccccc");
    QColor bgColor;

    if (!isEnabled()) {
        bgColor = QColor("#f5f5f5");
    } else if (isChecked()) {
        bgColor = QColor("#0078d4");
    } else {
        bgColor = Qt::white;
    }

    painter.setPen(QPen(borderColor, 1.5));
    painter.setBrush(bgColor);
    painter.drawRoundedRect(boxRect, 3, 3);
}

void CheckBox::drawCheckMark(QPainter &painter) {
    if (!isChecked())
        return;

    int boxSize = 18;
    int margin = 2;
    QRect boxRect(margin, (height() - boxSize) / 2, boxSize, boxSize);

    QColor checkColor = isEnabled() ? Qt::white : QColor("#999999");
    painter.setPen(QPen(checkColor, 2));

    QPainterPath path;
    path.moveTo(boxRect.left() + 4, boxRect.center().y());
    path.lineTo(boxRect.left() + boxSize / 3, boxRect.bottom() - 4);
    path.lineTo(boxRect.right() - 4, boxRect.top() + 4);

    painter.drawPath(path);
}

void CheckBox::drawText(QPainter &painter) {
    if (text().isEmpty())
        return;

    int boxSize = 18;
    int textLeft = boxSize + 8;

    painter.setPen(palette().windowText().color());
    // painter.setFont(font());

    QRect textRect(textLeft, 0, width() - textLeft, height());
    painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text());
}

void CheckBox::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    drawCheckBoxBackground(painter);

    if (isChecked()) {
        drawCheckMark(painter);
    }

    drawText(painter);
}
