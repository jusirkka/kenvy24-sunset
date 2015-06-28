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
#include <ksharedconfig.h>

class QButtonGroup;
class QCheckBox;
class QListWidgetItem;
class DBusIface;
class QTimer;
class KStatusNotifierItem;
class EnvyCard;

namespace Ui {
class MainWindow;
}


class MainWindow: public KMainWindow {

    Q_OBJECT



public:

    MainWindow(DBusIface*);
    virtual ~MainWindow();

public slots:

    void updateBoolConfig(const QString&, bool);
    void updateEnumConfig(const QString&, const QString&);

    void internalClockRateChanged(int);
    void digitalClockRateChanged(int);
    void deemphasisChanged(int);
    void masterClockChanged(int);


private slots:

    void on_actionNewProfile_triggered();
    void on_actionSaveProfile_triggered();
    void on_actionDeleteProfile_triggered();
    void on_actionRenameProfile_triggered();

    void on_actionEnableLeds_toggled(bool);

    void on_actionQuit_triggered();
    void on_actionClose_triggered();

    void on_checkLock_toggled(bool);
    void on_checkReset_toggled(bool);

    void on_profiles_currentItemChanged(QListWidgetItem* curr, QListWidgetItem* prev);

    void updateMeters();

private:

    virtual bool queryClose();

    void readState();
    void readProfiles();
    void loadProfile();

    void writeState();
    void writeProfile();

    void createDefaultProfile();

    void setupHWTab();
    void loadHWFromConfig(KConfigBase*);
    void saveHWToConfig(KConfigBase*);

    void connectToCard();
    void connectFromCard();

    QString uniqueName(const QString&);

signals:

    void enumConfigChanged(const QString&, const QString&);
    void boolConfigChanged(const QString&, bool);

private:

    bool inSlotFlag;
    bool inEventFlag;

    struct ExclusiveFlag {
        bool& slotFlag;
        ExclusiveFlag(bool& flag) : slotFlag(flag) {
            flag = true;
        }
        ~ExclusiveFlag() {
            slotFlag = false;
        }
    };

    typedef QVector<int> IndexList;
    IndexList mLevelIndices;
    Ui::MainWindow *mUI;

    QHash<QString, QStringList> mHWStrings;
    QHash<QString, QButtonGroup*> mHWGroups;
    QHash<QString, QCheckBox*> mHWFlags;
    KStatusNotifierItem* mTray;
    bool m_startDocked;
    bool m_shuttingDown;
    QTimer* mTimer;
    int mUpdateInterval;
    EnvyCard* mCard;
};


#endif
