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

#include "envycard.h"

#include <assert.h>
#include <QSocketNotifier>
#include <algorithm>
#include <math.h>
#include <kdebug.h>

/* MidiMan */
#define ICE1712_SUBDEVICE_DELTA1010 0x121430d6
#define ICE1712_SUBDEVICE_DELTADIO2496  0x121431d6
#define ICE1712_SUBDEVICE_DELTA66   0x121432d6
#define ICE1712_SUBDEVICE_DELTA44   0x121433d6
#define ICE1712_SUBDEVICE_AUDIOPHILE    0x121434d6
#define ICE1712_SUBDEVICE_DELTA410      0x121438d6
#define ICE1712_SUBDEVICE_DELTA1010LT   0x12143bd6

/* Terratec */
#define ICE1712_SUBDEVICE_EWX2496       0x3b153011
#define ICE1712_SUBDEVICE_EWS88MT       0x3b151511
#define ICE1712_SUBDEVICE_EWS88D        0x3b152b11
#define ICE1712_SUBDEVICE_DMX6FIRE      0x3b153811

/* Hoontech */
#define ICE1712_SUBDEVICE_STDSP24       0x12141217      /* Hoontech SoundTrack Audio DSP 24 */

#define ANALOG_PLAYBACK_ROUTE_NAME	"H/W Playback Route"
#define SPDIF_PLAYBACK_ROUTE_NAME	"IEC958 Playback Route"
#define	MULTI_PLAYBACK_SWITCH		"Multi Playback Switch"
#define HW_MULTI_CAPTURE_SWITCH		"H/W Multi Capture Switch"
#define IEC958_MULTI_CAPTURE_SWITCH	"IEC958 Multi Capture Switch"

#define HW_MULTI_CAPTURE_VOLUME		"H/W Multi Capture Volume"
#define IEC958_MULTI_CAPTURE_VOLUME	"IEC958 Multi Capture Volume"
#define MULTI_PLAYBACK_VOLUME       "Multi Playback Volume"

#define DAC_VOLUME_NAME	"DAC Volume"
#define ADC_VOLUME_NAME	"ADC Volume"

EnvyCard::SndCtlElemValue::SndCtlElemValue(_snd_ctl_elem_iface iface, const char* name) {
  snd_ctl_elem_value_malloc(&val);
  snd_ctl_elem_value_set_interface(val, iface);
  snd_ctl_elem_value_set_name(val, name);
}
                                          
EnvyCard::SndCtlElemValue::SndCtlElemValue(_snd_ctl_elem_iface iface, const char* name, int index) {
    snd_ctl_elem_value_malloc(&val);
    snd_ctl_elem_value_set_interface(val, iface);
    snd_ctl_elem_value_set_name(val, name);
    snd_ctl_elem_value_set_index(val, index);
}

EnvyCard::SndCtlElemValue::~SndCtlElemValue() {
  snd_ctl_elem_value_free(val);
}
                                           

EnvyCard::EnvyCard() : QObject(),
    cardName(),
    mPeaks(22)
{
    assert(instance == 0);
    instance = this;
    
    if (findCard()) {
        SndCtlElemValue val(SND_CTL_ELEM_IFACE_CARD, "ICE1712 EEPROM");
        
        int err;
        if ((err = snd_ctl_elem_read(ctl, &val)) < 0) {
            fprintf(stderr, "Unable to read EEPROM contents: %s\n", snd_strerror(err));
            exit(EXIT_FAILURE);
        }
        memcpy(&card_eeprom, snd_ctl_elem_value_get_bytes(&val), 32);
    
        snd_ctl_elem_value_malloc(&peaks);
        snd_ctl_elem_value_set_interface(peaks, SND_CTL_ELEM_IFACE_PCM);
        snd_ctl_elem_value_set_name(peaks, "Multi Track Peak");

        initPatchbay();
        initConfig();
        initEvents();
    }
    
}

EnvyCard::~EnvyCard() {
  while (!eventNotifiers.empty()) {
      delete eventNotifiers.back();
      eventNotifiers.pop_back();
  }
  snd_ctl_close(ctl);
}

EnvyCard* EnvyCard::instance = 0;
EnvyCard* EnvyCard::getInstance() {return instance;}


