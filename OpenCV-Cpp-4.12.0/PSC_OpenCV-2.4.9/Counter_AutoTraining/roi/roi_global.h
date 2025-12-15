#ifndef ROI_GLOBAL_H
#define ROI_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(ROI_LIBRARY)
#  define ROISHARED_EXPORT Q_DECL_EXPORT
#else
#  define ROISHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // ROI_GLOBAL_H
