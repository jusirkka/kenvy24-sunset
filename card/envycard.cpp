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


EnvyCard::ControlElement::ControlElement(_snd_ctl_elem_iface iface, const QString& name) {
  snd_ctl_elem_value_malloc(&element);
  snd_ctl_elem_value_set_interface(element, iface);
  snd_ctl_elem_value_set_name(element, name.toLatin1().constData());
}
                                          
EnvyCard::ControlElement::ControlElement(_snd_ctl_elem_iface iface, const QString& name, int index) {
    snd_ctl_elem_value_malloc(&element);
    snd_ctl_elem_value_set_interface(element, iface);
    snd_ctl_elem_value_set_name(element, name.toLatin1().constData());
    snd_ctl_elem_value_set_index(element, index);
}

EnvyCard::ControlElement::~ControlElement() {
    snd_ctl_elem_value_free(element);
}

int EnvyCard::ControlElement::get(int index) const {
    return snd_ctl_elem_value_get_integer(element, index);
}
void EnvyCard::ControlElement::set(int index, int value) const {
    snd_ctl_elem_value_set_integer(element, index, value);
}


int EnvyCard::ControlElement::write(snd_ctl_t* card) const {
    qDebug() << "entering";
    int err = snd_ctl_elem_write(card, element);
    if (err < 0) qWarning() << "Unable to write value: " << snd_strerror(err);
    qDebug() << "leaving";
    return err;
}

int EnvyCard::ControlElement::read(snd_ctl_t* card) const {
    // kDebug() << "entering";
    int err = snd_ctl_elem_read(card, element);
    if (err < 0) qWarning() << "Unable to read value: " << snd_strerror(err);
    // kDebug() << "leaving";
    return err;
}