void EnvyCard::initPatchbay() {
    snd_ctl_elem_info_t *info;

    snd_ctl_elem_info_alloca(&info);

    snd_ctl_elem_info_set_interface(info, SND_CTL_ELEM_IFACE_MIXER);
    snd_ctl_elem_info_set_name(info, ANALOG_PLAYBACK_ROUTE_NAME);
    snd_ctl_elem_info_set_numid(info, 0);
    snd_ctl_elem_info_set_index(info, 0);
    snd_ctl_elem_info(ctl, info);
    int numRoutes = snd_ctl_elem_info_get_items(info);
    for (int i = 0; i < numRoutes; i++) {
        snd_ctl_elem_info_set_item(info, i);
        snd_ctl_elem_info(ctl, info);
        mRoutes << QString(snd_ctl_elem_info_get_item_name(info));
        kDebug() << i << ": " << mRoutes[i];
    }
}

void EnvyCard::initConfig() {
    snd_ctl_elem_info_t *info;
    snd_ctl_elem_info_alloca(&info);
    snd_ctl_elem_info_set_interface(info, SND_CTL_ELEM_IFACE_MIXER);


    QStringList keys;
    keys << QString(HW_ENUM_INTERNAL_CLOCK) << QString(HW_ENUM_CLOCK_DEFAULT) << QString(HW_ENUM_DEEMPHASIS);

    foreach (QString key, keys) {
        QStringList enums;
        snd_ctl_elem_info_set_name(info, key.toAscii());
        snd_ctl_elem_info_set_numid(info, 0);
        snd_ctl_elem_info_set_index(info, 0);
        snd_ctl_elem_info(ctl, info);
        int numItems = snd_ctl_elem_info_get_items(info);
        for (int i = 0; i < numItems; i++) {
            snd_ctl_elem_info_set_item(info, i);
            snd_ctl_elem_info(ctl, info);
            enums << QString(snd_ctl_elem_info_get_item_name(info));
            kDebug() << i << ": " << (enums)[i];
        }
        mConfigEnums[key] = enums;
    }
}


void EnvyCard::initEvents() {
    int npfds = snd_ctl_poll_descriptors_count(ctl);
    if (npfds > 0) {
        struct pollfd* pfds = (pollfd*)alloca(sizeof(*pfds) * npfds);
        npfds = snd_ctl_poll_descriptors(ctl, pfds, npfds);
        for (int i = 0; i < npfds; i++) {
            kDebug() << "creating event notifier for fd = " << pfds[i].fd;
            QSocketNotifier* eventNotifier = new QSocketNotifier(pfds[i].fd, QSocketNotifier::Read);
            connect(eventNotifier, SIGNAL(activated(int)), SLOT(driverEvent(int)));
            eventNotifiers.push_back(eventNotifier);
        }
        snd_ctl_subscribe_events(ctl, 1);
    }
}


bool EnvyCard::findCard() {
    char cardname[8];
    int err;
  
    snd_ctl_card_info_t     *hw_info;
    snd_ctl_card_info_alloca(&hw_info);
  
  // FIXME hard coded number of cards
  for (cardNumber = 0; cardNumber < 8; cardNumber++) {
      sprintf(cardname, "hw:%d", cardNumber);
      if ((err = snd_ctl_open(&ctl, cardname, 0)) < 0)
          continue;
      if (snd_ctl_card_info(ctl, hw_info) < 0 ||
          strcmp(snd_ctl_card_info_get_driver(hw_info), "ICE1712")) {
          snd_ctl_close(ctl);
          continue;
      }
      /* found */
      cardName = cardname;
      return true;
  }
  return false;
}

const EnvyCard::PeakList& EnvyCard::getPeaks(const IndexList& indices) {
    int err;
    err = snd_ctl_elem_read(ctl, peaks);
    if (err < 0) {
        kWarning() << "peak update failed" << snd_strerror(err);
    }

    for (int index = 0; index < indices.size(); index++) {
        StereoLevels level;
        level.left = snd_ctl_elem_value_get_integer(peaks, indices[index]);
        level.right = snd_ctl_elem_value_get_integer(peaks, indices[index] + 1);
        mPeaks[index] = level;
    }
    return mPeaks;
}



