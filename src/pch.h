#pragma once

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif

#include <memory>

#include <QtCore>
#include <QtDebug>
#include <QtGui>
#include <QtNetwork>
#include <QtWidgets>

// QtMultimedia is optional and only pulled in when the module is available at
// build time (see RB_HAVE_MULTIMEDIA in CMake).
#ifdef RB_HAVE_MULTIMEDIA
#include <QtMultimedia>
#endif

// Qt 6 removed QtWinExtras/QtMacExtras. On Windows the native shell APIs used
// for file icons now come directly from the Windows SDK headers.
#if defined(Q_OS_WIN32)
// windows.h must precede shellapi.h (which relies on its macros/types).
#include <windows.h>
// clang-format off
#include <shellapi.h>
// clang-format on
#endif

#ifdef _MSC_VER
#pragma warning pop
#endif
