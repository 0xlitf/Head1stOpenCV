#include "roundedwidget.h"

RoundedWidget::RoundedWidget(QWidget *parent) : QWidget(parent)
{
    // 设置鼠标跟踪以启用悬停事件
    setMouseTracking(true);

    // 设置背景透明，以便自定义绘制
    setAttribute(Qt::WA_StyledBackground, true);
}

void RoundedWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 抗锯齿

    // 根据状态选择边框颜色
    QColor borderColor = m_normalColor;
    if (m_isPressed) {
        borderColor = m_pressedColor;
    } else if (m_isHovered) {
        borderColor = m_hoverColor;
    }

    // 创建圆角矩形路径
    QPainterPath path;
    QRect rect = this->rect().adjusted(1, 1, -1, -1); // 留出边框空间
    path.addRoundedRect(rect, m_borderRadius, m_borderRadius);

    // 填充背景
    painter.fillPath(path, m_backgroundColor);

    // 绘制边框
    if (m_borderWidth > 0) {
        QPen pen(borderColor, m_borderWidth);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(path);
    }
}

void RoundedWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isPressed = true;
        update(); // 触发重绘
        event->accept();
    } else {
        QWidget::mousePressEvent(event);
    }
}

void RoundedWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_isPressed) {
        m_isPressed = false;
        update(); // 触发重绘
        this->setFocus(Qt::MouseFocusReason);

        // 检查鼠标是否仍在控件内，如果是则发射点击信号
        if (rect().contains(event->pos())) {
            emit clicked();
        }
        event->accept();
    } else {
        QWidget::mouseReleaseEvent(event);
    }
}

void RoundedWidget::enterEvent(QEvent *event)
{
    m_isHovered = true;
    update(); // 触发重绘以更新颜色
    event->accept();
}

void RoundedWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    m_isHovered = false;
    m_isPressed = false; // 离开时重置按下状态
    update(); // 触发重绘以更新颜色
}

// 外观设置函数实现
void RoundedWidget::setBorderRadius(int radius)
{
    m_borderRadius = radius;
    update();
}

void RoundedWidget::setBorderWidth(int width)
{
    m_borderWidth = width;
    update();
}

void RoundedWidget::setNormalColor(const QColor &color)
{
    m_normalColor = color;
    update();
}

void RoundedWidget::setHoverColor(const QColor &color)
{
    m_hoverColor = color;
    update();
}

void RoundedWidget::setPressedColor(const QColor &color)
{
    m_pressedColor = color;
    update();
}

void RoundedWidget::setBackgroundColor(const QColor &color)
{
    m_backgroundColor = color;
    update();
}
