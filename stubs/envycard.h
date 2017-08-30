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


#ifndef _ENVYCARD_H_
#define _ENVYCARD_H_

#include <QObject>
#include <alsa/asoundlib.h>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QHash>
#include <QVector>


/**
 * Router sources: these are routed to either digital L/R out or analog L/R out
 */
#define R_SRC_PCM "PCM Out"
#define R_SRC_ANALOG "H/W In 0"
#define R_SRC_DIGITAL_L "IEC958 In L"
#define R_SRC_DIGITAL_R "IEC958 In R"
#define R_SRC_MIXER "Digital Mixer"


/**
 * HW settings items
 */

#define HW_ENUM_INTERNAL_CLOCK "Multi Track Internal Clock"
#define HW_ENUM_CLOCK_DEFAULT "Multi Track Internal Clock Default"
#define HW_ENUM_DEEMPHASIS "Deemphasis"
#define HW_BOOL_RATE_LOCKING "Multi Track Rate Locking"
#define HW_BOOL_RATE_RESET "Multi Track Rate Reset"


/**
 * number of PCM output channels
 */
#define MAX_PCM_CHANNELS 4

/**
 * number of *stereo* output SPDIF channels
 */
#define MAX_SPDIF_CHANNELS	1

#define MAX_MULTI_CHANNELS 5 // MAX_PCM_CHANNELS + MAX_SPDIF_CHANNELS

/**
 * Structure used by level monitors
 */

enum class Position {Left, Right};

class ChannelState;

struct StereoLevels {
  int left, right;
  StereoLevels(int l=0, int r=0): left(l), right(r) {}

  ChannelState state(Position p) const;

};


/**
 * Structure used to adjust mixer levels
 */
struct ChannelState {
    int volume, stereo;

    ChannelState(int v=0, int s=0): volume(v), stereo(s) {}

    StereoLevels levels(Position k) const;
};


class CardNotFound {}; // Exception

class EnvyCard : public QObject {
    Q_OBJECT
public:

    typedef QMap<int, StereoLevels> PeakMap;
    typedef QVector<int> IndexList;

private:

    PeakMap mPeaks;
    QHash<QString, QStringList> mConfigEnums;
    int mDummyPeak;

    QMap<QString, int> mAddressBase;
    QMap<int, QString> mSectionBase;
    QMap<int, QString> mMuteSwitchSectionBase;


    EnvyCard();
    EnvyCard(const EnvyCard&); // not implemented
    void operator=(EnvyCard const&); // not implemented

public:

    ~EnvyCard();

    static EnvyCard& Instance();

    void configAddresses(const QList<int>&);

    /**
     * This method reads current peak levels from the sound card.
     */
    const PeakMap& peaks();

    /**
     * Emit current state of the card
     */

    void pulse();

    // addresses
    int PCMAddress(int index) const;
    int AnalogInAddress() const;
    int DigitalInAddress() const;
    int AnalogOutAddress() const;
    int DigitalOutAddress() const;
    int DACAddress() const;



signals:
    void mixerVolumeChanged(int source, Position channel, const ChannelState&);
    void mixerMuteSwitchChanged(int source, Position channel, bool on);
    void mixerRouteChanged(int sink, Position, const QString&);
    void masterVolumeChanged(Position channel, int);
    void boolConfigChanged(const QString&, bool);
    void enumConfigChanged(const QString&, const QString&);

protected slots:

    void on_slot_mixerVolumeChanged(int source, Position, const ChannelState&);
    void on_slot_mixerMuteSwitchChanged(int source, Position, bool);
    void on_slot_mixerRouteChanged(int sink, Position channel, const QString&);
    void on_slot_masterVolumeChanged(Position, int level);
    void on_slot_boolConfigChanged(const QString&, bool);
    void on_slot_enumConfigChanged(const QString&, const QString&);

};

#endif // _ENVYCARD_H_
