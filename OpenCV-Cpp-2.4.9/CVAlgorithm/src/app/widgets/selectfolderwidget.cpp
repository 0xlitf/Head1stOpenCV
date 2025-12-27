#include "selectfolderwidget.h"
#include "utils/fileutils.h"

SelectFolderWidget::SelectFolderWidget(QWidget *parent) : WidgetBase{parent} {
    this->setMinimumSize(300, 100);
    this->setMaximumSize(300, 150);

    this->createComponents();
}

void SelectFolderWidget::createComponents() {
    auto selectButton = new NormalButton("选择目录", this);
    selectButton->setFixedWidth(100);
    auto openButton = new NormalButton("打开目录", this);
    openButton->setFixedWidth(100);
    openButton->setEnabled(false);

    TextEdit* textEdit = new TextEdit(this);
    textEdit->setPlaceholderText(FileUtils::getImageFileFilter());
    connect(textEdit, &QTextEdit::textChanged, this, [=](){
        auto folderPath = textEdit->toPlainText().trimmed();
        QFileInfo info(folderPath);
        if (!info.exists()) {
            openButton->setEnabled(false);
        } else {
            openButton->setEnabled(true);
            emit this->folderChanged(folderPath);
        }
    });

    connect(selectButton, &QPushButton::clicked, this, [=]() {
        auto folderName = FileUtils::selectFolderDialog();
        if (!folderName.isEmpty()) {
            textEdit->setText(folderName);
        }
    });
    connect(openButton, &QPushButton::clicked, this, [=]() {
        QString filePath = textEdit->toPlainText().trimmed();
        FileUtils::showInFolder(filePath);
    });

    Layouting::Column{Layouting::Row{selectButton, openButton}, Layouting::Row{textEdit}}.attachTo(this);

}
