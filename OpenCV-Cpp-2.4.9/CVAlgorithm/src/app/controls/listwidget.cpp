#include "listwidget.h"

ListWidget::ListWidget() {}

void ListWidget::focusOutEvent(QFocusEvent *event)
{
    // 先调用基类的处理，确保默认行为正常
    QListWidget::focusOutEvent(event);

    // 失去焦点时，清空当前选中项
    this->setCurrentRow(-1); // 将当前行设置为-1
    // 或者也可以使用 this->clearSelection(); 来清除所有选中项，但setCurrentRow(-1)通常更直接。
}