void EnvyCard::pulse() {

    kDebug() << "entering";

    mixerUpdatePlaybackVolume(0);
    mixerUpdatePlaybackVolume(1);
    mixerUpdatePlaybackSwitch(0);
    mixerUpdatePlaybackSwitch(1);

    mixerUpdateInputVolume(0);
    mixerUpdateInputVolume(1);
    mixerUpdateInputSwitch(0);
    mixerUpdateInputSwitch(1);

    mixerUpdateSPDIFVolume(0);
    mixerUpdateSPDIFVolume(1);
    mixerUpdateSPDIFSwitch(0);
    mixerUpdateSPDIFSwitch(1);

    dacVolumeUpdate(0);
    dacVolumeUpdate(1);

    patchbayAnalogUpdate(0);
    patchbayAnalogUpdate(1);

    patchbayDigitalUpdate(0);
    patchbayDigitalUpdate(1);

    enumConfigUpdate(HW_ENUM_INTERNAL_CLOCK);
    enumConfigUpdate(HW_ENUM_CLOCK_DEFAULT);
    enumConfigUpdate(HW_ENUM_DEEMPHASIS);
    boolConfigUpdate(HW_BOOL_RATE_LOCKING);
    boolConfigUpdate(HW_BOOL_RATE_RESET);

    kDebug() << "leaving";
}





void EnvyCard::driverEvent(int) {
    snd_ctl_event_t *ev;
    snd_ctl_event_alloca(&ev);
    if (snd_ctl_read(ctl, ev) < 0)
        return;
    
    const char* name = snd_ctl_event_elem_get_name(ev);
    int index = snd_ctl_event_elem_get_index(ev);

    kDebug() << "driverEvent: " << name << "(" << index << ")";

    unsigned int mask = snd_ctl_event_elem_get_mask(ev);
    if (! (mask & (SND_CTL_EVENT_MASK_VALUE | SND_CTL_EVENT_MASK_INFO)))
        return;
    
    kDebug() << "driverEvent: " << name << "(" << index << ")";

    switch (snd_ctl_event_elem_get_interface(ev)) {
        case SND_CTL_ELEM_IFACE_MIXER:
            if (!strcmp(name, "Multi Playback Volume"))
                mixerUpdatePlaybackVolume(index);
            else if (!strcmp(name, "H/W Multi Capture Volume"))
                mixerUpdateInputVolume(index);
            else if (!strcmp(name, "IEC958 Multi Capture Volume"))
                mixerUpdateSPDIFVolume(index);
            
            else if (!strcmp(name, "Multi Playback Switch"))
                mixerUpdatePlaybackSwitch(index);
            else if (!strcmp(name, "H/W Multi Capture Switch"))
                mixerUpdateInputSwitch(index);
            else if (!strcmp(name, "IEC958 Multi Capture Switch"))
                mixerUpdateSPDIFSwitch(index);
            
            else if (!strcmp(name, "H/W Playback Route"))
                patchbayAnalogUpdate(index);
            else if (!strcmp(name, "IEC958 Playback Route"))
                patchbayDigitalUpdate(index);
            else if (!strcmp(name, "DAC Volume"))
                dacVolumeUpdate(index);
            else if (!strcmp(name, HW_ENUM_INTERNAL_CLOCK))
                enumConfigUpdate(name);
            else if (!strcmp(name, HW_ENUM_CLOCK_DEFAULT))
                enumConfigUpdate(name);
            else if (!strcmp(name, HW_ENUM_DEEMPHASIS))
                enumConfigUpdate(name);
            else if (!strcmp(name, HW_BOOL_RATE_LOCKING))
                boolConfigUpdate(name);
            else if (!strcmp(name, HW_BOOL_RATE_RESET))
                boolConfigUpdate(name);
            // TODO: implement these cases
/*            else if (!strcmp(name, "Multi Track Volume Rate"))
            volumeChangeRateUpdate(); */
         

            break;
        default:
            break;
    }
}

void EnvyCard::mixerUpdatePlaybackVolume(int index){
    kDebug() << "entering";
    SndCtlElemValue vols(SND_CTL_ELEM_IFACE_MIXER, getSectionName(SECTION_PLAYBACK));
    snd_ctl_elem_value_set_index(&vols, index);
    readSndCtl(vols);
    int left = snd_ctl_elem_value_get_integer(&vols, 0);
    int right = snd_ctl_elem_value_get_integer(&vols, 1);
    MixerAdjustement adj = deduceMixerAdjustement((LeftRight)(index%2), left, right);
    emit mixerUpdatePlaybackVolume(index/2, (LeftRight)(index%2), adj);
    kDebug() << "leaving";
}

void EnvyCard::mixerUpdateInputVolume(int index){
    SndCtlElemValue vols(SND_CTL_ELEM_IFACE_MIXER, getSectionName(SECTION_CAPTURE));
    snd_ctl_elem_value_set_index(&vols, index);
    readSndCtl(vols);
    int left = snd_ctl_elem_value_get_integer(&vols, 0);
    int right = snd_ctl_elem_value_get_integer(&vols, 1);
    MixerAdjustement adj = deduceMixerAdjustement((LeftRight)(index%2), left, right);
    emit mixerUpdateAnalogInVolume(index/2, (LeftRight)(index%2), adj);
}

