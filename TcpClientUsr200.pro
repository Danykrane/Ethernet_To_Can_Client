QT       += core gui
QT       += network
QT       += serialbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    #libq cdevice
    core/libq/cdevice.cpp\
    core/libq/TupleAddressParser.cpp\
    # tcp client
    client/tcpclient.cpp \
    # sqpinBix hex widget
    modules/basecan/basecdevicecan.cpp \
    modules/basecan/cdeviceusrcanet200.cpp \
    utils/hexspinbox.cpp \
    # main widget
    mainwindow.cpp \
    # main
    main.cpp

HEADERS += \
    #libq cdevice
    core/libq/cdevice.h\
    core/libq/TupleAddressParser.h\
    client/tcpclient.h \
    mainwindow.h \
    modules/basecan/basecdevicecan.h \
    modules/basecan/cdeviceusrcanet200.h


INCLUDEPATH += $${PWD}/core/libq
INCLUDEPATH += $${PWD}/core/client
INCLUDEPATH += $${PWD}/modules/basecan

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

