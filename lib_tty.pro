#TEMPLATE = app                  # comment out main.cpp to make the library for linker use, and at top turn on "+=console"
#CONFIG += console
#SOURCES += \
    #main.cpp                    \   # main.cpp # comment out main.cpp to make the library for linker use, and at top turn on "+=console"

TEMPLATE = lib                 # comment out main.cpp to make the library for linker use, and at top turn on "+=console"
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
        -Wall   		\  # https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
        -Wextra   		\
        #-Wno-comment 		\
        #-Wno-unused-parameter 	\
        #-Wno-uninitialized 	\
        #-Wno-reorder 		\

LIBS += \
        -lpthread \
        -lstdc++_libbacktrace \
        -lrt

SOURCES += \
    lib_tty.cpp                 \
    lib_tty_internal.cpp

HEADERS += \
    lib_tty.h \
    lib_tty_internal.h

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    LICENSE.txt \
    README.txt
