#ifndef CRECT_H
#define CRECT_H
#include "roi_global.h"

#include "croi.h"

namespace ROI{

class ROISHARED_EXPORT CRect : public CRoi
{
public:
    enum IndexRole { XRole, YRole, WidthRole, HeightRole };
    enum { ID = Rect };

    CRect()
        : CRoi(Type(ID))
        , m_x(0.0)
        , m_y(0.0)
        , m_width(100.0)
        , m_height(100.0) { }

    CRect(const QString &strName)
        : CRoi(Type(ID), strName)
        , m_x(0.0)
        , m_y(0.0)
        , m_width(100.0)
        , m_height(100.0) { }

    CRect(double x, double y, double width, double height)
        : CRoi(Type(ID))
        , m_x(x)
        , m_y(y)
        , m_width(width)
        , m_height(height) { }

    CRect(const QString &strName, double x, double y, double width, double height)
        : CRoi(Type(ID), strName)
        , m_x(x)
        , m_y(y)
        , m_width(width)
        , m_height(height) { }

    CRect(const CRoi *pRoiData) : CRoi(Type(ID)) { setData(pRoiData); }

    virtual ~CRect() { }

    virtual bool setValue(const CRoi *pRoiData);

    virtual bool unequalValue(const CRoi *pRoiData);

    virtual CRoi *copySelf() const;

    virtual void adjustRect(const QRect &rect);

    virtual int elementCount(bool) const { return 4; }

    virtual QStringList elementNames(bool) const;

    virtual QString elementName(int role) const;

    virtual double elementValue(int role) const;

    virtual bool setElementValue(int role, double value);

    double m_x;
    double m_y;
    double m_width;
    double m_height;

private:
    CRect(const CRect &other);
    CRect & operator= (const CRect &other);
};

class ROISHARED_EXPORT CRotationRect : public CRoi
{
public:
    enum IndexRole { XRole, YRole, WidthRole, HeightRole, RadianRole };
    enum ExIndexRole { AngleRole = RadianRole + 1, ExRadianRole, ExAngleRole };
    enum { ID = RotationRect };

    CRotationRect()
        : CRoi(Type(ID))
        , m_x(0.0)
        , m_y(0.0)
        , m_width(100.0)
        , m_height(100.0)
        , m_radian(0.0) { }

    CRotationRect(const QString &strName)
        : CRoi(Type(ID)
        , strName)
        , m_x(0.0)
        , m_y(0.0)
        , m_width(100.0)
        , m_height(100.0)
        , m_radian(0.0) { }

    CRotationRect(double x, double y, double radian, double width, double height)
        : CRoi(Type(ID))
        , m_x(x)
        , m_y(y)
        , m_width(width)
        , m_height(height)
        , m_radian(radian) { }

    CRotationRect(const QString &strName, double x, double y, double radian, double width, double height)
        : CRoi(Type(ID), strName)
        , m_x(x)
        , m_y(y)
        , m_width(width)
        , m_height(height)
        , m_radian(radian) { }

    CRotationRect(const CRoi *pRoiData) : CRoi(Type(ID)) { setData(pRoiData); }

    virtual ~CRotationRect() { }

    virtual bool setValue(const CRoi *pRoiData);

    virtual bool unequalValue(const CRoi *pRoiData);

    virtual CRoi *copySelf() const;

    virtual void adjustRect(const QRect &rect);

    virtual int elementCount(bool all = false) const { return all ? 8 : 5; }

    virtual QStringList elementNames(bool) const;

    virtual QString elementName(int role) const;

    virtual double elementValue(int role) const;

    virtual bool setElementValue(int role, double value);

    virtual bool elementWrite(int role) const;

    double m_x;
    double m_y;
    double m_width;
    double m_height;
    double m_radian;

private:
    CRotationRect(const CRotationRect &other);
    CRotationRect & operator= (const CRotationRect &other);
};

} // namespace ROI
#endif // CRECT_H