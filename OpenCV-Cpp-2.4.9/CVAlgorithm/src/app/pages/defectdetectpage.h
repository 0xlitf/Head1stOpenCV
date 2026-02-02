#ifndef DEFECTDETECTPAGE_H
#define DEFECTDETECTPAGE_H

#pragma execution_character_set("utf-8")

#include <controls/controls.h>
#include "contourextractor.h"

class DefectDetectPage : public WidgetBase {
    Q_OBJECT
public:
    DefectDetectPage();

    void testAreaDiff();
    void testDefect();
    void testContour();

    void testCorner();
};

#endif // DEFECTDETECTPAGE_H
