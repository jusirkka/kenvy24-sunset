#include "envycard.h"

#include <assert.h>
#include <QSocketNotifier>
#include <algorithm>
#include <math.h>
#include <QDebug>

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

#define PEAK_CONTROL_NAME "Multi Track Peak"

ChannelState StereoLevels::state(Position k) const {
    if (k == Position::Left) {
        return ChannelState(left, right);
    }
    return ChannelState(right, left);
}

StereoLevels ChannelState::levels(Position k) const {
       if (k == Position::Left) {
           return StereoLevels(volume, stereo);
       }
       return StereoLevels(stereo, volume);
}




EnvyCard::EnvyCard():
    QObject(),
    mDummyPeak(0)
{


    // Routing tables
    mAddressBase[MULTI_PLAYBACK_VOLUME] = 0;
    mAddressBase[HW_MULTI_CAPTURE_VOLUME] = 10;
    mAddressBase[IEC958_MULTI_CAPTURE_VOLUME] = 12;
    mAddressBase[MULTI_PLAYBACK_SWITCH] = 0;
    mAddressBase[HW_MULTI_CAPTURE_SWITCH] = 10;
    mAddressBase[IEC958_MULTI_CAPTURE_SWITCH] = 12;
    mAddressBase[ANALOG_PLAYBACK_ROUTE_NAME] = 30;
    mAddressBase[SPDIF_PLAYBACK_ROUTE_NAME] = 32;
    mAddressBase[DAC_VOLUME_NAME] = 20;

    mSectionBase[0] = MULTI_PLAYBACK_VOLUME;
    mSectionBase[10] = HW_MULTI_CAPTURE_VOLUME;
    mSectionBase[12] = IEC958_MULTI_CAPTURE_VOLUME;
    mSectionBase[30] = ANALOG_PLAYBACK_ROUTE_NAME;
    mSectionBase[32] = SPDIF_PLAYBACK_ROUTE_NAME;
    mSectionBase[20] = DAC_VOLUME_NAME;

    mMuteSwitchSectionBase[0] = MULTI_PLAYBACK_SWITCH;
    mMuteSwitchSectionBase[10] = HW_MULTI_CAPTURE_SWITCH;
    mMuteSwitchSectionBase[12] = IEC958_MULTI_CAPTURE_SWITCH;
}


EnvyCard::~EnvyCard() {
}


EnvyCard& EnvyCard::Instance() {
    static EnvyCard card;
    return card;
}


void EnvyCard::configAddresses(const QList<int>& channels) {
    mPeaks.clear();
    StereoLevels nosound(0, 0);
    foreach (int channel, channels) {
        mPeaks[channel] = nosound;
    }
}

int EnvyCard::PCMAddress(int index) const {
    return mAddressBase[MULTI_PLAYBACK_VOLUME] + 2 * index;
}

int EnvyCard::AnalogInAddress() const {
    return mAddressBase[HW_MULTI_CAPTURE_VOLUME];
}

int EnvyCard::DigitalInAddress() const {
    return mAddressBase[IEC958_MULTI_CAPTURE_VOLUME];
}

int EnvyCard::AnalogOutAddress() const {
    return mAddressBase[ANALOG_PLAYBACK_ROUTE_NAME];
}

int EnvyCard::DigitalOutAddress() const {
    return mAddressBase[SPDIF_PLAYBACK_ROUTE_NAME];
}

int EnvyCard::DACAddress() const {
    return mAddressBase[DAC_VOLUME_NAME];
}


const EnvyCard::PeakMap& EnvyCard::peaks() {


    foreach (int addr, mPeaks.keys()) {
        mDummyPeak = (mDummyPeak + 1) % 255;
        StereoLevels level(mDummyPeak, mDummyPeak);
        mPeaks[addr] = level;
    }
    return mPeaks;
}



void EnvyCard::pulse() {}




// ---------------------------------------------------------------------------
// Service Interface
// ---------------------------------------------------------------------------

void EnvyCard::on_slot_mixerVolumeChanged(int , Position , const ChannelState& ) {
}


void EnvyCard::on_slot_mixerMuteSwitchChanged(int , Position , bool ) {
}


void EnvyCard::on_slot_mixerRouteChanged(int , Position , const QString& ) {
}


void EnvyCard::on_slot_masterVolumeChanged(Position , int ) {
}

void EnvyCard::on_slot_boolConfigChanged(const QString& , bool ) {
}

void EnvyCard::on_slot_enumConfigChanged(const QString& , const QString& ) {
}
