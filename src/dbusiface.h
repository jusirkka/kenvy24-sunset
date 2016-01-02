/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-12
 * Description : a DCROP interface. 
 * 
 * Copyright (C) 2005 by Leonid Zeitlin <lz@europe.com> 
 * Copyright (C) 2006 Tom Albers <tomalbers@kde.nl>
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
 
#ifndef DBUSIFACE_H
#define DBUSIFACE_H

#include <QObject>

class DBusIface : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.github.kenvy24_sunset")

signals:
    
    void signalDACVolumeIncrement(int incr);
    void signalMixerVolumeIncrement(int  address, int incr);
    void signalMixerVolumeMute(int address);
    void peakColorChanged(int color);



public slots:

    void pcmPlaybackVolumeUp();
    void pcmPlaybackVolumeDown();
    void pcmPlaybackMute();

    void iec958PlaybackVolumeUp();
    void iec958PlaybackVolumeDown();
    void iec958PlaybackMute();

    void analogInVolumeUp();
    void analogInVolumeDown();
    void analogInMute();

    void digitalInVolumeUp();
    void digitalInVolumeDown();
    void digitalInMute();

    void dacVolumeUp();
    void dacVolumeDown();

    int peakColor();

    void on_slot_colorChanged(int color);

public:

    DBusIface(QObject* parent=0);

    ~DBusIface();

private:
    int m_pcm, m_iec958, m_din, m_ain;
    int m_peakColor;
};

#endif // DBUSIFACE_H
