QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    SharedMemoryReader.cpp \
    logwindow.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    SharedMemoryReader.h \
    logwindow.h \
    mainwindow.h

FORMS += \
    logwindow.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    log.txt \
    GUI.pro.user

LIBS += C:/Users/zhuor/Desktop/secure_design/detours/lib.X64/detours.lib

INCLUDEPATH += C:/Users/zhuor/Desktop/secure_design/detours/include
