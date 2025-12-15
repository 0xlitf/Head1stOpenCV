#ifndef CLINE_H
#define CLINE_H
#include "roi_global.h"

#include "croi.h"

namespace ROI{

class ROISHARED_EXPORT CLine : public CRoi
{
public:
    enum IndexRole { X1Role, Y1Role, X2Role, Y2Role };
    enum ExIndexRole { RadianRole = Y2Role + 1, AngleRole };
    enum { ID = Line };

    CLine()
        : CRoi(Type(ID))
        , m_x1(0.0)
        , m_y1(0.0)
        , m_x2(100.0)
        , m_y2(100.0) { }

    CLine(const QString &strName)
        : CRoi(Type(ID)
        , strName)
        , m_x1(0.0)
        , m_y1(0.0)
        , m_x2(100.0)
        , m_y2(100.0) { }

    CLine(double x1, double y1, double x2, double y2)
        : CRoi(Type(ID))
        , m_x1(x1)
        , m_y1(y1)
        , m_x2(x2)
        , m_y2(y2) { }

    CLine(const QString &strName, double x1, double y1, double x2, double y2)
        : CRoi(Type(ID), strName)
        , m_x1(x1)
        , m_y1(y1)
        , m_x2(x2)
        , m_y2(y2) { }

    CLine(const CRoi *pRoiData) : CRoi(Type(ID)) { setData(pRoiData); }

    virtual ~CLine() { }

    virtual bool setValue(const CRoi *pRoiData);

    virtual bool unequalValue(const CRoi *pRoiData);

    virtual CRoi *copySelf() const;

    virtual void adjustRect(const QRect &rect);

    virtual int elementCount(bool all = false) const { return all ? 6 : 4; }

    virtual QStringList elementNames(bool all = false) const;

    virtual QString elementName(int role) const;

    virtual double elementValue(int role) const;

    virtual bool setElementValue(int role, double value);

    double m_x1;
    double m_y1;
    double m_x2;
    double m_y2;

private:
    CLine(const CLine &other);
    CLine & operator= (const CLine &other);
};

class ROISHARED_EXPORT CHorLine : public CRoi
{
public:
    enum IndexRole { ValueRole };
    enum { ID = HorLine };

    CHorLine()
        : CRoi(Type(ID))
        , m_value(100.0) { }

    CHorLine(const QString &strName)
        : CRoi(Type(ID), strName)
        , m_value(100.0) { }

    CHorLine(double value)
        : CRoi(Type(ID))
        , m_value(value) { }

    CHorLine(const QString &strName, double value)
        : CRoi(Type(ID), strName)
        , m_value(value) { }

    CHorLine(const CRoi *pRoiData) : CRoi(Type(ID)) { setData(pRoiData); }

    virtual ~CHorLine() { }

    virtual bool setValue(const CRoi *pRoiData);

    virtual bool unequalValue(const CRoi *pRoiData);

    virtual CRoi *copySelf() const;

    virtual void adjustRect(const QRect &rect);

    virtual int elementCount(bool) const { return 1; }

    virtual QStringList elementNames(bool) const;

    virtual QString elementName(int role) const;

    virtual double elementValue(int role) const;

    virtual bool setElementValue(int role, double value);

    double m_value;

private:
    CHorLine(const CHorLine &other);
    CHorLine & operator= (const CHorLine &other);
};

class ROISHARED_EXPORT CVerLine : public CRoi
{
public:
    enum IndexRole { ValueRole };
    enum { ID = VerLine };

    CVerLine()
        : CRoi(Type(ID))
        , m_value(100.0) { }

    CVerLine(double value)
        : CRoi(Type(ID))
        , m_value(value) { }

    CVerLine(const QString &strName)
        : CRoi(Type(ID), strName)
        , m_value(100.0) { }

    CVerLine(const QString &strName, double value)
        : CRoi(Type(ID), strName)
        , m_value(value) { }

    CVerLine(const CRoi *pRoiData) : CRoi(Type(ID)) { setData(pRoiData); }

    virtual ~CVerLine() { }

    virtual bool setValue(const CRoi *pRoiData);

    virtual bool unequalValue(const CRoi *pRoiData);

    virtual CRoi *copySelf() const;

    virtual void adjustRect(const QRect &rect);

    virtual int elementCount(bool) const { return 1; }

    virtual QStringList elementNames(bool) const;

    virtual QString elementName(int role) const;

    virtual double elementValue(int role) const;

    virtual bool setElementValue(int role, double value);

    double m_value;

private:
    CVerLine(const CVerLine &other);
    CVerLine & operator= (const CVerLine &other);
};

} // namespace ROI
#endif // CLINE_H