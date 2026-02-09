#ifndef CVA_GLOBAL_H
#define CVA_GLOBAL_H

#pragma execution_character_set("utf-8")

#include <QtCore/qglobal.h>

#if defined(CVA_LIBRARY)
#  define CVASHARED_EXPORT Q_DECL_EXPORT
#else
#  define CVASHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // CVA_GLOBAL_H