EnvyCard::EnvyCard():
    QObject(),
    mRawPeaks(SND_CTL_ELEM_IFACE_PCM, PEAK_CONTROL_NAME)
{

    // Initialise Card
    snd_ctl_card_info_t *hw_info;
    snd_ctl_card_info_alloca(&hw_info);
    mCard = 0;
    for (int index=0; index < 8; index++) {
        QString name = QString("hw:%1").arg(index);
        int ok = snd_ctl_open(&mCard, name.toLatin1().constData(), 0);
        if (ok < 0) continue;
        ok = snd_ctl_card_info(mCard, hw_info);
        if (ok < 0) {
            snd_ctl_close(mCard);
            mCard = 0;
            continue;
        }
        QString driver = snd_ctl_card_info_get_driver(hw_info);
        if (driver != "ICE1712") {
            snd_ctl_close(mCard);
            mCard = 0;
            continue;
        }
        /* found */
        break;
    }

    if (!mCard) throw CardNotFound();

    snd_ctl_elem_info_t *info;
    snd_ctl_elem_info_alloca(&info);
    snd_ctl_elem_info_set_interface(info, SND_CTL_ELEM_IFACE_MIXER);

    // Patchbay
    snd_ctl_elem_info_set_name(info, ANALOG_PLAYBACK_ROUTE_NAME);
    snd_ctl_elem_info_set_numid(info, 0);
    snd_ctl_elem_info_set_index(info, 0);
    snd_ctl_elem_info(mCard, info);
    int numRoutes = snd_ctl_elem_info_get_items(info);
    for (int i = 0; i < numRoutes; i++) {
        snd_ctl_elem_info_set_item(info, i);
        snd_ctl_elem_info(mCard, info);
        mRoutes << QString(snd_ctl_elem_info_get_item_name(info));
        qDebug() << i << ": " << mRoutes[i];
    }


    // Configuration items
    QStringList keys;
    keys << QString(HW_ENUM_INTERNAL_CLOCK) << QString(HW_ENUM_CLOCK_DEFAULT) << QString(HW_ENUM_DEEMPHASIS);

    foreach (QString key, keys) {
        QStringList enums;
        snd_ctl_elem_info_set_name(info, key.toLatin1());
        snd_ctl_elem_info_set_numid(info, 0);
        snd_ctl_elem_info_set_index(info, 0);
        snd_ctl_elem_info(mCard, info);
        int numItems = snd_ctl_elem_info_get_items(info);
        for (int i = 0; i < numItems; i++) {
            snd_ctl_elem_info_set_item(info, i);
            snd_ctl_elem_info(mCard, info);
            enums << QString(snd_ctl_elem_info_get_item_name(info));
            qDebug() << i << ": " << (enums)[i];
        }
        mConfigEnums[key] = enums;
    }


    // Event Listener

    mHandlers[MULTI_PLAYBACK_VOLUME] = &EnvyCard::on_event_mixerVolumeChanged;
    mHandlers[HW_MULTI_CAPTURE_VOLUME] = &EnvyCard::on_event_mixerVolumeChanged;
    mHandlers[IEC958_MULTI_CAPTURE_VOLUME] = &EnvyCard::on_event_mixerVolumeChanged;

    mHandlers[MULTI_PLAYBACK_SWITCH] = &EnvyCard::on_event_mixerMuteSwitchChanged;
    mHandlers[HW_MULTI_CAPTURE_SWITCH] = &EnvyCard::on_event_mixerMuteSwitchChanged;
    mHandlers[IEC958_MULTI_CAPTURE_SWITCH] = &EnvyCard::on_event_mixerMuteSwitchChanged;


    mHandlers[ANALOG_PLAYBACK_ROUTE_NAME] = &EnvyCard::on_event_mixerRouteChanged;
    mHandlers[SPDIF_PLAYBACK_ROUTE_NAME] = &EnvyCard::on_event_mixerRouteChanged;

    mHandlers[DAC_VOLUME_NAME] = &EnvyCard::on_event_masterVolumeChanged;

    mHandlers[HW_ENUM_INTERNAL_CLOCK] = &EnvyCard::on_event_enumConfigChanged;
    mHandlers[HW_ENUM_CLOCK_DEFAULT] = &EnvyCard::on_event_enumConfigChanged;
    mHandlers[HW_ENUM_DEEMPHASIS] = &EnvyCard::on_event_enumConfigChanged;

    mHandlers[HW_BOOL_RATE_LOCKING] = &EnvyCard::on_event_boolConfigChanged;
    mHandlers[HW_BOOL_RATE_RESET] = &EnvyCard::on_event_boolConfigChanged;

    /* TODO "Multi Track Volume Rate" */


    int npfds = snd_ctl_poll_descriptors_count(mCard);
    if (npfds > 0) {
        struct pollfd* pfds = (pollfd*)alloca(sizeof(*pfds) * npfds);
        npfds = snd_ctl_poll_descriptors(mCard, pfds, npfds);
        for (int i = 0; i < npfds; i++) {
            qDebug() << "creating event notifier for fd = " << pfds[i].fd;
            QSocketNotifier* eventNotifier = new QSocketNotifier(pfds[i].fd, QSocketNotifier::Read);
            connect(eventNotifier, SIGNAL(activated(int)), SLOT(driverEvent(int)));
            mEventNotifiers.push_back(eventNotifier);
        }
        snd_ctl_subscribe_events(mCard, 1);
    }

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
    snd_ctl_close(mCard);
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
    if (mRawPeaks.read(mCard) < 0) return mPeaks;

    foreach (int addr, mPeaks.keys()) {
        StereoLevels level(mRawPeaks.get(addr), mRawPeaks.get(addr+1));
        mPeaks[addr] = level;
    }
    return mPeaks;
}



