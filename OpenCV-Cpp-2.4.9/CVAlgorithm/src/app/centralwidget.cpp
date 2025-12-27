#include "centralwidget.h"
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
#include "controls/controls.h"
#include "controls/pagetabbutton.h"
#include "utils/fileutils.h"
#include "pages/cutoutobjectpage.h"
#include "pages/humomentspage.h"

CentralWidget::CentralWidget(QWidget *parent) : WidgetBase{parent} {
    this->setRandomColor();
    this->createComponents();

    CutoutObjectPage *cutoutPage = new CutoutObjectPage();
    HuMomentsPage *huPage = new HuMomentsPage();

    m_stackedWidget->addWidget(cutoutPage);
    m_stackedWidget->addWidget(huPage);
}

void CentralWidget::createComponents() {
    m_buttonStringList << tr("1") << tr("2");

    m_stackedWidget = new QStackedWidget(this);
    m_stackedWidget->setContentsMargins(5, 5, 5, 5);

    auto createButton =
        [this](const QString &text, QWidget *parent, QStackedWidget *stacked,
               QButtonGroup *group, int index) {
        auto w = new PageTabButton(parent);
        w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        w->setText(text);
        w->setCheckable(true);
        w->setFixedWidth(180);
        group->addButton(w);
        connect(w, &QAbstractButton::clicked, this, [=]() {
            qDebug() << "index" << index;
            stacked->setCurrentIndex(index);
        });
        return w;
        };

    auto m_centralTop = [=]() {
        WidgetBase *centralTop = new WidgetBase();
        centralTop->setBackgroundColor(QColor("#0078d7"));
        centralTop->setFixedHeight(50);
        centralTop->setContentsMargins(0, 0, 0, 0);
        QButtonGroup *group = new QButtonGroup(centralTop);

        auto m_topContainer = new QHBoxLayout(centralTop);
        m_topContainer->setContentsMargins(0, 0, 0, 0);
        m_topContainer->setSpacing(0);
        m_topButtonRow = new QHBoxLayout();
        m_topButtonRow->setContentsMargins(0, 0, 0, 0);
        m_topButtonRow->setSpacing(0);

        m_topContainer->addLayout(m_topButtonRow);

        for (int i = 0; i < m_buttonStringList.length(); ++i) {
            m_topButtonRow->addWidget([&]() {
                auto button = createButton(QString(m_buttonStringList[i]), centralTop,
                                           m_stackedWidget, group, i);
                m_buttonList.append(button);

                return button;
            }());
        }
        m_topContainer->addStretch();
        return centralTop;
    }();

    auto m_centralCenter = [=]() {
        WidgetBase *centralCenter = new WidgetBase();
        centralCenter->setBackgroundColor(QColor("#215a94"));

        Layouting::Column{m_stackedWidget}.attachTo(centralCenter);

        return centralCenter;
    }();

    Layouting::Column{m_centralTop, m_centralCenter}.attachTo(this);

    if (m_buttonList.size() > 1) {
        m_buttonList[0]->setChecked(true);
        m_stackedWidget->setCurrentIndex(0);
    }

    auto initIndex = 0;
    if (initIndex >= m_buttonList.length()) {
        QMessageBox::warning(nullptr, "Array overreach", QString("Array overreach at buttonList"));
    } else {
        m_buttonList[initIndex]->setChecked(true);
        m_stackedWidget->setCurrentIndex(initIndex);
    }
}
