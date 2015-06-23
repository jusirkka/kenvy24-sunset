// ------------------------------------------------------------------------------
//   Copyright (C) 2007 by Jukka Sirkka
//   jukka.sirkka@iki.fi
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the
//   Free Software Foundation, Inc.,
//   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// ------------------------------------------------------------------------------

#ifndef kenvy_mainwindow_h
#define kenvy_mainwindow_h

#include <kmainwindow.h>
#include <QModelIndex>

namespace Ui {
class MainWindow;
}


class MainWindow: public KMainWindow {

    Q_OBJECT

public:

    MainWindow();
    virtual ~MainWindow();

protected:

    virtual void closeEvent(QCloseEvent *event);

private slots:

    void on_actionNewProfile_triggered();
    void on_actionSaveProfile_triggered();
    void on_actionDeleteProfile_triggered();
    void on_actionRenameProfile_triggered();

    void on_actionLedsEnabled_toggled(bool);

    void on_actionQuit_triggered();
    void on_actionClose_triggered();

    void on_dbus_PCMVolumeUp();
    void on_dbus_PCMVolumeDown();
    void on_dbus_PCMVolumeMute();


private:


    //! Read saved state
    void readSettings();

    //! Save current state
    void writeSettings();


private:

    Ui::MainWindow *mUI;
    QModelIndex mSelectedIndex;
};


#endif
