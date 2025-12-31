#ifndef ROUNDEDWIDGET_H
#define ROUNDEDWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QVBoxLayout>

class RoundedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RoundedWidget(QWidget *parent = nullptr);

    // 外观设置函数
    void setBorderRadius(int radius);
    void setBorderWidth(int width);
    void setNormalColor(const QColor &color);
    void setHoverColor(const QColor &color);
    void setPressedColor(const QColor &color);
    void setBackgroundColor(const QColor &color);

signals:
    void clicked(); // 类似QPushButton的点击信号

protected:
    // 重写绘制事件
    void paintEvent(QPaintEvent *event) override;

    // 重写鼠标事件
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    // void enterEvent(QEnterEvent *event) override;
    void enterEvent(QEvent *event) override; // 注意参数类型是 QEvent*
    void leaveEvent(QEvent *event) override;

private:
    // 外观属性
    int m_borderRadius = 5;
    int m_borderWidth = 1;
    QColor m_normalColor = QColor(100, 100, 100);    // 默认灰色边框
    QColor m_hoverColor = QColor(66, 133, 244);      // 默认蓝色悬浮
    QColor m_pressedColor = QColor(49, 105, 198);    // 默认深蓝点击
    QColor m_backgroundColor = QColor(240, 240, 240);// 默认浅灰背景

    // 状态标志
    bool m_isHovered = false;
    bool m_isPressed = false;
};

#endif // ROUNDEDWIDGET_H
