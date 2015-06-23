/*
 * KMix -- KDE's full featured mini mixer
 *
 * Copyright (C) 2000 Stefan Schimanski <schimmi@kde.org>
 * Copyright (C) 2001 Preston Brown <pbrown@kde.org>
 * Copyright (C) 2003 Sven Leiber <s.leiber@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "kenvy24.h"
#include "mainwindow.h"
#include <kdebug.h>
#include "dbusiface.h"

KEnvy24App::KEnvy24App(): KUniqueApplication(), mEnvy(0) {}


KEnvy24App::~KEnvy24App() {
    //  delete mEnvy;
}


int KEnvy24App::newInstance() {
    if (mEnvy) return KUniqueApplication::newInstance();

    mEnvy = new MainWindow;
    mDBus = new DBusIface(this);

    connect(mDBus, SIGNAL(signalPCMVolumeUp()), mEnvy, SLOT(on_dbus_PCMVolumeUp()));
    connect(mDBus, SIGNAL(signalPCMVolumeDown()), mEnvy, SLOT(on_dbus_PCMVolumeDown()));
    connect(mDBus, SIGNAL(signalPCMVolumeMute()), mEnvy, SLOT(on_dbus_PCMVolumeMute()));

    if (isSessionRestored() && KMainWindow::canBeRestored(1)) {
        mEnvy->restore(1, false);
    }
    mEnvy->show();
    return 0;
}


#include "kenvy24.moc"
