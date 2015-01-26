/*
 * KMix -- KDE's full featured mini mixer
 *
 *
 * Copyright (C) 2000 Stefan Schimanski <1Stein@gmx.de>
 * Copyright (C) 2001 Preston Brown <pbrown@kde.org>
 * Copyright (C) 2003 Sven Leiber <s.leiber@web.de>
 * Copyright (C) 2004 Christian Esken <esken@kde.org>
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

#include <kaction.h>
#include <klocale.h>
#include <kapplication.h>
#include <kpanelapplet.h>
#include <kpopupmenu.h>
#include <kglobalsettings.h>
#include <kdialog.h>
#include <kaudioplayer.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kwin.h>

#include <qapplication.h>
#include <qcursor.h>
#include <qtooltip.h>
#include <X11/Xlib.h>
#include <fixx11h.h>

#include "kenvy24dockwidget.h"
#include "tabviewimpl.h"

KEnvy24DockWidget::KEnvy24DockWidget(TabViewImpl* mixer, QWidget *parent, const char *name):
    KSystemTray(parent, name),
    mMixerWin(mixer)
{
    setPixmap(loadIcon("kenvy24gui"));
}

KEnvy24DockWidget::~KEnvy24DockWidget() {}


#include "kenvy24dockwidget.moc"
