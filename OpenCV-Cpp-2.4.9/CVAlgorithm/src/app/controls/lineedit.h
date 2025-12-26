#ifndef LINEEDIT_H
#define LINEEDIT_H

#pragma execution_character_set("utf-8")

#include <QAction>
#include <QLineEdit>
#include <QDebug>

class LineEdit : public QLineEdit {
    Q_OBJECT
public:
    explicit LineEdit(QWidget *parent = nullptr);
    explicit LineEdit(const QString& str, QWidget *parent = nullptr);

    void setBorderColor(const QColor& color);
    void setBorderWidth(int width);
    void setCornerRadius(int radius);

    void setupActions();
protected:
    void paintEvent(QPaintEvent* event) override;

    QSize sizeHint() const override {
        static QSize size = QLineEdit::sizeHint();
        return size + QSize(10, 10);
    }

    void focusInEvent(QFocusEvent* event) override {
        QLineEdit::focusInEvent(event);
        updateActionVisibility();
    }

    void focusOutEvent(QFocusEvent* event) override {
        QLineEdit::focusOutEvent(event);
        updateActionVisibility();
    }

private slots:
    void updateActionVisibility() {
        if (this->hasFocus()) {
            if (this->actions().contains(m_settingsAction)) {

            } else {
                this->addAction(m_settingsAction, QLineEdit::TrailingPosition);
            }
        } else {
            this->removeAction(m_settingsAction);
        }
    }

private:
    QAction* m_settingsAction;
    int m_spacing = 3;
    QColor m_borderColor = Qt::gray;
    int m_borderWidth = 1;
    int m_cornerRadius = 5;
    bool m_hasError = false;
    void drawFocusIndicator(QPainter& painter);
};

#endif // LINEEDIT_H
