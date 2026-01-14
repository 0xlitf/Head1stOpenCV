#ifndef PSC_GLOBAL_H
#define PSC_GLOBAL_H

#include <QtCore/qglobal.h>

#pragma execution_character_set("utf-8")

#if defined(PSC_LIBRARY)
#  define PSCSHARED_EXPORT Q_DECL_EXPORT
#else
#  define PSCSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // PSC_GLOBAL_H
