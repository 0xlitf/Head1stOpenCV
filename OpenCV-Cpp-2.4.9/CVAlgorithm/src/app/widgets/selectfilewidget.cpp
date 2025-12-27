#include "selectfilewidget.h"
#include "utils/fileutils.h"

SelectFileWidget::SelectFileWidget(QWidget *parent) : WidgetBase{parent} {
    this->setMinimumSize(300, 100);
    this->setMaximumSize(300, 150);

    this->createComponents();
}

void SelectFileWidget::createComponents() {
    auto selectButton = new NormalButton("选择文件", this);
    selectButton->setFixedWidth(100);
    auto openButton = new NormalButton("打开", this);
    openButton->setFixedWidth(100);

    TextEdit* textEdit = new TextEdit(this);
    textEdit->setPlaceholderText("选择图片");

    connect(selectButton, &QPushButton::clicked, this, [=]() {
        auto folderName = FileUtils::selectFileDialog();
        if (!folderName.isEmpty()) {
            textEdit->setText(folderName);
        }
    });

    Layouting::Column{Layouting::Row{selectButton, openButton}, Layouting::Row{textEdit}}.attachTo(this);

}
