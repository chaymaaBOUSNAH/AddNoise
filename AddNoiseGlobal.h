#ifndef ADDNOISE_GLOBAL_H
#define ADDNOISE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(ADDNOISE_LIBRARY)
#  define ADDNOISESHARED_EXPORT Q_DECL_EXPORT
#else
#  define ADDNOISESHARED_EXPORT Q_DECL_IMPORT
#endif

#endif 
