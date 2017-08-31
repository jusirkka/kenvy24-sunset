TEMPLATE = app

TARGET.files = kenvy24-sunset
TARGET.path = /usr/bin

CONFIG += -debug

QT += core gui widgets dbus

DBUS_ADAPTORS += mixer.xml tray.xml

SOURCES += mainwindow.cpp mixerinput.cpp \
           peakmeter.cpp card/envycard.cpp main.cpp mastervolume.cpp \
           patchbox.cpp settings.cpp led.cpp trayitem.cpp

HEADERS += mainwindow.h mixerinput.h \
           peakmeter.h card/envycard.h mastervolume.h \
           patchbox.h settings.h led.h trayitem.h

FORMS += mainwindow.ui mixerinput.ui mastervolume.ui patchbox.ui

OTHER_FILES += kenvy24-sunset.desktop INSTALL README.md

LIBS += -lasound

INCLUDEPATH += ./card

DT.files = kenvy24-sunset.desktop
DT.path = /usr/share/applications

ICONS16.files = icons/16x16/kenvy24-sunset.png
ICONS16.path = /usr/share/icons/hicolor/16x16/apps

ICONS32.files = icons/32x32/kenvy24-sunset.png
ICONS32.path = /usr/share/icons/hicolor/32x32/apps


INSTALLS += TARGET DT ICONS16 ICONS32
