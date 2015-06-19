#ifndef KEnvy24App_h
#define KEnvy24App_h

#include <kuniqueapplication.h>

class KEnvy24Window;
class DBusIface;

class KEnvy24App : public KUniqueApplication
{
    Q_OBJECT
    public:
        KEnvy24App();
        virtual ~KEnvy24App();
        int newInstance ();

    public slots:

    signals:
        void stopUpdatesOnVisibility();

    private:
        KEnvy24Window *mEnvy;
        DBusIface *mDBus;
};

#endif // Kenvy24App_h
