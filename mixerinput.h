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
#ifndef _MIXERINPUT_H_INCLUDED_
#define _MIXERINPUT_H_INCLUDED_

#include <QWidget>

class Settings;
enum class Position;
class ChannelState;
class EnvyCard;
class StereoLevels;

namespace Ui {
class MixerInput;
}

class MixerInput : public QWidget {
    Q_OBJECT

private:
    int mAddress;
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

    Ui::MixerInput* mUI;
    int mVDelta;
    int mSDelta;

public:

    typedef QMap<int, QWidget*> Routing;

    MixerInput(QWidget* parent);
    ~MixerInput();

    void setup(int index, const QString& name, const QString& title, Routing& routing);
    void connectToCard(EnvyCard* envyCard);
    void connectFromCard(EnvyCard* envyCard);
        
    void saveToConfig(Settings&);
    void loadFromConfig(Settings&);

    void updatePeaks(const StereoLevels& level);

    void dbus_volumeIncrement(int incr);
    void dbus_volumeMute();

public slots:

    void mixerUpdateMuteSwitch(int, Position, bool);
    void mixerUpdatePlaybackVolume(int index, Position channel, const ChannelState&);

    void on_leftVolume_valueChanged(int);
    void on_leftStereo_valueChanged(int);
    void on_rightVolume_valueChanged(int);
    void on_rightStereo_valueChanged(int);

    void on_checkMuteLeft_toggled(bool);
    void on_checkMuteRight_toggled(bool);
    void on_checkLock_toggled(bool);



signals:

    void muted(int index, Position channel, bool m);
    void adjusted(int index, Position channel, const ChannelState&);
    void notifyRightMute(bool);
    void notifyLeftMute(bool);
    void notifyRightVolume(int);
    void notifyLeftVolume(int);
    void notifyRightStereo(int);
    void notifyLeftStereo(int);
};


#endif // _MIXERINPUT_H_INCLUDED_
