#include "textedit.h"
#include <QPainterPath>
#include <QDebug>

TextEdit::TextEdit(QWidget* parent)
    : QTextEdit(parent)
    , m_borderColor("#d9d9d9") // 默认灰色边框
    , m_borderWidth(1)         // 边框宽度
    , m_cornerRadius(5)        // 圆角
    , m_padding(5)             // 内容与边框的距离
{
    this->setStyleSheet("QTextEdit { border: 2px solid #cccccc; border-radius: 10px; }");

    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 1. 禁用系统自带边框，防止画两层
    this->setFrameShape(QFrame::NoFrame);

    // 2. 【核心】设置视口（写字的地方）为透明
    // 这样我们才能透过文字看到下面画的圆角背景
    this->viewport()->setAutoFillBackground(false);

    // 3. 设置最小高度
    this->setMinimumHeight(80);

    // 4. 设置文字颜色（防止默认是白色导致看不见）
    QPalette pal = this->palette();
    pal.setColor(QPalette::Text, QColor("#222222"));
    this->setPalette(pal);

    // 5. 初始化边距
    updateViewportMargins();
}

TextEdit::TextEdit(const QString &text, QWidget *parent)
    : TextEdit(parent) {
    this->setText(text);
}

void TextEdit::setBorderColor(const QColor& color) {
    m_borderColor = color;
    update();
}

void TextEdit::setCornerRadius(int radius) {
    m_cornerRadius = radius;
    update();
}

// 核心：设置视口边距
// 这决定了文字区域相对于控件边缘缩进多少
// 如果不设置，文字会压在边框上，或者遮挡边框
void TextEdit::updateViewportMargins() {
    // 左右上下都留出：边框宽度 + padding
    int m = m_borderWidth + m_padding;
    this->setViewportMargins(m, m, m, m);
}

void TextEdit::resizeEvent(QResizeEvent* event) {
    QTextEdit::resizeEvent(event);
    updateViewportMargins();
}

void TextEdit::paintEvent(QPaintEvent* event) {
    // 【注意】不要调用 QTextEdit::paintEvent(event);
    // 因为我们要完全接管背景绘制，而且我们已经 setFrameShape(NoFrame) 了。
    // 如果你调用它，它可能会画出系统的方形背景覆盖掉你的圆角。

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 1. 准备绘制区域
    // 为了防止笔触画出界，通常需要内缩半个像素或1个像素
    QRectF rect = this->rect();
    rect.adjust(0.5, 0.5, -0.5, -0.5);

    // 2. 绘制圆角背景 (这也是为什么 viewport 必须透明)
    QColor bgColor;
    if (this->isEnabled()) {
        bgColor = this->hasFocus() ? Qt::white : QColor("#f0f0f0"); // 聚焦白底，失焦灰底
    } else {
        bgColor = QColor("#e0e0e0");
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(bgColor);
    painter.drawRoundedRect(rect, m_cornerRadius, m_cornerRadius);

    // 3. 绘制圆角边框
    // 聚焦时蓝色，失焦时灰色
    QColor borderColor = m_borderColor;

    // 如果想要聚焦时边框变色，可以在这里判断
    // if (hasFocus()) borderColor = QColor("#005a9e");

    QPen pen(borderColor);
    pen.setWidth(m_borderWidth);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush); // 边框只画线，不填充
    painter.drawRoundedRect(rect, m_cornerRadius, m_cornerRadius);

    // 4. 绘制底部的焦点指示条 (类似 LineEdit 的效果)
    if (m_borderWidth > 0) {
        painter.save();

        // 使用裁切路径，确保底部的线条不会画出圆角外面
        QPainterPath clipPath;
        clipPath.addRoundedRect(rect, m_cornerRadius, m_cornerRadius);
        painter.setClipPath(clipPath);

        // 线条颜色
        QColor lineColor = this->hasFocus() ? QColor("#005a9e") : QColor("#999999");
        int lineHeight = 2;

        // 计算线条位置：在控件最底部
        QRectF lineRect(rect.x(), rect.height() - lineHeight, rect.width(), lineHeight);

        painter.setPen(Qt::NoPen);
        painter.setBrush(lineColor);
        painter.drawRect(lineRect);

        painter.restore();
    }

    // 【关于文字显示】
    // 文字不是由 QTextEdit::paintEvent 画的，而是由 viewport 的 paintEvent 画的。
    // 只要 viewport 是透明的，并且我们上面画好了背景，文字就会自然浮现在上面。
    QTextEdit::paintEvent(event);
}

void TextEdit::focusInEvent(QFocusEvent* event) {
    QTextEdit::focusInEvent(event); // 必须调用，否则无法输入文字
    update(); // 触发重绘，改变边框/背景颜色
}

void TextEdit::focusOutEvent(QFocusEvent* event) {
    QTextEdit::focusOutEvent(event); // 必须调用
    update(); // 触发重绘
}
