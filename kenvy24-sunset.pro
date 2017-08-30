TEMPLATE = app

TARGET = kenvy24-sunset

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
