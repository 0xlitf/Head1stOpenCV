#pragma once

#pragma execution_character_set("utf-8")

#include <QStyle>
#include <QLayout>
#include <QFormLayout>
#include <QList>
#include <QLinkedList>
#include <QString>
#include <QtGlobal>
#include <functional>

QT_BEGIN_NAMESPACE
class QLayout;
class QMargins;
class QObject;
class QWidget;

template<class T>
T qobject_cast(QObject* object);

QT_END_NAMESPACE

namespace Layouting {

    class FlowLayout final : public QLayout {
        Q_OBJECT

    public:
        explicit FlowLayout(QWidget* parent, int margin = -1, int hSpacing = -1, int vSpacing = -1);

        FlowLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);

        ~FlowLayout() override;

        void addItem(QLayoutItem* item) override;

        int horizontalSpacing() const;

        int verticalSpacing() const;

        Qt::Orientations expandingDirections() const override;

        bool hasHeightForWidth() const override;

        int heightForWidth(int width) const override;

        int count() const override;

        QLayoutItem* itemAt(int index) const override;

        QSize minimumSize() const override;

        void setGeometry(const QRect& rect) override;

        QSize sizeHint() const override;

        QLayoutItem* takeAt(int index) override;

    private:
        int doLayout(const QRect& rect, bool testOnly) const;

        int smartSpacing(QStyle::PixelMetric pm) const;

