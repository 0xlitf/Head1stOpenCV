#ifndef CROILIST_H
#define CROILIST_H
#include "roi_global.h"

#include <QObject>
#include "croi.h"

class QDomDocument;
class QDomElement;

class ROISHARED_EXPORT CRoiList : public QObject
{
    Q_OBJECT
public:
    explicit CRoiList(QObject *pParent = NULL) : QObject(pParent), m_currentIndex(-1) { }
    CRoiList(const QString &strPath, QObject *pParent = NULL) : QObject(pParent), m_currentIndex(-1) { fromXmlFile(strPath); }
    ~CRoiList() { clear(); }

    CRoiList(const CRoiList &other, QObject *pParent = NULL);
    CRoiList & operator= (const CRoiList &other);

    inline CRoi *append(int type, const QString &strName);
    inline CRoi *append(const CRoi *pRoi);

    CRoi *insert(int index, int type, const QString &strName);
    CRoi *insert(int index, const CRoi *pRoi);

    inline void remove(CRoi *pRoi);
    void remove(int index);

    inline void move(CRoi *pRoi, int to);
    void move(int index, int to);

    inline int count() const;
    inline int size() const;

    CRoi *at(int index);
    const CRoi *at(int index) const;

    CRoi *find(const QString &strName);
    const CRoi *find(const QString &strName) const;

    template <class T> inline T * findRoi(const QString &strName);
    template <class T> inline const T * findRoi(const QString &strName) const;

    inline int indexOf(CRoi *pRoi) const;

    inline QString name(int index) const;
    bool setName(int index, const QString &strName);

    inline quint32 color(int index) const;
    void setColor(int index, quint32 rgb);

    inline void setCurrentColor(quint32 rgb);
    void setAllColor(quint32 rgb);

    inline bool visible(int index) const;
    void setVisible(int index, bool visible);

    inline void setCurrentVisible(bool visible);
    void setAllVisible(bool visible);

    inline bool enable(int index) const;
    void setEnable(int index, bool enable);

    inline void setCurrentEnable(bool enable);
    void setAllEnable(bool enable);

    inline bool getData(int index, CRoi *pRoi) const;
    bool setData(int index, const CRoi *pRoi, QObject *pSender = NULL);
    bool setValue(int index, const CRoi *pRoi, QObject *pSender = NULL);

    inline bool getCurrentData(CRoi *pRoi) const;
    inline bool setCurrentData(const CRoi *pRoi, QObject *pSender = NULL);
    inline bool setCurrentValue(const CRoi *pRoi, QObject *pSender = NULL);

    inline double getElementValue(int index, int role) const;
    bool setElementValue(int index, int role, double value, QObject *pSender = NULL);
    inline bool setCurrentElementValue(int role, double value, QObject *pSender = NULL);

    inline CRoi *currentRoi();
    inline const CRoi *currentRoi() const;

    inline int currentIndex() const;
    inline void setCurrentIndex(int index);

    inline void adjustRect(const QRect &rect, QObject *pSender = NULL);

    void clear();

    bool fromXmlFile(const QString &strFile, QString *pstrError = NULL);
    bool toXmlFile(const QString &strFile, QString *pstrError = NULL) const;

    void fromXmlElement(const QDomElement &listElem);
    QDomElement toXmlElement(QDomDocument &doc) const;

signals:
    void signal_inserted(int index, void *pRoiData);
    void signal_removed(int index);
    void signal_moved(int index, int to);
    void signal_data_changed(int index, void *pRoiData, QObject *pSender);
    void signal_value_changed(int index, void *pRoiData, QObject *pSender);
    void signal_adjust_window(int index);
    void signal_current_index_changed(int index);
    void signal_set_name(int index, QString strName);
    void signal_set_color(int index, quint32 rgb);
    void signal_set_visible(int index, bool visible);
    void signal_set_enable(int index, bool enable);

private:
    QList<CRoi*> m_roiList;
    int m_currentIndex;

    friend CRoiList;
};

template <class T>
T * CRoiList::findRoi(const QString &strName)
{
    for (int i = 0; i < m_roiList.size(); ++i)
    {
        CRoi *pRoi = m_roiList.at(i);
        if (pRoi->m_type == T::ID && pRoi->m_strName == strName)
            return static_cast<T*>(pRoi);
    }
    return NULL;
}

