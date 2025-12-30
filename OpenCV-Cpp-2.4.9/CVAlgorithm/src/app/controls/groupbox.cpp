#include "groupbox.h"

void GroupBox::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QStyleOptionGroupBox option;
    initStyleOption(&option);

    // 1. 获取字体度量，计算标题文字的大小
    QFontMetrics fm(option.fontMetrics);
    QRect textRect = style()->subControlRect(QStyle::CC_GroupBox, &option, QStyle::SC_GroupBoxLabel, this);

    // 如果样式没有返回正确的 label 区域（有时发生），手动微调
    if (textRect.isEmpty() && !option.text.isEmpty()) {
        QSize textSize = fm.size(0, option.text);
        textRect = QRect(10, 0, textSize.width() + 4, textSize.height());
    }

    // 2. 计算边框区域
    // 通常要向下偏移一点，让出标题的一半高度
    int topMargin = 0;
    if (!option.text.isEmpty()) {
        topMargin = fm.height() / 2;
    }
    QRect frameRect = option.rect.adjusted(0, topMargin, 0, 0);

    // 3. 设置绘制属性
    QColor borderColor("#005a9e"); // 蓝色
    int borderWidth = 2;
    int cornerRadius = 6;

    // 4. 关键：处理遮罩 (Clipping)
    // 我们不希望边框线横穿文字，所以要在绘制边框前，把文字区域从绘制区域中扣除
    painter.save();
    if (!option.text.isEmpty()) {
        QRegion region(option.rect);
        // 稍微扩大一点 textRect，让线离字远一点点
        region -= QRegion(textRect.adjusted(-2, 0, 2, 0));
        painter.setClipRegion(region);
    }

    // 5. 绘制蓝色圆角边框
    QPen pen(borderColor);
    pen.setWidth(borderWidth);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    // 由于画笔宽度占空间，rect 需要向内缩半个画笔宽，否则边缘会被切掉
    QRectF drawRect = QRectF(frameRect).adjusted(borderWidth / 2.0, borderWidth / 2.0, -borderWidth / 2.0, -borderWidth / 2.0);
    painter.drawRoundedRect(drawRect, cornerRadius, cornerRadius);

    // 6. 恢复 Clipping，准备画文字
    painter.restore();

    // 7. 绘制标题文字
    if (!option.text.isEmpty()) {
        // 使用系统样式颜色或者强制蓝色
        painter.setPen(borderColor); // 文字也用蓝色
        // painter.setPen(option.palette.color(QPalette::WindowText)); // 或者用默认黑色

        // 绘制文字时要注意对齐
        int alignment = Qt::AlignLeft | Qt::AlignTop;
        // 简单的绘制，如果包含 CheckBox 需要更复杂的计算
        painter.drawText(textRect, alignment, option.text);
    }

    // 注意：这里没有处理 Checkable GroupBox 的 Checkbox 绘制
    // 如果需要 Checkbox，还得调用 style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, ...)
}
