#ifndef CLICKABLEWIDGET_H
#define CLICKABLEWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QVBoxLayout>

class ClickableWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClickableWidget(QWidget *parent = nullptr);

    // 外观设置函数
    void setBorderRadius(int radius);
    void setBorderWidth(int width);
    void setNormalColor(const QColor &color);
    void setHoverColor(const QColor &color);
    void setPressedColor(const QColor &color);
    void setBackgroundColor(const QColor &color);

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    // void enterEvent(QEnterEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    int m_borderRadius = 5;
    int m_borderWidth = 1;
    QColor m_normalColor = QColor(100, 100, 100);
    QColor m_hoverColor = QColor(66, 133, 244);
    QColor m_pressedColor = QColor(49, 105, 198);
    QColor m_backgroundColor = QColor(240, 240, 240);

    bool m_isHovered = false;
    bool m_isPressed = false;
};

#endif // CLICKABLEWIDGET_H
