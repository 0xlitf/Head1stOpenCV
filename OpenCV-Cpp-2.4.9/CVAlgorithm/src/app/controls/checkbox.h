#ifndef CHECKBOX_H
#define CHECKBOX_H

#pragma execution_character_set("utf-8")

#include <QCheckBox>
#include <QString>
#include <QPainter>

class CheckBox : public QCheckBox {
    Q_OBJECT
public:
    explicit CheckBox(QWidget *parent = nullptr);
    explicit CheckBox(const QString &text, QWidget *parent = nullptr);

private:
    void drawCheckBoxBackground(QPainter& painter);

    void drawCheckMark(QPainter& painter);

    void drawText(QPainter& painter);
protected:
    void paintEvent(QPaintEvent* event) override;

    QSize sizeHint() const override {
        int boxSize = 20;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        int textWidth = fontMetrics().horizontalAdvance(text());
#else
        int textWidth = fontMetrics().width(text());
#endif
        return QSize(boxSize + textWidth + 10, qMax(boxSize, fontMetrics().height()));
    }

signals:
};

#endif // CHECKBOX_H
