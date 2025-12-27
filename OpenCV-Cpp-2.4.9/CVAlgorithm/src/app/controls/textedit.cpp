#include "textedit.h"
#include <QDebug>
TextEdit::TextEdit(QWidget* parent)
    : QTextEdit(parent)
    , m_borderColor("#d9d9d9")
    , m_borderWidth(1)
    , m_cornerRadius(5)
    , m_spacing(0)
    , m_padding(5)
{
    // 1. 去除系统自带的边框
    this->setFrameShape(QFrame::NoFrame);

    // 2. 【关键修正】设置调色板
    // 将 Base (基础背景色) 设为透明，否则 Viewport 会用白色填满矩形，挡住圆角
    QPalette pal = this->palette();
    pal.setColor(QPalette::Base, Qt::transparent);
    pal.setColor(QPalette::Text, QColor("#222222")); // 确保文字是深色的，防止白字在白底上看不见
    this->setPalette(pal);

    // 3. 【关键修正】告诉 Viewport 它是透明的
    // 这样它才不会遮挡我们在 paintEvent 里画的背景
    this->viewport()->setAutoFillBackground(false);

    // 4. 设置最小高度
    this->setMinimumHeight(80);

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

void TextEdit::setBorderWidth(int width) {
    m_borderWidth = width;
    updateViewportMargins(); // 边框变宽时，可能需要调整文字边距
    update();
}

void TextEdit::setCornerRadius(int radius) {
    m_cornerRadius = radius;
    update();
}

void TextEdit::setCustomPadding(int padding) {
    m_padding = padding;
    updateViewportMargins();
}

void TextEdit::updateViewportMargins() {
    // 设置 Viewport 的外边距，给边框和圆角留出空间
    // 计算公式：边框宽度 + 自定义内边距
    int totalMargin = m_borderWidth + m_padding;
    // 参数顺序: left, top, right, bottom
    this->setViewportMargins(totalMargin, totalMargin, totalMargin, totalMargin);
}

void TextEdit::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 计算绘制区域（逻辑参考你的 LineEdit）
    // innerRect 是整个控件减去 m_spacing 后的区域
    QRect innerRect = this->rect().adjusted(m_spacing, m_spacing, -m_spacing, -m_spacing);

    // 设置画笔（边框）
    QPen borderPen(m_borderColor, m_borderWidth);
    painter.setPen(borderPen);

    // 设置画刷（背景色）
    if (this->hasFocus()) {
        painter.setBrush(Qt::white);
    } else {
        // 未获取焦点时，背景稍微变暗，模拟 LineEdit 效果
        painter.setBrush(QColor("#ffffff").darker(105));
    }

    // 绘制圆角矩形背景和边框
    // 注意：在 QTextEdit 中，因为 Viewport 透明，这个背景会显示在文字下方
    painter.drawRoundedRect(innerRect, m_cornerRadius, m_cornerRadius);

    // 绘制底部的焦点指示条
    drawFocusIndicator(painter);

    // 注意：这里不需要调用 QTextEdit::paintEvent(event)
    // 因为 base 的 paintEvent 主要负责画 Frame（已禁用）和背景。
    // 文字绘制由 Viewport 自动处理，那是另一个 paintEvent，不受这里影响。

    QTextEdit::paintEvent(event);
}

void TextEdit::drawFocusIndicator(QPainter& painter) {
    // 复用你的 LineEdit 逻辑
    QRect innerRect = this->rect().adjusted(m_spacing, m_spacing, -m_spacing, 0);
    painter.save();

    // 创建裁切路径，确保底部的线不会超出圆角
    QPainterPath maskPath;
    QRect clipRect = this->rect().adjusted(m_spacing, m_spacing, -m_spacing, -m_spacing);
    maskPath.addRoundedRect(clipRect, m_cornerRadius, m_cornerRadius);

    painter.setClipPath(maskPath);

    QColor lineColor = this->hasFocus() ? QColor("#005a9e") : QColor("#999999");

    // 如果想要类似 Material Design 的效果，未聚焦时可以不画线，或者画细线
    // 这里保持和你代码一致：未聚焦画灰色，聚焦画蓝色

    int lineHeight = 2; // 线高
    int lineY = innerRect.height() - lineHeight - m_spacing; // 底部位置

    painter.setPen(Qt::NoPen);
    painter.setBrush(lineColor);

    // 绘制底部的条
    // 这里宽度稍微调整一下，确保贴合
    painter.drawRect(innerRect.x(), lineY, innerRect.width(), lineHeight);

    painter.restore();
}

void TextEdit::focusInEvent(QFocusEvent* event) {
    QTextEdit::focusInEvent(event);
    update(); // 触发重绘以改变背景色和底条颜色
}

void TextEdit::focusOutEvent(QFocusEvent* event) {
    QTextEdit::focusOutEvent(event);
    update(); // 触发重绘
}