void EnvyCard::mixerUpdateSPDIFVolume(int index){
    SndCtlElemValue vols(SND_CTL_ELEM_IFACE_MIXER, getSectionName(SECTION_IEC598));
    snd_ctl_elem_value_set_index(&vols, index);
    readSndCtl(vols);
    int left = snd_ctl_elem_value_get_integer(&vols, 0);
    int right = snd_ctl_elem_value_get_integer(&vols, 1);
    MixerAdjustement adj = deduceMixerAdjustement((LeftRight)(index%2), left, right);
    emit mixerUpdateSPDIFVolume(index/2, (LeftRight)(index%2), adj);
}

void EnvyCard::dacVolumeUpdate(int index)
{
    SndCtlElemValue vols( SND_CTL_ELEM_IFACE_MIXER,  "DAC Volume" );
    snd_ctl_elem_value_set_index( &vols, index );
    readSndCtl( vols );
    int vol = snd_ctl_elem_value_get_integer( &vols, 0 );
    emit analogUpdateDACVolume((LeftRight)(index), vol);
}

void EnvyCard::mixerUpdatePlaybackSwitch(int index){
    SndCtlElemValue sw(SND_CTL_ELEM_IFACE_MIXER, MULTI_PLAYBACK_SWITCH);
    snd_ctl_elem_value_set_index(&sw, index);
    if (snd_ctl_elem_read(ctl, &sw) >= 0) {
        bool left = snd_ctl_elem_value_get_boolean(&sw, 0);
        bool right = snd_ctl_elem_value_get_boolean(&sw, 1);
        if (left != right)
            kWarning() << "Mute switches for channel " << index
                     << " do not match. Displaying state of left channel only";
        emit mixerUpdatePCMMuteSwitch(index /2, (LeftRight)(index %2), !left);
    }
}

void EnvyCard::mixerUpdateInputSwitch(int index) {
    SndCtlElemValue sw(SND_CTL_ELEM_IFACE_MIXER, HW_MULTI_CAPTURE_SWITCH);
    snd_ctl_elem_value_set_index(&sw, index);
    if (snd_ctl_elem_read(ctl, &sw) >= 0) {
        bool leftOn = snd_ctl_elem_value_get_boolean(&sw, 0);
        bool rightOn = snd_ctl_elem_value_get_boolean(&sw, 1);
        if (leftOn != rightOn)
            kWarning() << "Mute switches for channel " << index
                     << " do not match. Displaying state of left channel only";
        emit mixerUpdateAnalogInMuteSwitch(index /2, (LeftRight)(index %2), !leftOn);
    }
}

void EnvyCard::mixerUpdateSPDIFSwitch(int index){
    SndCtlElemValue sw(SND_CTL_ELEM_IFACE_MIXER, IEC958_MULTI_CAPTURE_SWITCH);
    snd_ctl_elem_value_set_index(&sw, index);
    if (snd_ctl_elem_read(ctl, &sw) >= 0) {
        bool left = snd_ctl_elem_value_get_boolean(&sw, 0);
        bool right = snd_ctl_elem_value_get_boolean(&sw, 1);
        if (left != right)
            kWarning() << "Mute switches for channel " << index
                     << " do not match. Displaying state of left channel only";
        emit mixerUpdateSPDIFInMuteSwitch(index / 2, (LeftRight)(index % 2), !left);
    }
}



void EnvyCard::patchbayAnalogUpdate(int index) {
    SndCtlElemValue val(SND_CTL_ELEM_IFACE_MIXER, ANALOG_PLAYBACK_ROUTE_NAME);
    snd_ctl_elem_value_set_index(&val, index);
    int err = snd_ctl_elem_read(ctl, &val);
    if (err < 0) {
        kWarning() << "cannot update analog route: " << snd_strerror(err);
        return;
    }

    int routeIdx = snd_ctl_elem_value_get_enumerated(&val, 0);
    kDebug() << "new route: " << mRoutes[routeIdx];
    emit analogRouteUpdated(index / 2, (LeftRight)(index % 2), mRoutes[routeIdx]);
}

