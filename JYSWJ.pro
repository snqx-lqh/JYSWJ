QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += APPVERSION=\\\"V1.0.0\\\"

SOURCES += \
    introduceform.cpp \
    main.cpp \
    mainwindow.cpp \
    portsettingsform.cpp \
    recvareaform.cpp \
    sendareaform.cpp \
    serialsettingsform.cpp

HEADERS += \
    introduceform.h \
    mainwindow.h \
    portsettingsform.h \
    recvareaform.h \
    sendareaform.h \
    serialsettingsform.h

FORMS += \
    introduceform.ui \
    mainwindow.ui \
    portsettingsform.ui \
    recvareaform.ui \
    sendareaform.ui \
    serialsettingsform.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
