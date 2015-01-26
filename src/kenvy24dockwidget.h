/*
 * KMix -- KDE's full featured mini mixer
 *
 *
 * Copyright (C) 2000 Stefan Schimanski <1Stein@gmx.de>
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

#ifndef KENVY24DOCKWIDGET_H
#define KENVY24DOCKWIDGET_H

class QFrame;
class QString;
#include <qwidget.h>
#include <qvbox.h>

#include <ksystemtray.h>

class TabViewImpl;

class KEnvy24DockWidget: public KSystemTray  {
    Q_OBJECT


public:
    KEnvy24DockWidget(TabViewImpl* mixer, QWidget* parent = 0, const char *name = 0);
    ~KEnvy24DockWidget();


public slots:

protected:

private:
    TabViewImpl* mMixerWin;

private slots:

};

#endif