void EnvyCard::pulse() {

    qDebug() << "entering";



    for (int index = 0; index < MAX_MULTI_CHANNELS; index++) {
        on_event_mixerVolumeChanged(MULTI_PLAYBACK_VOLUME, 2 * index);
        on_event_mixerVolumeChanged(MULTI_PLAYBACK_VOLUME, 2 * index + 1);
        on_event_mixerMuteSwitchChanged(MULTI_PLAYBACK_SWITCH, 2 * index);
        on_event_mixerMuteSwitchChanged(MULTI_PLAYBACK_SWITCH, 2 * index + 1);
    }

    on_event_mixerVolumeChanged(HW_MULTI_CAPTURE_VOLUME, 0);
    on_event_mixerVolumeChanged(HW_MULTI_CAPTURE_VOLUME, 1);
    on_event_mixerMuteSwitchChanged(HW_MULTI_CAPTURE_SWITCH, 0);
    on_event_mixerMuteSwitchChanged(HW_MULTI_CAPTURE_SWITCH, 1);

    on_event_mixerVolumeChanged(IEC958_MULTI_CAPTURE_VOLUME, 0);
    on_event_mixerVolumeChanged(IEC958_MULTI_CAPTURE_VOLUME, 1);
    on_event_mixerMuteSwitchChanged(IEC958_MULTI_CAPTURE_SWITCH, 0);
    on_event_mixerMuteSwitchChanged(IEC958_MULTI_CAPTURE_SWITCH, 1);


    on_event_masterVolumeChanged(DAC_VOLUME_NAME, 0);
    on_event_masterVolumeChanged(DAC_VOLUME_NAME, 1);

    on_event_mixerRouteChanged(ANALOG_PLAYBACK_ROUTE_NAME, 0);
    on_event_mixerRouteChanged(ANALOG_PLAYBACK_ROUTE_NAME, 1);

    on_event_mixerRouteChanged(SPDIF_PLAYBACK_ROUTE_NAME, 0);
    on_event_mixerRouteChanged(SPDIF_PLAYBACK_ROUTE_NAME, 1);

    on_event_enumConfigChanged(HW_ENUM_INTERNAL_CLOCK, 0);
    on_event_enumConfigChanged(HW_ENUM_CLOCK_DEFAULT, 0);
    on_event_enumConfigChanged(HW_ENUM_DEEMPHASIS, 0);
    on_event_boolConfigChanged(HW_BOOL_RATE_LOCKING, 0);
    on_event_boolConfigChanged(HW_BOOL_RATE_RESET, 0);

    qDebug() << "leaving";
}




// ---------------------------------------------------------------------------
// Event Interface
// ---------------------------------------------------------------------------



void EnvyCard::driverEvent(int) {
    snd_ctl_event_t *ev;
    snd_ctl_event_alloca(&ev);
    if (snd_ctl_read(mCard, ev) < 0)
        return;

    const QString& name = snd_ctl_event_elem_get_name(ev);
    int index = snd_ctl_event_elem_get_index(ev);

    qDebug() << "driverEvent: " << name << "(" << index << ")";

    unsigned int mask = snd_ctl_event_elem_get_mask(ev);
    if (!(mask & (SND_CTL_EVENT_MASK_VALUE | SND_CTL_EVENT_MASK_INFO)))
        return;

    qDebug() << "driverEvent: " << name << "(" << index << ")";

    switch (snd_ctl_event_elem_get_interface(ev)) {
    case SND_CTL_ELEM_IFACE_MIXER: {
        if (!mHandlers.contains(name)) {
            qWarning() << name << "is not a supported event";
            return;
        }
        EventHandler handler = mHandlers[name];
        (this->*handler)(name, index);
        break;
    }
    default: {} // noop
    }
}




void EnvyCard::on_event_mixerVolumeChanged(const QString& section, int index) {
    qDebug() << "entering";
    Position k = (Position) (index % 2);
    ControlElement vols(SND_CTL_ELEM_IFACE_MIXER, section, index);
    if (vols.read(mCard) < 0) return;
    ChannelState state = StereoLevels(vols.get(0), vols.get(1)).state(k);
    emit mixerVolumeChanged(Address(section, index), k, state);
    qDebug() << "leaving";
}



void EnvyCard::on_event_mixerMuteSwitchChanged(const QString& section, int index) {
    ControlElement sw(SND_CTL_ELEM_IFACE_MIXER, section, index);
    if (sw.read(mCard) < 0) return;
    bool left = snd_ctl_elem_value_get_boolean(&sw, 0);
    bool right = snd_ctl_elem_value_get_boolean(&sw, 1);
    if (left != right)
        qWarning() << "Mute switches for channel " << index << " do not match. Displaying state of left channel only";
    emit mixerMuteSwitchChanged(Address(section, index), (Position)(index % 2), !left);
}

void EnvyCard::on_event_mixerRouteChanged(const QString& section, int index) {
    ControlElement val(SND_CTL_ELEM_IFACE_MIXER, section, index);
    if (val.read(mCard) < 0) return;

    int routeIdx = snd_ctl_elem_value_get_enumerated(&val, 0);
    qDebug() << "new route: " << mRoutes[routeIdx];
    emit mixerRouteChanged(Address(section, index), (Position)(index % 2), mRoutes[routeIdx]);
}


void EnvyCard::on_event_masterVolumeChanged(const QString& section, int index)
{
    ControlElement vols(SND_CTL_ELEM_IFACE_MIXER, section, index);
    if (vols.read(mCard) < 0) return;
    emit masterVolumeChanged((Position)(index % 2), vols.get(0));
}


