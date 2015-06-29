#ifndef KEnvy24App_h
#define KEnvy24App_h

#include <kuniqueapplication.h>

class MainWindow;
class DBusIface;

class KEnvy24App : public KUniqueApplication
{
    Q_OBJECT
    public:
        KEnvy24App(bool docked);
        virtual ~KEnvy24App();
        int newInstance ();

    public slots:

    signals:
        void stopUpdatesOnVisibility();

    private:
        MainWindow *mEnvy;
        DBusIface *mDBus;
        bool mDocked;
};

#endif // Kenvy24App_h
