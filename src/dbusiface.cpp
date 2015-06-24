/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-12
 * Description : a DCOP interface.
 * 
 * Copyright (C) 2005 by Leonid Zeitlin <lz@europe.com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

// Local includes.

#include "dbusiface.h"
#include "kenvy24adaptor.h"

DBusIface::DBusIface(QObject *parent)
         : QObject(parent)
{
    new Kenvy24Adaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/kenvy24", this);
    dbus.registerService("org.kvanttiapina.kenvy24");
}

DBusIface::~DBusIface() {}

void DBusIface::pcmVolumeUp()
{
    emit signalPCMVolumeUp();
}
 
void DBusIface::pcmVolumeDown()
{
    emit signalPCMVolumeDown();
}

void DBusIface::pcmVolumeMute()
{
    emit signalPCMVolumeMute();
}