void EnvyCard::on_event_enumConfigChanged(const QString& var, int index) {
    ControlElement cfg(SND_CTL_ELEM_IFACE_MIXER, var, index);
    if (cfg.read(mCard) < 0) return;

    int idx = snd_ctl_elem_value_get_enumerated(&cfg, 0);
    const QStringList& enums = mConfigEnums[var];
    qDebug() << var << " = " << enums[idx];
    emit enumConfigChanged(var, enums[idx]);
}

void EnvyCard::on_event_boolConfigChanged(const QString& var, int index) {
    ControlElement cfg(SND_CTL_ELEM_IFACE_MIXER, var, index);
    if (cfg.read(mCard) < 0) return;

    bool bvalue = snd_ctl_elem_value_get_boolean(&cfg, 0);
    qDebug() << var << " = " << bvalue;
    emit boolConfigChanged(var, bvalue);
}

// ---------------------------------------------------------------------------
// Service Interface
// ---------------------------------------------------------------------------

void EnvyCard::on_slot_mixerVolumeChanged(int source, Position channel, const ChannelState& levels) {

    int left = (channel == Position::Left ? levels.volume : levels.stereo);
    int right = (channel == Position::Left ? levels.stereo : levels.volume);

    const QString& section = Section(source);
    int index = Index(source, channel);
    ControlElement vol(SND_CTL_ELEM_IFACE_MIXER, section, index);
    vol.set(0, left);
    vol.set(1, right);
    vol.write(mCard);
}


void EnvyCard::on_slot_mixerMuteSwitchChanged(int source, Position channel, bool muteOn) {
    const QString& section = MuteSwitchSection(source);
    int index = Index(source, channel);
    ControlElement sw(SND_CTL_ELEM_IFACE_MIXER, section, index);
    snd_ctl_elem_value_set_boolean(&sw, 0, !muteOn);
    snd_ctl_elem_value_set_boolean(&sw, 1, !muteOn);
    sw.write(mCard);
}


void EnvyCard::on_slot_mixerRouteChanged(int sink, Position channel, const QString& soundSource) {
    const QString& section = Section(sink);
    int index = Index(sink, channel);
    ControlElement val(SND_CTL_ELEM_IFACE_MIXER, section, index);
    qDebug() << "new route: " << soundSource;
    snd_ctl_elem_value_set_enumerated(&val, 0, mRoutes.indexOf(soundSource));
    val.write(mCard);
}


void EnvyCard::on_slot_masterVolumeChanged(Position k, int level) {
    ControlElement val(SND_CTL_ELEM_IFACE_MIXER, DAC_VOLUME_NAME, (int)k);
    val.set(0, level);
    val.write(mCard);
}

void EnvyCard::on_slot_boolConfigChanged(const QString& key, bool value) {
    qDebug() << key << " = " << value;
    ControlElement cfg(SND_CTL_ELEM_IFACE_MIXER, key, 0);
    snd_ctl_elem_value_set_boolean(&cfg, 0, value);
    cfg.write(mCard);
}

void EnvyCard::on_slot_enumConfigChanged(const QString& key, const QString& value) {
    qDebug() << key << " = " << value;
    ControlElement cfg(SND_CTL_ELEM_IFACE_MIXER, key, 0);
    snd_ctl_elem_value_set_enumerated(&cfg, 0, mConfigEnums[key].indexOf(value));
    cfg.write(mCard);
}

int EnvyCard::Address(const QString& section, int index) const {
    return mAddressBase[section] + 2 * (index / 2);
}

QString EnvyCard::Section(int address) {
    if (address < 10) return mSectionBase[0];
    if (address < 12) return mSectionBase[10];
    if (address < 20) return mSectionBase[12];
    if (address < 30) return mSectionBase[20];
    if (address < 32) return mSectionBase[30];
    return mSectionBase[32];
}

QString EnvyCard::MuteSwitchSection(int address) {
    if (address < 10) return mMuteSwitchSectionBase[0];
    if (address < 12) return mMuteSwitchSectionBase[10];
    return mMuteSwitchSectionBase[12];
}


int EnvyCard::Index(int address, Position channel) const {
    if (address < 10) return address + (int) channel;
    if (address < 12) return address - 10 + (int) channel;
    if (address < 20) return address - 12 + (int) channel;
    if (address < 30) return address - 20 + (int) channel;
    if (address < 32) return address - 30 + (int) channel;
    return address - 32 + (int) channel;
}

