TEMPLATE =  app                  # comment out TEMPLATE = lib 2 lines to make main.cpp
CONFIG +=   console
SOURCES  =  main.cpp

#TEMPLATE = lib                    # comment out main.cpp 3 lines to make the library for linker use, and at top turn on "+=console"
#CONFIG += staticlib

CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++latest
#CONFIG += c++20
CONFIG += warn_on
# ??CONFIG += QMAKE_CFLAGS_WARN_ON
# ??CONFIG += QMAKE_CXXFLAGS_WARN_ON

QMAKE_CXXFLAGS += \
        #-std=gnu++23		\
        -g3 		        \
        -O0 		        \
        -Wall   		\  # https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
        -Wextra   		\
        #-Wno-comment 		\
        -Wno-unused 		\
        -Wno-unused-parameter 	\
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
    #infocmp-1faE.h \
    #infocmp-e.h \
    lib_tty.hpp \
    lib_tty_internal.hpp

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    LICENSE.txt \
    README.txt
