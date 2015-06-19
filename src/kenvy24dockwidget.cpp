/*
 * KMix -- KDE's full featured mini mixer
 *
 *
 * Copyright (C) 2000 Stefan Schimanski <1Stein@gmx.de>
 * Copyright (C) 2001 Preston Brown <pbrown@kde.org>
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
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */



#include "kenvy24dockwidget.h"
#include "kenvy24gui.h"

KEnvy24DockWidget::KEnvy24DockWidget(KEnvy24Window* parent)
    :KStatusNotifierItem(parent)
    ,mParent(parent)
{
    setToolTipIconByName("kenvy24");
    setTitle(i18n("Envy Card Mixer"));
    setCategory(Hardware);
    setStatus(Active);
}

KEnvy24DockWidget::~KEnvy24DockWidget()
{
}


void KEnvy24DockWidget::activate(const QPoint&)
{
    if (mParent->isHidden()) mParent->show();
    else mParent->hide();
}


