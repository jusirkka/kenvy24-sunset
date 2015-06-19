/***************************************************************************
 *   Copyright (C) 2007 by Valentin Rusu   *
 *   kenvy24@rusu.info   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifndef _KENVY24GUI_H_
#define _KENVY24GUI_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kmainwindow.h>

class TabView;
class KEnvy24DockWidget;

/**
 * @short Application Main Window
 * @author Valentin Rusu <kenvy24@rusu.info>
 * @version 0.1
 */
class KEnvy24Window: public KMainWindow {
    Q_OBJECT
private:
    TabView* mainView;
    KEnvy24DockWidget *mDockWidget;
    bool mShuttingDown;

public:
    /**
     * Default Constructor
     */
    KEnvy24Window();

    /**
     * Default Destructor
     */
    virtual ~KEnvy24Window();

    virtual void show();
    virtual void hide();

public slots:

    void slotPCMVolumeUp();
    void slotPCMVolumeDown();
    void slotPCMVolumeMute();

protected:
    void showAboutApplication();

private:

    virtual bool queryExit();
    virtual bool queryClose();

private slots:
    void slotAboutToQuit();
    void slotQuit();
};

#endif // _KENVY24GUI_H_