template <class T>
const T * CRoiList::findRoi(const QString &strName) const
{
    for (int i = 0; i < m_roiList.size(); ++i)
    {
        CRoi *pRoi = m_roiList.at(i);
        if (pRoi->m_type == T::ID && pRoi->m_strName == strName)
            return static_cast<T*>(pRoi);
    }
    return NULL;
}

inline CRoi * CRoiList::append(int type, const QString &strName)
{
    return insert(m_roiList.size(), type, strName);
}


CRoi * CRoiList::append(const CRoi *pRoi)
{
    return insert(m_roiList.size(), pRoi);
}

inline void CRoiList::remove(CRoi *pRoi)
{
    remove(indexOf(pRoi));
}

inline void CRoiList::move(CRoi *pRoi, int to)
{
    move(indexOf(pRoi), to);
}

inline int CRoiList::count() const
{
    return m_roiList.size();
}

inline int CRoiList::size() const
{
    return m_roiList.size();
}

inline int CRoiList::indexOf(CRoi *pRoi) const
{
    return m_roiList.indexOf(pRoi);
}

inline QString CRoiList::name(int index) const
{
    const CRoi *pRoi = at(index);
    return pRoi != NULL ? pRoi->m_strName : QString();
}

inline quint32 CRoiList::color(int index) const
{
    const CRoi *pRoi = at(index);
    return pRoi != NULL ? pRoi->m_rgb : 0;
}

inline void CRoiList::setCurrentColor(quint32 rgb)
{
    setColor(m_currentIndex, rgb);
}

inline bool CRoiList::visible(int index) const
{
    const CRoi *pRoi = at(index);
    return pRoi != NULL ? pRoi->m_visible : false;
}

inline void CRoiList::setCurrentVisible(bool visible)
{
    setVisible(m_currentIndex, visible);
}

inline bool CRoiList::enable(int index) const
{
    const CRoi *pRoi = at(index);
    return pRoi != NULL ? pRoi->m_enable : false;
}

inline void CRoiList::setCurrentEnable(bool enable)
{
    setEnable(m_currentIndex, enable);
}

inline bool CRoiList::getData(int index, CRoi *pRoi) const
{
    return pRoi != NULL ? pRoi->setData(at(index)) : false;
}

inline bool CRoiList::getCurrentData(CRoi *pRoi) const
{
    return getData(m_currentIndex, pRoi);
}

inline bool CRoiList::setCurrentData(const CRoi *pRoi, QObject *pSender /*= NULL*/)
{
    return setData(m_currentIndex, pRoi, pSender);
}

inline bool CRoiList::setCurrentValue(const CRoi *pRoi, QObject *pSender /*= NULL*/)
{
    return setValue(m_currentIndex, pRoi, pSender);
}

inline double CRoiList::getElementValue(int index, int role) const
{
    const CRoi *pRoi = at(index);
    return pRoi != NULL ? pRoi->elementValue(role) : 0.0;
}

inline bool CRoiList::setCurrentElementValue(int role, double value, QObject *pSender /*= NULL*/)
{
    return setElementValue(m_currentIndex, role, value, pSender);
}

inline void CRoiList::setCurrentIndex(int index)
{
    if (index < -1 || index >= m_roiList.size())
        index = -1;

    if (m_currentIndex != index)
    {
        m_currentIndex = index;
        emit signal_current_index_changed(index);
    }
}

inline CRoi *CRoiList::currentRoi()
{
    return m_currentIndex != -1 ? m_roiList.at(m_currentIndex) : NULL;
}

inline const CRoi * CRoiList::currentRoi() const
{
    return m_currentIndex != -1 ? m_roiList.at(m_currentIndex) : NULL;
}

inline int CRoiList::currentIndex() const
{
    return m_currentIndex;
}

inline void CRoiList::adjustRect(const QRect &rect, QObject *pSender /*= NULL*/)
{
    if (m_currentIndex != -1)
    {
        m_roiList.at(m_currentIndex)->adjustRect(rect);
        emit signal_value_changed(m_currentIndex, m_roiList.at(m_currentIndex), pSender);
    }
}

#endif // CROILIST_H