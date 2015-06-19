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
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KENVY24DOCKWIDGET_H
#define KENVY24DOCKWIDGET_H

class KEnvy24Window;

#include <kstatusnotifieritem.h>


class KEnvy24DockWidget : public KStatusNotifierItem
{
   Q_OBJECT


 public:
   explicit KEnvy24DockWidget(KEnvy24Window *parent);
   virtual ~KEnvy24DockWidget();

 public slots:
   void activate(const QPoint &pos);

 private:
   KEnvy24Window* mParent;

};

#endif