void EnvyCard::patchbayDigitalUpdate(int index) {
    SndCtlElemValue val(SND_CTL_ELEM_IFACE_MIXER, SPDIF_PLAYBACK_ROUTE_NAME);
    snd_ctl_elem_value_set_index(&val, index);
    int err = snd_ctl_elem_read(ctl, &val);
    if (err < 0) {
        kWarning() << "cannot update digital route: " << snd_strerror(err);
        return;
    }

    int routeIdx = snd_ctl_elem_value_get_enumerated(&val, 0);
    kDebug() << "new route: " << mRoutes[routeIdx];
    emit digitalRouteUpdated(index / 2, (LeftRight)(index % 2), mRoutes[routeIdx]);
}

void EnvyCard::enumConfigUpdate(const char* var) {
    SndCtlElemValue cfg(SND_CTL_ELEM_IFACE_MIXER, var);
    snd_ctl_elem_value_set_index(&cfg, 0);
    int err = snd_ctl_elem_read(ctl, &cfg);
    if (err < 0) {
        kWarning() << "cannot configure " << var << " :" << snd_strerror(err);
        return;
    }

    int idx = snd_ctl_elem_value_get_enumerated(&cfg, 0);
    QString key(var);
    const QStringList& enums = mConfigEnums[key];
    kDebug() << key << " = " << enums[idx];
    emit enumConfigUpdated(key, enums[idx]);
}

void EnvyCard::boolConfigUpdate(const char* var) {
    SndCtlElemValue cfg(SND_CTL_ELEM_IFACE_MIXER, var);
    snd_ctl_elem_value_set_index(&cfg, 0);
    int err = snd_ctl_elem_read(ctl, &cfg);
    if (err < 0) {
        kWarning() << "cannot configure " << var << " :" << snd_strerror(err);
        return;
    }

    bool bvalue = snd_ctl_elem_value_get_boolean(&cfg, 0);
    QString qvar(var);
    kDebug() << qvar << " = " << bvalue;
    emit boolConfigUpdated(qvar, bvalue);
}

void EnvyCard::mixerMuteCaptureChannel(int index, LeftRight channel, bool muteOn) {
    SndCtlElemValue sw(SND_CTL_ELEM_IFACE_MIXER, HW_MULTI_CAPTURE_SWITCH);
    snd_ctl_elem_value_set_index(&sw, index *2 + (int)channel);
    snd_ctl_elem_value_set_boolean(&sw, 0, !muteOn);
    snd_ctl_elem_value_set_boolean(&sw, 1, !muteOn);
    writeSndCtl(sw);
}

void EnvyCard::mixerMutePlaybackChannel(int index, LeftRight channel, bool muteOn) {
    SndCtlElemValue sw(SND_CTL_ELEM_IFACE_MIXER, MULTI_PLAYBACK_SWITCH);
    snd_ctl_elem_value_set_index(&sw, index *2 + (int)channel);
    snd_ctl_elem_value_set_boolean(&sw, 0, !muteOn);
    snd_ctl_elem_value_set_boolean(&sw, 1, !muteOn);
    writeSndCtl(sw);
}

void EnvyCard::mixerMuteSPDIFChannel(int index, LeftRight channel, bool muteOn) {
    SndCtlElemValue sw(SND_CTL_ELEM_IFACE_MIXER, IEC958_MULTI_CAPTURE_SWITCH);
    snd_ctl_elem_value_set_index(&sw, index *2 + (int)channel);
    snd_ctl_elem_value_set_boolean(&sw, 0, !muteOn);
    snd_ctl_elem_value_set_boolean(&sw, 1, !muteOn);
    writeSndCtl(sw);
}

void EnvyCard::writeSndCtl(const SndCtlElemValue& val) {
    int err = snd_ctl_elem_write(ctl, &val);
    if (err <0) 
        qWarning("Unable to write value: %s", snd_strerror(err));
}

void EnvyCard::readSndCtl(const SndCtlElemValue& val) {
    kDebug() << "entering";
    int err = snd_ctl_elem_read(ctl, &val);
    if (err < 0) kWarning() << "Unable to read value: " << snd_strerror(err);
    kDebug() << "leaving";
}

void EnvyCard::setMixerCaptureVolume(int index, LeftRight channel, int vol, int stereo) {
    setVolume(SECTION_CAPTURE, index, channel, vol, stereo);
}

void EnvyCard::setMixerPlaybackVolume(int index, LeftRight channel, int vol, int stereo) {
    setVolume(SECTION_PLAYBACK, index, channel, vol, stereo);
}

void EnvyCard::setMixerSPDIFVolume(int index, LeftRight channel, int vol, int stereo) {
    setVolume(SECTION_IEC598, index, channel, vol, stereo);
}

