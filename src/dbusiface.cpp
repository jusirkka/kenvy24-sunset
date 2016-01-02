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
#include "envycard.h"

DBusIface::DBusIface(QObject *parent)
         : QObject(parent)
{
    new Kenvy24_sunsetAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/Mixer", this);

    EnvyCard* card = &EnvyCard::Instance();
    m_pcm = card->PCMAddress(0);
    m_ain = card->AnalogInAddress();
    m_din = card->DigitalInAddress();
    m_iec958 = card->PCMAddress(4);
}

DBusIface::~DBusIface() {}

void DBusIface::pcmPlaybackVolumeUp()
{
    emit signalMixerVolumeIncrement(m_pcm, 1);
}
 
void DBusIface::pcmPlaybackVolumeDown()
{
    emit signalMixerVolumeIncrement(m_pcm, -1);
}

void DBusIface::pcmPlaybackMute()
{
    emit signalMixerVolumeMute(m_pcm);
}


void DBusIface::iec958PlaybackVolumeUp()
{
    emit signalMixerVolumeIncrement(m_iec958, 1);
}

void DBusIface::iec958PlaybackVolumeDown()
{
    emit signalMixerVolumeIncrement(m_iec958, -1);
}

void DBusIface::iec958PlaybackMute()
{
    emit signalMixerVolumeMute(m_iec958);
}

void DBusIface::analogInVolumeUp()
{
    emit signalMixerVolumeIncrement(m_ain, 1);
}

void DBusIface::analogInVolumeDown()
{
    emit signalMixerVolumeIncrement(m_ain, -1);
}

void DBusIface::analogInMute()
{
    emit signalMixerVolumeMute(m_ain);
}

void DBusIface::digitalInVolumeUp()
{
    emit signalMixerVolumeIncrement(m_din, 1);
}

void DBusIface::digitalInVolumeDown()
{
    emit signalMixerVolumeIncrement(m_din, -1);
}

void DBusIface::digitalInMute()
{
    emit signalMixerVolumeMute(m_din);
}

void DBusIface::dacVolumeUp()
{
    emit signalDACVolumeIncrement(1);
}

void DBusIface::dacVolumeDown()
{
    emit signalDACVolumeIncrement(-1);
}

void DBusIface::on_slot_colorChanged(int color) {
    m_peakColor = color;
    emit peakColorChanged(m_peakColor);
}

int DBusIface::peakColor() {
    return m_peakColor;
}
