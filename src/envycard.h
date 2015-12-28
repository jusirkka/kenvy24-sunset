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

enum LeftRight {
    LEFT = 0,
    RIGHT = 1
};

class ChannelState;

struct StereoLevels {
  int left, right;
  StereoLevels(int l=0, int r=0): left(l), right(r) {}

  ChannelState state(LeftRight k) const;

};


/**
 * Structure used to adjust mixer levels
 */
struct ChannelState {
    int volume, stereo;

    ChannelState(int v=0, int s=0): volume(v), stereo(s) {}

    StereoLevels levels(LeftRight k) const;
};


class CardNotFound {}; // Exception

class QSocketNotifier;

class EnvyCard : public QObject {
    Q_OBJECT
public:

    typedef QMap<int, StereoLevels> PeakMap;
    typedef QVector<int> IndexList;
    typedef void (EnvyCard::*(EventHandler))(const QString&, int);

private:

    snd_ctl_t *mCard;
    QList<QSocketNotifier*> mEventNotifiers;
    PeakMap mPeaks;
    QStringList mRoutes;
    QHash<QString, QStringList> mConfigEnums;
    QHash<QString, EventHandler> mHandlers;

    QMap<QString, int> mAddressBase;
    QMap<int, QString> mSectionBase;
    QMap<int, QString> mMuteSwitchSectionBase;

    class ControlElement {

    public:
        snd_ctl_elem_value_t *element;

        ControlElement(_snd_ctl_elem_iface, const QString& name);
        ControlElement(_snd_ctl_elem_iface, const QString& name,  int index);
        ~ControlElement();

        snd_ctl_elem_value_t* operator & () const {
            return element;
        }
        int get(int index=0) const;
        void set(int index, int v) const;
        int write(snd_ctl_t* card) const;
        int read(snd_ctl_t* card) const;
    };

    ControlElement mRawPeaks;

    EnvyCard();
    EnvyCard(const EnvyCard&); // not implemented
    void operator=(EnvyCard const&); // not implemented

public:

    ~EnvyCard();

    static EnvyCard& Instance();

    void configAddresses(const IndexList&);

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
    void mixerVolumeChanged(int source, LeftRight channel, const ChannelState&);
    void mixerMuteSwitchChanged(int source, LeftRight channel, bool on);
    void mixerRouteChanged(int sink, LeftRight, const QString&);
    void masterVolumeChanged(LeftRight channel, int);
    void boolConfigChanged(const QString&, bool);
    void enumConfigChanged(const QString&, const QString&);

protected slots:
    void driverEvent(int);

    void on_slot_mixerVolumeChanged(int source, LeftRight, const ChannelState&);
    void on_slot_mixerMuteSwitchChanged(int source, LeftRight, bool);
    void on_slot_mixerRouteChanged(int sink, LeftRight channel, const QString&);
    void on_slot_masterVolumeChanged(LeftRight, int level);
    void on_slot_boolConfigChanged(const QString&, bool);
    void on_slot_enumConfigChanged(const QString&, const QString&);

private:

    // driver event handlers
    void on_event_mixerVolumeChanged(const QString&, int);
    void on_event_mixerMuteSwitchChanged(const QString&, int);
    void on_event_mixerRouteChanged(const QString&, int);
    void on_event_masterVolumeChanged(const QString&, int);
    void on_event_enumConfigChanged(const QString&, int);
    void on_event_boolConfigChanged(const QString&, int);


    int Address(const QString& section, int index) const;
    QString Section(int address);
    QString MuteSwitchSection(int address);
    int Index(int address, LeftRight channel) const;

};

#endif // _ENVYCARD_H_
