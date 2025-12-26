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

CentralWidget::CentralWidget(QWidget *parent) : WidgetBase{parent} {
    this->setRandomColor();
    this->createComponents();
}

void CentralWidget::createComponents() {
    m_buttonStringList = QStringList()
                         << tr("1")
        ;
    auto stack = Layouting::Stack{
        [this]() {
            WidgetBase *w = new WidgetBase();
            // w->setBackgroundColor();

            w->setContentsMargins(5, 5, 5, 5);

            return w;
        }(),
    }.emerge();
    m_stackedWidget = static_cast<QStackedWidget*>(stack);
    m_stackedWidget->setContentsMargins(5, 5, 5, 5);

    auto createButton =
        [this](const QString &text, QWidget *parent,
                              QStackedWidget *stacked, QButtonGroup *group,
                              int index) {
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

        auto row = Layouting::Row{};

        for (int i = 0; i < m_buttonStringList.length(); ++i) {
            row.addItem([&]() {
                auto button = createButton(QString(m_buttonStringList[i]), centralTop,
                                           m_stackedWidget, group, i);
                m_buttonList.append(button);

                return button;
            }());
        }
        row.addItem(Layouting::Stretch());
        // row.addItem([&, this]() {
        //     auto settingsButton = new QPushButton();
        //     settingsButton->setIcon(QIcon("://image/icons8-settings.svg"));
        //     settingsButton->setIconSize(QSize(30, 30));
        //     settingsButton->setFixedSize(40, 40);
        //     settingsButton->setCheckable(false);
        //     settingsButton->setContentsMargins(0, 0, 0, 0);
        //     group->addButton(settingsButton);
        //     connect(settingsButton, &QAbstractButton::clicked, this, [&]() {
        //         // QMessageBox::information(nullptr, "Settings",
        //         QString("Settings")); qDebug() << "settingsButton clicked";

        //         SettingDialog dialog;
        //         dialog.exec();
        //     });
        //     return settingsButton;
        // }());

        row.attachTo(centralTop);
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
