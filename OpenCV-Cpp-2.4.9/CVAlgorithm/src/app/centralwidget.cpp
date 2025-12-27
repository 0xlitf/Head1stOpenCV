#include "centralwidget.h"
#include "controls/controls.h"
#include "controls/pagetabbutton.h"
#include "pages/cutoutobjectpage.h"
#include "pages/humomentspage.h"
#include "utils/fileutils.h"
#include <QButtonGroup>
#include <QFile>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QMetaEnum>
#include <QPainter>
#include <QRadioButton>
#include <QSizePolicy>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStringList>
#include <QVBoxLayout>

CentralWidget::CentralWidget(QWidget *parent) : WidgetBase{parent} {
    this->createComponents();

    this->addPage("CutoutObject", []{
        return new CutoutObjectPage();
    }());
    this->addPage("HuMoments", []{
        return new HuMomentsPage();
    }());

    // 默认选中界面
    this->setDefaultPageIndex(0);
}

void CentralWidget::setDefaultPageIndex(int i) {
    if (i < m_buttonList.size() && i < m_stackedWidget->count()) {
        m_buttonList[i]->setChecked(true);
        m_stackedWidget->setCurrentIndex(i);
    } else {
        qWarning()
            << "false: i < m_buttonList.size() && i < m_stackedWidget->count()";
    }
}

void CentralWidget::createTopButton(const QString &text) {
    auto w = new PageTabButton();
    w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    w->setText(text);
    w->setCheckable(true);
    w->setFixedWidth(180);
    m_buttonGroup->addButton(w);
    int index = m_buttonList.count();
    connect(w, &QAbstractButton::clicked, this, [=]() {
        qDebug() << "TopButton clicked index" << index;
        m_stackedWidget->setCurrentIndex(index);
    });

    if (m_buttonList.isEmpty()) {
        w->setChecked(true);
    }

    m_buttonList.append(w);
    m_topButtonRow->addWidget(w);
}

void CentralWidget::addPage(const QString &text, QWidget *w) {
    this->createTopButton(text);
    m_stackedWidget->addWidget(w);
}

void CentralWidget::createComponents() {
    m_stackedWidget = new QStackedWidget();
    // m_stackedWidget->setContentsMargins(5, 5, 5, 5);

    auto m_centralTop = [=]() {
        WidgetBase *centralTop = new WidgetBase();
        centralTop->setBackgroundColor(QColor("#0078d7"));
        centralTop->setFixedHeight(50);
        centralTop->setContentsMargins(0, 0, 0, 0);
        m_buttonGroup = new QButtonGroup(centralTop);

        auto m_topContainer = new QHBoxLayout(centralTop);
        m_topContainer->setContentsMargins(0, 0, 0, 0);
        m_topContainer->setSpacing(0);
        m_topButtonRow = new QHBoxLayout();
        m_topButtonRow->setContentsMargins(0, 0, 0, 0);
        m_topButtonRow->setSpacing(0);

        m_topContainer->addLayout(m_topButtonRow);

        m_topContainer->addStretch();
        return centralTop;
    }();

    auto m_centralCenter = [=]() {
        WidgetBase *centralCenter = new WidgetBase();
        centralCenter->setBackgroundColor(QColor("#215a94"));
        centralCenter->setContentsMargins(5, 5, 5, 5);

        Layouting::Column{m_stackedWidget}.attachTo(centralCenter);

        return centralCenter;
    }();

    Layouting::Column{m_centralTop, m_centralCenter}.attachTo(this);
}
