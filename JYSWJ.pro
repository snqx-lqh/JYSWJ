QT       += core gui serialport network printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

#CONFIG += force_debug_info

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += APPVERSION=\\\"V1.0.4.1\\\"

RC_ICONS = JYSWJ.ico

LIBS += -ldbghelp

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += $$PWD/DeviceIOService
include($$PWD/DeviceIOService/DeviceIOService.pri)

INCLUDEPATH += $$PWD/GraphShow
include($$PWD/GraphShow/GraphShow.pri)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    MyImages.qrc

## ========== 调试信息配置 ==========
## Debug 模式：默认有 -g，不动
## Release 模式：额外加上 -g，同时保留优化
#QMAKE_CXXFLAGS_RELEASE -= -O2
#QMAKE_CXXFLAGS_RELEASE += -O2 -g

## 避免 Release 链接时 strip 掉符号
#QMAKE_LFLAGS_RELEASE -= -s

## MSVC 下也强制生成 PDB（Release 模式）
#win32-msvc* {
#    QMAKE_CXXFLAGS_RELEASE += /Zi
#    QMAKE_LFLAGS_RELEASE   += /DEBUG /INCREMENTAL:NO
#}
