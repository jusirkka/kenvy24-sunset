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

#include "kenvy24app.h"
#include "kenvy24gui.h"
#include "dcopiface.h"
#include <kdebug.h>


KEnvy24App::KEnvy24App(): KUniqueApplication(), mEnvy(0) {}


KEnvy24App::~KEnvy24App() {
    //  delete mEnvy;
}


int KEnvy24App::newInstance() {
    if (mEnvy) return KUniqueApplication::newInstance();

    mEnvy = new KEnvy24Window;
    mDCOP = new DCOPIface(this, "mixer");

    connect(mDCOP, SIGNAL(signalPCMVolumeUp()), mEnvy, SLOT(slotPCMVolumeUp()));
    connect(mDCOP, SIGNAL(signalPCMVolumeDown()), mEnvy, SLOT(slotPCMVolumeDown()));
    connect(mDCOP, SIGNAL(signalPCMVolumeMute()), mEnvy, SLOT(slotPCMVolumeMute()));

    if (isRestored() && KMainWindow::canBeRestored(0)) {
        mEnvy->restore(0, FALSE);
    }
    mEnvy->show();
    return 0;
}


#include "kenvy24app.moc"
