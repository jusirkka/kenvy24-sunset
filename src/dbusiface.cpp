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
#include "kenvy24_sunsetadaptor.h"

DBusIface::DBusIface(QObject *parent)
         : QObject(parent)
{
    new Kenvy24_sunsetAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/Mixer", this);
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

void DBusIface::analogVolumeUp()
{
    emit signalAnalogVolumeUp();
}

void DBusIface::analogVolumeDown()
{
    emit signalAnalogVolumeDown();
}

void DBusIface::analogVolumeMute()
{
    emit signalAnalogVolumeMute();
}


void DBusIface::digitalVolumeUp()
{
    emit signalDigitalVolumeUp();
}

void DBusIface::digitalVolumeDown()
{
    emit signalDigitalVolumeDown();
}

void DBusIface::digitalVolumeMute()
{
    emit signalDigitalVolumeMute();
}


