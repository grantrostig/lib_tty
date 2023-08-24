#TEMPLATE = app
TEMPLATE = lib
#CONFIG += console
CONFIG += staticlib
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++latest
#CONFIG += c++20
CONFIG += warn_on
# ??CONFIG += QMAKE_CFLAGS_WARN_ON
# ??CONFIG += QMAKE_CXXFLAGS_WARN_ON

QMAKE_CXXFLAGS += \
        -std=gnu++2b		\
        -g3 		        \
        -O0 		        \

LIBS += \
        -lpthread \
        -lstdc++_libbacktrace \
        -lrt

SOURCES += \
    lib_tty.cpp

HEADERS += \
    lib_tty.h


# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target
