#-------------------------------------------------
#
# Project created by QtCreator 2016-05-10T22:54:17
#
#-------------------------------------------------

QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RuilxJudgeServer0v20
TEMPLATE = app
QMAKE_CXXFLAGS += --std=c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    com/network.cpp \
    com/networkhandle.cpp \
    com/networkcommunication.cpp \
    com/database.cpp \
    com/compile.cpp \
    config.cpp \
    com/networkjudgecommunication.cpp \
    com/runcode.cpp \
    com/assessment.cpp

HEADERS  += mainwindow.h \
    com/network.h \
    com/networkhandle.h \
    com/networkcommunication.h \
    com/global.h \
    com/database.h \
    com/compile.h \
    config.h \
    com/networkjudgecommunication.h \
    com/runcode.h \
    com/assessment.h

DISTFILES += \
    config.ini

TRANSLATIONS += zh_CN.UTF-8.ts
