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

#include <qobject.h>
#include <alsa/asoundlib.h>
#include <string>
#include <vector>
#include <qstringlist.h>
#include <qptrlist.h>
#include "envystructs.h"

class QSocketNotifier;

class EnvyCard : public QObject {
    Q_OBJECT
public:

    typedef QMemArray<StereoLevels> PeakList;
    typedef QMemArray<int> IndexList;

private:

    typedef struct {
        unsigned int subvendor; /* PCI[2c-2f] */
        unsigned char size; /* size of EEPROM image in bytes */
        unsigned char version;  /* must be 1 */
        unsigned char codec;    /* codec configuration PCI[60] */
        unsigned char aclink;   /* ACLink configuration PCI[61] */
        unsigned char i2sID;    /* PCI[62] */
        unsigned char spdif;    /* S/PDIF configuration PCI[63] */
        unsigned char gpiomask; /* GPIO initial mask, 0 = write, 1 = don't */
        unsigned char gpiostate; /* GPIO initial state */
        unsigned char gpiodir;  /* GPIO direction state */
        unsigned short ac97main;
        unsigned short ac97pcm;
        unsigned short ac97rec;
        unsigned char ac97recsrc;
        unsigned char dacID[4]; /* I2S IDs for DACs */
        unsigned char adcID[4]; /* I2S IDs for ADCs */
        unsigned char extra[4];
    } ice1712_eeprom_t;

    static EnvyCard*        instance;
    snd_ctl_t               *ctl;
    int                     cardNumber;
    std::string             cardName;
    bool                    cardIsDMX6FIRE;
    ice1712_eeprom_t        card_eeprom;
    snd_ctl_elem_value_t    *peaks;
    bool                    outputActive[ MAX_OUTPUT_CHANNELS ];
    int                     outChannels;
    bool                    outputSPDIFActive[ MAX_SPDIF_CHANNELS ];
    int                     spdifChannels;
    std::vector< QSocketNotifier* >     eventNotifiers;
    PeakList mPeaks;
    QStringList mRoutes;
    QString mConfigInternalClock;
    QString mConfigClockDefault;
    QString mConfigDeemphasis;
    QStringList mConfigEnumVars;
    QPtrList<QStringList> mConfigEnums;

    class SndCtlElemValue {
    public:
        snd_ctl_elem_value_t *val;

        SndCtlElemValue(_snd_ctl_elem_iface, const char* name);
        SndCtlElemValue(_snd_ctl_elem_iface, const char* name,  int index);
        ~SndCtlElemValue();

        snd_ctl_elem_value_t* operator & () const {
            return val;
        }
        int getInteger(int index =0) const {
            return snd_ctl_elem_value_get_integer(val, index);
        }
        void setInteger(int intVal) const {
            snd_ctl_elem_value_set_integer(val, 0, intVal);
        }
        // TODO: map the other snd_ctl_elem_XXX  ALSA calls here, then update code to call them
    };

public:

    EnvyCard();
    ~EnvyCard();

    static EnvyCard* getInstance();

    bool foundEnvyCard() const {
        return cardName.length() >0;
    }
    const std::string& getCardName() const {
        return cardName;
    }


    /**
     * This method reads current peak levels from the sound card.
     */
    const PeakList& getPeaks(const IndexList&);

    /**
     * Emit current state of the card
     */

    void pulse();

    // driver events start
    void mixerUpdatePlaybackVolume(int);
    void mixerUpdateInputVolume(int);
    void mixerUpdateSPDIFVolume(int);
    void mixerUpdatePlaybackSwitch(int);
    void mixerUpdateInputSwitch(int);
    void mixerUpdateSPDIFSwitch(int);
    void patchbayAnalogUpdate(int);
    void patchbayDigitalUpdate(int);
    void dacVolumeUpdate(int);
    void enumConfigUpdate(const char*);
    void boolConfigUpdate(const char*);
    // driver events end

signals:
    void mixerUpdatePCMMuteSwitch(int index, LeftRight channel, bool on);
    void mixerUpdateAnalogInMuteSwitch(int index, LeftRight channel, bool on);
    void mixerUpdateSPDIFInMuteSwitch(int index, LeftRight channel, bool on);
    void mixerUpdatePlaybackVolume(int index, LeftRight channel, MixerAdjustement);
    void mixerUpdateAnalogInVolume(int index, LeftRight channel, MixerAdjustement);
    void mixerUpdateSPDIFVolume(int index, LeftRight channel, MixerAdjustement);
    void analogUpdateDACVolume(LeftRight channel, int);
    void digitalRouteUpdated(int, LeftRight, const QString&);
    void analogRouteUpdated(int, LeftRight, const QString&);
    void boolConfigUpdated(const QString&, bool);
    void enumConfigUpdated(const QString&, const QString&);

protected slots:
    void driverEvent(int);
    void mixerMuteCaptureChannel(int, LeftRight, bool);
    void mixerMutePlaybackChannel(int, LeftRight, bool);
    void mixerMuteSPDIFChannel(int, LeftRight, bool);
    void setMixerCaptureVolume(int, LeftRight, int vol, int stereo);
    void setMixerPlaybackVolume(int, LeftRight, int vol, int stereo);
    void setMixerSPDIFVolume(int, LeftRight, int vol, int stereo);
    void setDACVolume(LeftRight, int level);
    void setDigitalRoute(int index, LeftRight channel, const QString& soundSource);
    void setAnalogRoute(int index, LeftRight channel, const QString& soundSource);
    void setEnumConfig(const QString&, const QString&);
    void setBoolConfig(const QString&, bool);

private:
    enum Section {
        SECTION_PLAYBACK,
        SECTION_CAPTURE,
        SECTION_IEC598
    };

    bool findCard();
    void initPatchbay();
    void initEvents();
    void initConfig();
    void writeSndCtl(const SndCtlElemValue&);
    void readSndCtl(const SndCtlElemValue&);
    void setVolume(Section, int, LeftRight, int vol, int stereo);
    MixerAdjustement getMixerAdjustement(Section, int index, LeftRight);
    const char* getSectionName(Section);
    MixerAdjustement deduceMixerAdjustement(LeftRight channel, int left, int right);
};

#endif // _ENVYCARD_H_
