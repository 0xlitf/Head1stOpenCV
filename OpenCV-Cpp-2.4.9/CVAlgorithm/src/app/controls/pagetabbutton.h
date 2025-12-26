#ifndef PAGETABBUTTON_H
#define PAGETABBUTTON_H

#pragma execution_character_set("utf-8")

#include <QPainter>
#include <QPushButton>

class PageTabButton : public QPushButton {
    Q_OBJECT
public:
    explicit PageTabButton(QWidget *parent = nullptr);

    void setNormalColor(const QColor &color) {
        m_normalColor = color;
        this->updateAppearance();
    }

    void setHoverColor(const QColor &color) {
        m_hoverColor = color;
        this->updateAppearance();
    }

    void setPressColor(const QColor &color) {
        m_pressColor = color;
        this->updateAppearance();
    }

    void setCheckColor(const QColor &color) {
        m_checkColor = color;
        this->updateAppearance();
    }

protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override {
#else
    void enterEvent(QEvent *event) override {
#endif
        QPushButton::enterEvent(event);
        m_isHovered = true;
        this->updateAppearance();
    }

    void leaveEvent(QEvent *event) override {
        QPushButton::leaveEvent(event);
        m_isHovered = false;
        this->updateAppearance();
    }

    void mousePressEvent(QMouseEvent *event) override {
        QPushButton::mousePressEvent(event);
        m_isPressed = true;
        this->updateAppearance();
    }

    void mouseReleaseEvent(QMouseEvent *event) override {
        QPushButton::mouseReleaseEvent(event);
        m_isPressed = false;
        this->updateAppearance();
    }

    void paintEvent(QPaintEvent *event) override;

private:
    QColor getCurrentColor() const {
        if (!isEnabled()) {
            return m_disabledColor;
        }
        if (isChecked()) {
            return m_checkColor;
        }
        if (m_isPressed) {
            return m_pressColor;
        }
        if (m_isHovered) {
            return m_hoverColor;
        }
        return m_normalColor;
    }

    QColor getTextColor() const {
        return Qt::white;
    }

    void updateAppearance() {
        update();
    }

    void drawContentArea(QPainter &painter, const QRect &rect, const QColor &fillColor);

    void drawText(QPainter &painter, const QRect &rect);

    QColor m_normalColor = QColor("#3498db");
    QColor m_hoverColor = QColor("#2980b9");
    QColor m_pressColor = QColor("#21618c");
    QColor m_checkColor = QColor("#215a94");
    QColor m_disabledColor = QColor("#bdc3c7");

    bool m_isHovered = false;
    bool m_isPressed = false;
    int m_spacing = 5;
    int m_cornerRadius = 8;
};

#endif // PAGETABBUTTON_H
