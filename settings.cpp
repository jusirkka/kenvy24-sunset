#include "settings.h"

Settings::Settings():QSettings()
{}

int Settings::get(const QString &key, int def) {
    bool ok;
    int v = value(key, def).toInt(&ok);
    if (!ok) {
        return def;
    }
    return v;
}

Settings::~Settings() {}
