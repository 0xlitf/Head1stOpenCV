#ifndef CVA_GLOBAL_H
#define CVA_GLOBAL_H

#include <QtCore/qglobal.h>

#pragma execution_character_set("utf-8")

#if defined(CVA_LIBRARY)
#  define CVASHARED_EXPORT Q_DECL_EXPORT
#else
#  define CVASHARED_EXPORT
#endif

#endif // CVA_GLOBAL_H
