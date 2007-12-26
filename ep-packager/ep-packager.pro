CONFIG += exceptions \
    release
FORMS += ep-packager.ui
HEADERS += ep-packager.h \
    img2pdf.h \
    qstrnatcmp.h \
    widgets.h
LIBS += -lpdf \
    -lquazip
QT += core \
    gui
SOURCES += ep-packager.cpp \
    img2pdf.cpp \
    main.cpp \
    qstrnatcmp.cpp \
    widgets.cpp
TARGET += ep-packager
TEMPLATE = app
win32 { 
    QMAKE_CXXFLAGS_EXCEPTIONS_ON -= -mthreads
    QMAKE_LFLAGS_EXCEPTIONS_ON -= -mthreads
    RC_FILE += ep-packager.rc
    QMAKE_TARGET_PRODUCT = "E-Portfolio Packager"
    QMAKE_TARGET_DESCRIPTION = "E-Portfolio Packager"
    QMAKE_TARGET_COPYRIGHT = "Copyright (c) Pui Kiu College 2007"
    QMAKE_TARGET_COMPANY = "Pui Kiu College"
}
VERSION = 1.2.0.0
