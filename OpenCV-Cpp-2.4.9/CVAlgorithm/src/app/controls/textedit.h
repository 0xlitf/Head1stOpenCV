#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QTextEdit>
#include <QPainter>
#include <QEvent>

class TextEdit : public QTextEdit {
    Q_OBJECT

public:
    explicit TextEdit(QWidget* parent = nullptr);
    explicit TextEdit(const QString& text, QWidget* parent = nullptr);

    void setBorderColor(const QColor& color);
    void setCornerRadius(int radius);

protected:
    // 我们在这里画背景和边框
    void paintEvent(QPaintEvent* event) override;

    // 焦点改变时重绘
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

    // 大小改变时调整视口边距
    void resizeEvent(QResizeEvent* event) override;

private:
    void updateViewportMargins();

    QColor m_borderColor;
    int m_borderWidth;
    int m_cornerRadius;
    // 文字内容距离边框的内边距
    int m_padding;
};

#endif // TEXTEDIT_H
