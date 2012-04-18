#-------------------------------------------------
#
# Project created by QtCreator 2012-04-16T22:05:06
#
#-------------------------------------------------

QT       += core gui

TARGET = ftpClient
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    view.cpp

HEADERS  += mainwindow.h \
    view.h

FORMS    += mainwindow.ui

RESOURCES += \
    ftp.qrc

OTHER_FILES += \
    images/dir.png \
    images/file.png
