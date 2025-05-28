QT += core gui widgets

CONFIG += c++17

TEMPLATE = app
TARGET = FileExplorer

SOURCES += main.cpp \
           mainwindow.cpp

HEADERS += mainwindow.h

RESOURCES += resources.qrc

RC_FILE += appicon.rc
