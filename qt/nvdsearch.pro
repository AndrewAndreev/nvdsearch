
DEPENDPATH += .
include(../qt/nvdsearch.pri)

TEMPLATE = app
TARGET   = nvdsearch
QT += core sql gui widgets concurrent

win32:DEFINES += _WINDOWS WIN64
unix:DEFINES += UNIX
DEFINES += _UNICODE QT_DLL QT_SQL_LIB QT_WIDGETS_LIB QT_CONCURRENT_LIB

INCLUDEPATH += . \
    ./../nvdsearch/code \
    ./../build/GeneratedFiles/Debug \
    ./../build/GeneratedFiles/Release \
    ./../build/GeneratedFiles

CONFIG(debug, debug|release) {
    message("debug")
    Configuration = debug
} else {
    message("release")
    Configuration = release
}

DESTDIR = ../build/$${Configuration}
OBJECTS_DIR += ../build/objs/$${Configuration}

MOC_DIR += ./../build/GeneratedFiles/$${Configuration}
UI_DIR += ./../build/GeneratedFiles
RCC_DIR += ./../build/GeneratedFiles
