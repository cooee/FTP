# -------------------------------------------------
# Project created by QtCreator 2012-04-16T22:05:06
# -------------------------------------------------
QT += core \
    gui
QT += network
TARGET = ftpClient
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    dialog.cpp \
    ftp.cpp \
    ftpdownload.cpp \
    setting.cpp \
    gloabdata.cpp
HEADERS += mainwindow.h \
    dialog.h \
    ftp.h \
    ftpdownload.h \
    setting.h \
    gloabdata.h
FORMS += mainwindow.ui \
    dialog.ui \
    setting.ui
RESOURCES += ftp.qrc
OTHER_FILES += images/star.png \
    images/question.png \
    images/outbox.png \
    images/inbox.png \
    images/gear.png \
    images/button-synchronize.png \
    images/button-shuffle.png \
    images/button-play.png \
    images/button-pause.png \
    images/button-cross.png \
    images/web.png \
    images/folder.png
