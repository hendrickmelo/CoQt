TEMPLATE = lib
CONFIG += c++11 static
TARGET = CoQt

SOURCES += fiber.cpp \
    scheduler.cpp \
    context.cpp \
    wakecondition.cpp

#fiber backends
win32 {
    SOURCES += fiber_win32.cpp
}
#unix not ready yet
#else:unix {
#    SOURCES += fiber_unix.cpp
#}
else {
    INCLUDEPATH += "/usr/local/Cellar/boost/1.56.0/include/"
    LIBS += "-L/usr/local/Cellar/boost/1.56.0/lib/"
    LIBS += -lboost_coroutine-mt -lboost_system-mt

    SOURCES += fiber_boost.cpp
}

HEADERS += \
    fiber.h \
    scheduler.h \
    context.h \
    wakecondition.h \
    fiber_p.h

