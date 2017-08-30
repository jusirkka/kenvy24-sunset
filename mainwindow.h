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

#include <QMainWindow>
#include <QMap>

class QButtonGroup;
class QCheckBox;
class QListWidgetItem;
class QTimer;

class Settings;
class EnvyCard;
class TrayItem;

namespace Ui {
class MainWindow;
}


class MainWindow: public QMainWindow {

    Q_OBJECT



public:

    MainWindow(bool docked);
    virtual ~MainWindow();


public slots:

    void updateBoolConfig(const QString&, bool);
    void updateEnumConfig(const QString&, const QString&);

    void internalClockRateChanged(int);
    void digitalClockRateChanged(int);
    void deemphasisChanged(int);
    void masterClockChanged(int);

    // dbus mixer interface
    void pcmPlaybackVolumeUp();
    void pcmPlaybackVolumeDown();
    void pcmPlaybackMute();
    void iec958PlaybackVolumeUp();
    void iec958PlaybackVolumeDown();
    void iec958PlaybackMute();
    void analogInVolumeUp();
    void analogInVolumeDown();
    void analogInMute();
    void digitalInVolumeUp();
    void digitalInVolumeDown();
    void digitalInMute();
    void dacVolumeUp();
    void dacVolumeDown();
    int peakColor() const;
    // dbus tray interface
    void restoreWindow();
    void minimizeWindow();

protected:

    void closeEvent(QCloseEvent *event);

private slots:


    void on_actionQuit_triggered();
    void on_actionClose_triggered();
    void on_actionSave_triggered();

    void on_checkLock_toggled(bool);
    void on_checkReset_toggled(bool);

    void updateMeters();

private:

    void readGUIState();
    void readMixerState();

    void writeGUIState();
    void writeMixerState();


    void setupHWTab();
    void loadHWFromConfig(Settings&);
    void saveHWToConfig(Settings&);

    void connectToCard();
    void connectFromCard();

    QString uniqueName(const QString&);

    static int Color(int level);

signals:

    void enumConfigChanged(const QString&, const QString&);
    void boolConfigChanged(const QString&, bool);
    // dbus mixer interface
    void peakColorChanged(int color);

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

    Ui::MainWindow *mUI;

    QHash<QString, QStringList> mHWStrings;
    QHash<QString, QButtonGroup*> mHWGroups;
    QHash<QString, QCheckBox*> mHWFlags;
    QTimer* mTimer;
    EnvyCard* mCard;
    QMap<int, QWidget*> mRouting;
    int m_PeakSampleFreq, m_MaxPeakSize, m_CurrentSlot, m_MaxPeakSlot, m_MaxPeakValue, m_Color;
    QVector<int> m_MaxPeaks;

    QPoint mLastPos;
    bool mStartDocked;
    TrayItem* mTrayItem;

};


#endif
