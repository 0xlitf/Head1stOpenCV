#ifndef NORMALBUTTON_H
#define NORMALBUTTON_H

#pragma execution_character_set("utf-8")

#include <QPainter>
#include <QPushButton>

class NormalButton : public QPushButton {
    Q_OBJECT
public:
    explicit NormalButton(QWidget *parent = nullptr);
    explicit NormalButton(const QString& str, QWidget *parent = nullptr);

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
        updateAppearance();
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
            if (m_isPressed) {
                return m_checkColor.lighter(110);
            }
            if (m_isHovered) {
                return m_checkColor.darker(85);
            }
            return m_checkColor;
        } else {
            if (m_isPressed) {
                return m_pressColor;
            }
            if (m_isHovered) {
                return m_hoverColor;
            }
            return m_normalColor;
        }
    }

    QColor getTextColor() const {
        if (!isEnabled()) {
            return m_textDisableColor;
        }
        if (isChecked()) {
            if (m_isPressed) {
                return QColor(Qt::white).darker(110);
            }
            if (m_isHovered) {
                return QColor(Qt::white).darker(110);
            }
            return Qt::white;
        } else {
            if (m_isPressed) {
                return m_textNormalColor;
            }
            if (m_isHovered) {
                return m_textHoverColor;
            }
            return m_textNormalColor;
        }
    }

    void updateAppearance() { update(); }

    void drawOuterBorder(QPainter &painter, const QRect &rect);

    void drawContentArea(QPainter &painter, const QRect &rect, const QColor &fillColor);

    void drawText(QPainter &painter, const QRect &rect);

    QSize sizeHint() const override {
        QSize size = QPushButton::sizeHint();
        return size + QSize(m_spacing * 2, 0);
    }

    QColor m_normalColor = QColor("#fbfbfb");
    QColor m_hoverColor = QColor("#f6f6f6");
    QColor m_pressColor = QColor("#c5c5c5");
    QColor m_checkColor = QColor("#005a9e");
    QColor m_disabledColor = QColor("#f5f5f5");

    QColor m_textNormalColor = QColor("#1b1b1b");
    QColor m_textPressColor = QColor("#5d5d5d");
    QColor m_textDisableColor = QColor("#a9a9a9");
    QColor m_textHoverColor = QColor("#1b1b1b");

    bool m_isHovered = false;
    bool m_isPressed = false;
    int m_spacing = 3;
    int m_cornerRadius = 5;
    QColor m_borderColor = Qt::gray;
};

#endif // NORMALBUTTON_H