void EnvyCard::setDACVolume(LeftRight leftRight, int level) {
    SndCtlElemValue val(SND_CTL_ELEM_IFACE_MIXER, "DAC Volume", (int)leftRight);
    val.setInteger(level);
    writeSndCtl(val);
}

void EnvyCard::setAnalogRoute(int index, LeftRight channel, const QString& soundSource) {
    SndCtlElemValue val(SND_CTL_ELEM_IFACE_MIXER, ANALOG_PLAYBACK_ROUTE_NAME);
    snd_ctl_elem_value_set_index(&val, 2 * index + channel);

    kDebug() << "new route: " << soundSource;
    snd_ctl_elem_value_set_enumerated(&val, 0, mRoutes.indexOf(soundSource));
    writeSndCtl(val);
}

void EnvyCard::setDigitalRoute(int index, LeftRight channel, const QString& soundSource) {
    SndCtlElemValue val(SND_CTL_ELEM_IFACE_MIXER, SPDIF_PLAYBACK_ROUTE_NAME);
    snd_ctl_elem_value_set_index(&val, 2 * index + channel);

    kDebug() << "new route: " << soundSource;
    snd_ctl_elem_value_set_enumerated(&val, 0, mRoutes.indexOf(soundSource));
    writeSndCtl(val);
}

void EnvyCard::setEnumConfig(const QString& key, const QString& value) {
    kDebug() << key << " = " << value;
    SndCtlElemValue cfg(SND_CTL_ELEM_IFACE_MIXER, key.toAscii());
    snd_ctl_elem_value_set_index(&cfg, 0);
    snd_ctl_elem_value_set_enumerated(&cfg, 0, mConfigEnums[key].indexOf(value));
    writeSndCtl(cfg);
}

void EnvyCard::setBoolConfig(const QString& key, bool value) {
    kDebug() << key << " = " << value;
    SndCtlElemValue cfg(SND_CTL_ELEM_IFACE_MIXER, key.toAscii());
    snd_ctl_elem_value_set_index(&cfg, 0);
    snd_ctl_elem_value_set_boolean(&cfg, 0, value);
    writeSndCtl(cfg);
}



void EnvyCard::setVolume(Section section, int index, LeftRight channel, int vol, int stereo)
{
    int volLeft = (channel == LEFT ? vol : stereo);
    int volRight = (channel == LEFT ? stereo : vol);

    SndCtlElemValue volume( SND_CTL_ELEM_IFACE_MIXER, getSectionName( section ) );
    snd_ctl_elem_value_set_index( &volume, index * 2 + (int)channel );
    readSndCtl( volume );
    int curVolLeft = snd_ctl_elem_value_get_integer( &volume, 0 );
    int curVolRight = snd_ctl_elem_value_get_integer( &volume, 1 );
    if ( (curVolLeft != volLeft) || (curVolRight != volRight) )
    {
        snd_ctl_elem_value_set_integer( &volume, 0, volLeft );
        snd_ctl_elem_value_set_integer( &volume, 1, volRight );
        writeSndCtl( volume );
    }
}


const char* EnvyCard::getSectionName(Section section) {
    const char* elemName = 0;
    switch (section) {
        case SECTION_CAPTURE:
            elemName = HW_MULTI_CAPTURE_VOLUME;
            break;
        case SECTION_PLAYBACK:
            elemName = MULTI_PLAYBACK_VOLUME;
            break;
        case SECTION_IEC598:
            elemName = IEC958_MULTI_CAPTURE_VOLUME;
            break;
        default:
            assert(0);
    }
    return elemName;
}

MixerAdjustement EnvyCard::getMixerAdjustement(Section sect, int index, LeftRight channel) {
    SndCtlElemValue volume(SND_CTL_ELEM_IFACE_MIXER, getSectionName(sect));
    snd_ctl_elem_value_set_index(&volume, index * 2 + (int)channel);
    readSndCtl(volume);
   
    int curLeftVol = snd_ctl_elem_value_get_integer(&volume, 0);
    int curRightVol = snd_ctl_elem_value_get_integer(&volume, 1);
    return deduceMixerAdjustement(channel, curLeftVol, curRightVol);
}

MixerAdjustement EnvyCard::deduceMixerAdjustement(LeftRight ch, int aleft, int aright) {
    MixerAdjustement result;
    
    if (ch == LEFT) {
        result.volume = aleft;
        result.stereo = aright;
    } else {
        result.volume = aright;
        result.stereo = aleft;
    }

    return result;
}


#include "envycard.moc"