        QList<QLayoutItem*> itemList;
        int m_hSpace = 0;
        int m_vSpace = 0;
    };

    // LayoutItem

    class LayoutBuilder;
    class LayoutItem;
    using LayoutItems = QList<LayoutItem>;

    class LayoutItem {
    public:
        using Setter = std::function<void(QObject* target)>;

        LayoutItem();
        ~LayoutItem();

        LayoutItem(const LayoutItem& t);
        LayoutItem& operator=(const LayoutItem& t) = default;

        template <
            typename T,
            typename = typename std::enable_if<std::is_base_of<LayoutItem, T>::value>::type
            >
        LayoutItem(const T& t) {
            LayoutItem::operator=(t);
        }

        template <
            typename T,
            typename = typename std::enable_if<!std::is_base_of<LayoutItem, T>::value>::type,
            typename = void
            >
        LayoutItem(const T& t) {
            createItem(this, t);
        }

        void attachTo(QWidget* w) const;
        QWidget* emerge();

        void addItem(const LayoutItem& item);
        void addItems(const LayoutItems& items);
        void addRow(const LayoutItems& items);

        std::function<void(LayoutBuilder&)> onAdd;
        std::function<void(LayoutBuilder&)> onExit;
        std::function<void(QObject* target)> setter;
        LayoutItems subItems;
    };

    // Special items

    class Space {
    public:
        explicit Space(int space) : space(space) {
        }

        const int space;
    };

    class Stretch {
    public:
        explicit Stretch(int stretch = 1) : stretch(stretch) {
        }

        const int stretch;
    };

    class Span {
    public:
        Span(int span, const LayoutItem& item) : span(span), item(item) {
        }

        const int span;
        LayoutItem item;
    };

    class Column : public LayoutItem {
    public:
        Column(std::initializer_list<LayoutItem> items);
    };

    class ColumnWithMargin : public LayoutItem {
    public:
        ColumnWithMargin(std::initializer_list<LayoutItem> items, int margin = 5);
    };

    class Row : public LayoutItem {
    public:
        Row(std::initializer_list<LayoutItem> items);
    };

    class RowWithMargin : public LayoutItem {
    public:
        RowWithMargin(std::initializer_list<LayoutItem> items, int margin = 5);
    };

    class Flow : public LayoutItem {
    public:
        Flow(std::initializer_list<LayoutItem> items);
    };

    class Grid : public LayoutItem {
    public:
        Grid() : Grid({}) {
        }

        Grid(std::initializer_list<LayoutItem> items);
    };

    class Form : public LayoutItem {
    public:
        Form() : Form({}) {
        }

        Form(std::initializer_list<LayoutItem> items);
    };

    class Widget : public LayoutItem {
    public:
        Widget(std::initializer_list<LayoutItem> items);
    };

    class Stack : public LayoutItem {
    public:
        Stack() : Stack({}) {
        }

        Stack(std::initializer_list<LayoutItem> items);
    };

    class Tab : public LayoutItem {
    public:
        Tab(const QString& tabName, const LayoutItem& item);
    };

    class If : public LayoutItem {
    public:
        If(bool condition, const LayoutItems& item, const LayoutItems& other = {});
    };

    class Group : public LayoutItem {
    public:
        Group(std::initializer_list<LayoutItem> items);
    };

    class TextEdit : public LayoutItem {
    public:
        TextEdit(std::initializer_list<LayoutItem> items);
    };

    class PushButton : public LayoutItem {
    public:
        PushButton(std::initializer_list<LayoutItem> items);
    };

    class SpinBox : public LayoutItem {
    public:
        SpinBox(std::initializer_list<LayoutItem> items);
    };

    class HSplitter : public LayoutItem {
    public:
        HSplitter(std::initializer_list<LayoutItem> items);
    };

    class VSplitter : public LayoutItem {
    public:
        VSplitter(std::initializer_list<LayoutItem> items);
    };

    class ToolBar : public LayoutItem {
    public:
        ToolBar(std::initializer_list<LayoutItem> items);
    };

    class TabWidget : public LayoutItem {
    public:
        TabWidget(std::initializer_list<LayoutItem> items);
    };

    class Application : public LayoutItem {
    public:
        Application(std::initializer_list<LayoutItem> items);

        int exec(int& argc, char* argv[]);
    };

    void createItem(LayoutItem* item, const std::function<void(QObject* target)>& t);
    void createItem(LayoutItem* item, QWidget* t);
    void createItem(LayoutItem* item, QLayout* t);
    void createItem(LayoutItem* item, LayoutItem (*t)());
    void createItem(LayoutItem* item, const QString& t);
    void createItem(LayoutItem* item, const Span& t);
    void createItem(LayoutItem* item, const Space& t);
    void createItem(LayoutItem* item, const Stretch& t);


    // "Singletons"

    LayoutItem br();
    LayoutItem st();
    LayoutItem empty();
    LayoutItem hr();
    LayoutItem noMargin();
    LayoutItem normalMargin();
    LayoutItem customMargin(const QMargins& margin);
    LayoutItem withFormAlignment();

    // "Setters"

    LayoutItem title(const QString& title);
    LayoutItem text(const QString& text);
    LayoutItem tooltip(const QString& toolTip);
    LayoutItem resize(int, int);
    LayoutItem columnStretch(int column, int stretch);
    LayoutItem spacing(int);
    LayoutItem windowTitle(const QString& windowTitle);
    LayoutItem fieldGrowthPolicy(QFormLayout::FieldGrowthPolicy policy);

    // "Getters"

    class ID {
    public:
        QObject* ob = nullptr;
    };

    LayoutItem id(ID& out);

    void setText(ID id, const QString& text);


    // "Signals"

    LayoutItem onClicked(const std::function<void()>&, QObject* guard = nullptr);
    LayoutItem onStringChanged(const std::function<void(const QString&)>&,
                                           QObject* guard = nullptr);
    LayoutItem onValueChanged(const std::function<void(int)>&,
                                            QObject* guard = nullptr);

    LayoutItem onTextChanged(ID& id, QVariant (*sig)(QObject*));

    // Convenience

    QWidget* createHr(QWidget* parent = nullptr);

    template<class T>
    LayoutItem bindTo(T** out) {
        return [out](QObject* target) {
            *out = qobject_cast<T*>(target);
        };
    }

} // namespace Layouting
