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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QFile>
#include <QCloseEvent>
#include <QSettings>
#include <QUndoStack>
#include <QApplication>
#include <QImageReader>



MainWindow::MainWindow():
    KMainWindow(),
    mUI(new Ui::MainWindow)
{

    mUI->setupUi(this);

    mUI->profiles->addAction(mUI->actionNewProfile);
    mUI->profiles->addAction(mUI->actionSaveProfile);
    mUI->profiles->addAction(mUI->actionRenameProfile);
    mUI->profiles->addAction(mUI->actionDeleteProfile);

    readSettings();
}


void MainWindow::closeEvent(QCloseEvent*) {}


void MainWindow::on_actionNewProfile_triggered() {}
void MainWindow::on_actionSaveProfile_triggered() {}
void MainWindow::on_actionDeleteProfile_triggered() {}
void MainWindow::on_actionRenameProfile_triggered() {}

void MainWindow::on_actionLedsEnabled_toggled(bool) {}

void MainWindow::on_actionQuit_triggered() {}
void MainWindow::on_actionClose_triggered() {}


void MainWindow::readSettings() {}

void writeSettings() {}



MainWindow::~MainWindow() {
    delete mUI;
}

// #include "mainwindow.moc"
