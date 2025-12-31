#include "selectfolderwidget.h"
#include "utils/fileutils.h"

SelectFolderWidget::SelectFolderWidget(QWidget *parent) : WidgetBase{parent} {
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    this->setMinimumSize(300, 100);
    this->setMaximumSize(300, 150);

    this->createComponents();
}

void SelectFolderWidget::createComponents() {
    auto selectButton = new NormalButton("选择目录", this);
    selectButton->setFixedWidth(100);

    auto batchProcessButton = new NormalButton("批量处理", this);
    batchProcessButton->setFixedWidth(100);
    batchProcessButton->setEnabled(false);

    auto openButton = new NormalButton("打开目录", this);
    openButton->setFixedWidth(100);
    openButton->setEnabled(false);

    TextEdit* textEdit = new TextEdit(this);
    textEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    textEdit->setPlaceholderText(FileUtils::getImageFileFilter());
    connect(textEdit, &QTextEdit::textChanged, this, [=](){
        auto folderPath = textEdit->toPlainText().trimmed();
        QFileInfo info(folderPath);
        if (!info.exists()) {
            batchProcessButton->setEnabled(false);
            openButton->setEnabled(false);
        } else {
            batchProcessButton->setEnabled(true);
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
    connect(batchProcessButton, &QPushButton::clicked, this, [=]() {
        QString filePath = textEdit->toPlainText().trimmed();
        QDir dir(filePath);
        QString absolutePath = dir.absolutePath();

        QString processFolder = absolutePath + QDir::separator() + "_result";

        auto fileList = FileUtils::findAllImageFiles(filePath, false);

        qDebug() << "fileList.size" << fileList.size();

        for (int i = 0; i < fileList.size(); ++i) {

        }

        FileUtils::showInFolder(processFolder);
    });
    connect(openButton, &QPushButton::clicked, this, [=]() {
        QString filePath = textEdit->toPlainText().trimmed();
        FileUtils::showInFolder(filePath);
    });

    Layouting::Column{Layouting::Row{selectButton, batchProcessButton, openButton}, Layouting::Row{textEdit}}.attachTo(this);

}
