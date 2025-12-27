#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QTextEdit>
#include <QPainter>
#include <QPainterPath>
#include <QEvent>

class TextEdit : public QTextEdit {
    Q_OBJECT

public:
    explicit TextEdit(QWidget* parent = nullptr);
    explicit TextEdit(const QString& text, QWidget* parent = nullptr);

    // 设置边框颜色
    void setBorderColor(const QColor& color);
    // 设置边框宽度
    void setBorderWidth(int width);
    // 设置圆角半径
    void setCornerRadius(int radius);
    // 设置内部间距 (控制文字距离边框的距离)
    void setCustomPadding(int padding);

protected:
    // 重写绘制事件：画背景、边框、Focus条
    void paintEvent(QPaintEvent* event) override;

    // 焦点进入/离开时刷新界面（改变边框/背景颜色）
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private:
    void drawFocusIndicator(QPainter& painter);
    void updateViewportMargins();

    QColor m_borderColor;
    int m_borderWidth;
    int m_cornerRadius;
    int m_spacing; // 用于控制整体绘制区域收缩
    int m_padding; // 文字内容的内边距
};

#endif // TEXTEDIT_H
