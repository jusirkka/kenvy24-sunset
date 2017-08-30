/*
 * Cantata
 *
 * Copyright (c) 2011-2017 Craig Drummond <craig.p.drummond@gmail.com>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "trayitem.h"
#include "mainwindow.h"
#include <QMenu>


TrayItem::TrayItem(MainWindow *p)
    : QObject(p)
    , mMainWindow(p)
{

    mTrayItem = new QSystemTrayIcon(this);
    mTrayItemMenu = new QMenu(0);
    QAction* menuAction = mTrayItemMenu->addAction("Raise");
    menuAction->setCheckable(false);
    connect(menuAction, SIGNAL(triggered(bool)), mMainWindow, SLOT(restoreWindow()));
    mTrayItemMenu->addSeparator();
    menuAction = mTrayItemMenu->addAction("Quit");
    menuAction->setCheckable(false);
    connect(menuAction, SIGNAL(triggered(bool)), mMainWindow, SLOT(on_actionQuit_triggered()));
    mTrayItem->setContextMenu(mTrayItemMenu);
    mTrayItem->setIcon(QIcon::fromTheme("folder-music"));
    mTrayItem->show();
    connect(mTrayItem, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayItemClicked(QSystemTrayIcon::ActivationReason)));
}

void TrayItem::trayItemClicked(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        if (mMainWindow->isHidden()) {
            mMainWindow->restoreWindow();
        } else {
            mMainWindow->minimizeWindow();
        }
        break;
    default:
        break;
    }
}
