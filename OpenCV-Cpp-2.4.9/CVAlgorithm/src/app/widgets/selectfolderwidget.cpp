#include "selectfolderwidget.h"
#include "utils/fileutils.h"

SelectFolderWidget::SelectFolderWidget(QWidget *parent) : WidgetBase{parent} {
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    this->setMinimumSize(300, 100);
    this->setMaximumSize(300, 120);

    this->createComponents();
}

void SelectFolderWidget::createComponents() {
    auto selectButton = new NormalButton("选择目录", this);
    selectButton->setFixedWidth(100);

    auto openButton = new NormalButton("打开目录", this);
    openButton->setFixedWidth(100);
    openButton->setEnabled(false);

    m_textEdit = new TextEdit(this);
    m_textEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_textEdit->setPlaceholderText(FileUtils::getImageFileFilter());
    connect(m_textEdit, &QTextEdit::textChanged, this, [=]() {
        auto folderPath = m_textEdit->toPlainText().trimmed();
        QFileInfo info(folderPath);
        if (!info.exists()) {
            openButton->setEnabled(false);
        } else {
            openButton->setEnabled(true);
        }
        emit this->folderChanged(folderPath);
    });

    connect(selectButton, &QPushButton::clicked, this, [=]() {
        auto folderName = FileUtils::selectFolderDialog();
        if (!folderName.isEmpty()) {
            m_textEdit->setText(folderName);
        }
    });
    connect(openButton, &QPushButton::clicked, this, [=]() {
        QString filePath = m_textEdit->toPlainText().trimmed();
        FileUtils::showInFolder(filePath);
    });

    Layouting::Column{Layouting::Row{selectButton, openButton}, Layouting::Row{m_textEdit}}.attachTo(this);
}
