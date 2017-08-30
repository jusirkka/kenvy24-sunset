#include <QTimer>
#include <QApplication>
#include <QCommandLineParser>
#include <QMessageBox>
#include <QDBusConnection>


#include "mixer_adaptor.h"
#include "tray_adaptor.h"
#include "mainwindow.h"
#include "envycard.h"

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    app.setOrganizationName("Kvanttiapina");
    app.setApplicationName("IceMixer");
    app.setApplicationVersion("1.0");

    try {

        QCommandLineParser parser;
        parser.setApplicationDescription("Mixer application for envy24 sound cards");
        parser.addHelpOption();
        parser.addVersionOption();

        QCommandLineOption dockedOption("docked", "Hide app in the system tray");
        parser.addOption(dockedOption);

        parser.process(app);

        bool docked = parser.isSet(dockedOption);

        QDBusConnection conn = QDBusConnection::sessionBus();
        if (!conn.registerService("net.kvanttiapina.envy24")) {
            QString action = "restoreWindow";
            if (docked) {
                action = "minimizeWindow";
            }
            conn.send(QDBusMessage::createMethodCall("net.kvanttiapina.envy24", "/Mixer", "", action));
            return 0;
        }


        MainWindow* mw = new MainWindow(docked);
        new MixerAdaptor(mw);
        new TrayAdaptor(mw);
        conn.registerObject("/Mixer", mw);

        return app.exec();
    } catch (CardNotFound) {
        QMessageBox::critical(0, "Fatal error", "Cannot find an Envy24 chip based sound card in your system. Will now quit!");
        QTimer::singleShot(500, qApp, SLOT(quit()));
    }

}

