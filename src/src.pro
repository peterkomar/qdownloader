TEMPLATE = app
CONFIG += warn_on \
    thread \
    qt \
    release
TARGET = qdownload
OBJECTS_DIR = build
MOC_DIR = build
UI_DIR = build
RCC_DIR = build
SOURCES += main.cpp \
    mainwindow.cpp \
    listtasks.cpp \
    downloaditem.cpp \
    fileutils.cpp \
    sqldlg.cpp \
    prefdialog.cpp
QT += network \
    sql
HEADERS += mainwindow.h \
    listtasks.h \
    downloaditem.h \
    fileutils.h \
    sqldlg.h \
    prefdialog.h
RESOURCES += application.qrc
FORMS += sqldlg.ui \
    prefdialog.ui
RC_FILE = winicon.rc

wince* {
   DEPLOYMENT_PLUGIN += qjpeg qgif qtiff
}







