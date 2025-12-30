#include "selectfilewidget.h"
#include "utils/fileutils.h"

SelectFileWidget::SelectFileWidget(QWidget *parent) : WidgetBase{parent} {
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    this->setBackgroundColor(QColor(233, 233, 233));
    this->setMinimumSize(300, 100);
    this->setMaximumSize(300, 150);

    this->createComponents();
}

void SelectFileWidget::createComponents() {
    auto selectButton = new NormalButton("选择图片", this);
    selectButton->setFixedWidth(100);
    auto openButton = new NormalButton("打开目录", this);
    openButton->setFixedWidth(100);
    openButton->setEnabled(false);

    TextEdit* textEdit = new TextEdit(this);
    textEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    textEdit->setPlaceholderText(FileUtils::getImageFileFilter());
    connect(textEdit, &QTextEdit::textChanged, this, [=](){
        auto filePath = textEdit->toPlainText().trimmed();
        QFileInfo info(filePath);
        if (!info.exists()) {
            openButton->setEnabled(false);
        } else {
            openButton->setEnabled(true);
        }
        emit this->fileChanged(filePath);
    });

    connect(selectButton, &QPushButton::clicked, this, [=]() {
        auto folderName = FileUtils::selectFileDialog();
        if (!folderName.isEmpty()) {
            textEdit->setText(folderName);
        }
    });
    connect(openButton, &QPushButton::clicked, this, [=]() {
        QString filePath = textEdit->toPlainText().trimmed();
        FileUtils::showInFolder(filePath);
    });

    Layouting::Column{Layouting::Row{selectButton, openButton}, Layouting::Row{textEdit}}.attachTo(this);

    // QVBoxLayout* v = new QVBoxLayout(this);
    // QHBoxLayout* h1 = new QHBoxLayout;
    // QHBoxLayout* h2 = new QHBoxLayout;

    // v->addLayout(h1);
    // v->addLayout(h2);

    // h1->addWidget(selectButton);
    // h1->addWidget(openButton);
    // h2->addWidget(textEdit);
}
