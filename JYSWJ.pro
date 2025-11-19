QT       += core gui serialport network printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

RC_ICONS = favicon.ico

LIBS += -ldbghelp

SOURCES += \
    iosettingsform.cpp \
    main.cpp \
    mainwindow.cpp \
    multisendform.cpp \
    recvareaform.cpp \
    sendareaform.cpp \
    stateform.cpp \
    toolbarform.cpp \
    versionintroductionform.cpp

HEADERS += \
    common.h \
    iosettingsform.h \
    mainwindow.h \
    multisendform.h \
    recvareaform.h \
    sendareaform.h \
    stateform.h \
    toolbarform.h \
    versionintroductionform.h

FORMS += \
    iosettingsform.ui \
    mainwindow.ui \
    multisendform.ui \
    recvareaform.ui \
    sendareaform.ui \
    stateform.ui \
    toolbarform.ui \
    versionintroductionform.ui

INCLUDEPATH += $$PWD/DeviceIOService
include($$PWD/DeviceIOService/DeviceIOService.pri)

INCLUDEPATH += $$PWD/GraphShow
include($$PWD/GraphShow/GraphShow.pri)


INCLUDEPATH += $$PWD/ProtocolTransfer
include($$PWD/ProtocolTransfer/ProtocolTransfer.pri)


INCLUDEPATH += $$PWD/CommonWidget
include($$PWD/CommonWidget/CommonWidget.pri)

INCLUDEPATH += $$PWD/CommonTools
include($$PWD/CommonTools/CommonTools.pri)

INCLUDEPATH += $$PWD/RingBuffer
include($$PWD/RingBuffer/RingBuffer.pri)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    images.qrc
