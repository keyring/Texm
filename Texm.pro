#-------------------------------------------------
#
# Project created by QtCreator 2016-06-07T20:17:06
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Texm
TEMPLATE = app


SOURCES += main.cpp\
        texm.cpp \
    basicimageview.cpp \
    rect.cpp \
    max_rects_pack.cpp

HEADERS  += texm.h \
    texm_util.h \
    basicimageview.h \
    rect.h \
    max_rects_pack.h

FORMS    += texm.ui

RESOURCES += \
    resource/resource.qrc
