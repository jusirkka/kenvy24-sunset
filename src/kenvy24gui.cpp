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



#include <qlabel.h>

#include <kmainwindow.h>
#include <kstandardaction.h>
#include <kapplication.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <qslider.h>

#include "kenvy24gui.h"
#include "tabview.h"
#include "mixerinput.h"
#include "kenvy24dockwidget.h"
#include "envystructs.h"

KEnvy24Window::KEnvy24Window()
    : KMainWindow(),
      mShuttingDown(false)
{
    setAttribute(Qt::WA_DeleteOnClose);

    mainView = new TabView(this);

    // Actions
    KStandardAction::quit(this, SLOT(slotQuit()), actionCollection());
    (void) new QAction("Disable levels", 0, mainView, SLOT(enableLevels()), actionCollection(), "enable_levels");

    setStandardToolBarMenuEnabled(false);

    setCentralWidget(mainView);

    createGUI();
    
    mDockWidget = new KEnvy24DockWidget(this);

    mDockWidget->show();
    connect(mDockWidget, SIGNAL(quitSelected()), this, SLOT(slotAboutToQuit()));
    setAutoSaveSettings();

}

KEnvy24Window::~KEnvy24Window() {}


void KEnvy24Window::showAboutApplication() {
    // TODO: implement a custom "about dialog" here
}

bool KEnvy24Window::queryExit()
{
    hide();
    // TODO: save config?
    return true;
}


bool KEnvy24Window::queryClose()
{
    if (!mShuttingDown && !kapp->sessionSaving() &&
       mDockWidget)
    {
        KMessageBox::information(this,
            i18n("<qt>Closing the main window will keep KEnvy24 running in the system tray. "
                 "Use Quit from the File menu to quit the application.</qt>"),
            i18n("Docking in System Tray"), "hideOnCloseInfo");
        hide();
        return false;
    }
    return true;
}


void KEnvy24Window::slotAboutToQuit()
{
    mShuttingDown = true;
}

void KEnvy24Window::slotQuit()
{
    mShuttingDown = true;
    kapp->quit();
}

void KEnvy24Window::show() {
    kdDebug() << k_funcinfo << "enabling levels" << endl;
    mainView->setLevelsEnabled(true);
    KMainWindow::show();
}

void KEnvy24Window::hide() {
    kdDebug() << k_funcinfo << "disabling levels" << endl;
    mainView->setLevelsEnabled(false);
    KMainWindow::hide();
}


void KEnvy24Window::slotPCMVolumeDown() {
    // negative values
    kdDebug() << k_funcinfo << "volume down" << endl;
    int lval = mainView->mixerPCM1->leftVolume->value();
    int rval = mainView->mixerPCM1->rightVolume->value();
    int lmax = mainView->mixerPCM1->leftVolume->maxValue();
    int rmax = mainView->mixerPCM1->rightVolume->maxValue();

    if (lval >= lmax || rval >= rmax) {
        return;
    }

    mainView->mixerPCM1->leftVolume->setValue(lval + 1);

    if (!mainView->mixerPCM1->checkLock->isChecked()) {
        mainView->mixerPCM1->rightVolume->setValue(rval + 1);
    }
}

void KEnvy24Window::slotPCMVolumeUp() {
    // negative values
    kdDebug() << k_funcinfo << "volume up" << endl;
    int lval = mainView->mixerPCM1->leftVolume->value();
    int rval = mainView->mixerPCM1->rightVolume->value();
    int lmin = mainView->mixerPCM1->leftVolume->minValue();
    int rmin = mainView->mixerPCM1->rightVolume->minValue();

    if (lval <= lmin || rval <= rmin) {
        return;
    }

    mainView->mixerPCM1->leftVolume->setValue(lval - 1);

    if (!mainView->mixerPCM1->checkLock->isChecked()) {
        mainView->mixerPCM1->rightVolume->setValue(rval - 1);
    }
}

void KEnvy24Window::slotPCMVolumeMute() {
    kdDebug() << k_funcinfo << "volume toggle" << endl;
    mainView->mixerPCM1->checkMuteLeft->toggle();
    if (!mainView->mixerPCM1->checkLock->isChecked()) {
        mainView->mixerPCM1->checkMuteRight->toggle();
    }
}




#include "kenvy24gui.moc"
