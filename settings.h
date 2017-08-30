#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

class Settings: public QSettings
{
public:
    Settings();

    int get(const QString& key, int def);

    ~Settings();

};

#endif // SETTINGS_H
