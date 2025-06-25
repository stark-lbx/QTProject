QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    chat.cpp \
    file.cpp \
    friend.cpp \
    index.cpp \
    main.cpp \
    client.cpp \
    protocol.cpp \
    reshandler.cpp \
    sharefile.cpp \
    uploader.cpp

HEADERS += \
    chat.h \
    client.h \
    file.h \
    friend.h \
    index.h \
    protocol.h \
    reshandler.h \
    sharefile.h \
    uploader.h

FORMS += \
    chat.ui \
    client.ui \
    file.ui \
    friend.ui \
    index.ui \
    sharefile.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    src.qrc
