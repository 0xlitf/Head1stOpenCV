#ifndef CROI_H
#define CROI_H
#include "roi_global.h"

#include <QStringList>
class QRect;

class ROISHARED_EXPORT CRoi
{
public:
    enum Type{
        Invalid = -1,
        Line = 0,        // 直线
        HorLine,         // 水平线
        VerLine,         // 垂直线
        Rect,            // 矩形
        RotationRect,    // 旋转矩形
        Circle,          // 圆形
        Ring,            // 圆环
        Count            // 
    };

    enum { ID = Invalid };

    CRoi() : m_type(Invalid), m_rgb(0x0000FF00), m_visible(true), m_enable(true) { }
    CRoi(const QString &strName)
        : m_type(Invalid), m_strName(strName), m_rgb(0x0000FF00), m_visible(true), m_enable(true)
    { }
    virtual ~CRoi() { }

    static QStringList typeNames();
    static QString typeName(int type);
    static int typeIndex(const QString &strName);

    static QStringList iconPaths();
    static QString iconPath(int type);

    static CRoi * createRoi(int type, const QString &strName);

    static const double factorPi;
    static const double factor2Pi;
    static const double factorToAngle;
    static const double factorToRadian;

    virtual bool setData(const CRoi *pRoiData);
    virtual bool equalData(const CRoi *pRoiData);
    virtual bool unequalData(const CRoi *pRoiData);

    virtual bool setValue(const CRoi *pRoiData);
    virtual bool equalValue(const CRoi *pRoiData);
    virtual bool unequalValue(const CRoi *pRoiData);

    virtual CRoi *copySelf() const;

    virtual void adjustRect(const QRect &rect);

    virtual int elementCount(bool all = false) const;

    virtual QStringList elementNames(bool all = false) const;
    virtual QString elementName(int role) const;

    virtual double elementValue(int role) const;
    virtual bool setElementValue(int role, double value);

    virtual double elementValue(const QString &strName) const;
    virtual bool setElementValue(const QString &strName, double value);

    //默认所有IndexRole都能写，所有ExIndexRole都不能写
    virtual bool elementWrite(int role) const;

    Type    m_type;
    QString m_strName;
    quint32 m_rgb;
    bool    m_visible;
    bool    m_enable;

protected:
    CRoi(Type type)
        : m_type(type)
        , m_rgb(0x0000FF00)
        , m_visible(true)
        , m_enable(true) { }

    CRoi(Type type, const QString &strName)
        : m_type(type)
        , m_strName(strName)
        , m_rgb(0x0000FF00)
        , m_visible(true)
        , m_enable(true) { }

private:
    CRoi(const CRoi &other);
    CRoi & operator= (const CRoi &other);
};

#endif // CROI_H
