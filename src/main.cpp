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


#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include "kenvy24.h"
#include "version.h"


static const char description[] =
    I18N_NOOP("VIA Envy24 based sound cards control utility. See xxx for the documentation");



int main(int argc, char **argv)
{
    kDebug() << "starting";
    KCmdLineOptions options;
    KAboutData about(
        "kenvy24-sunset", 0,
        ki18n("KEnvy24Sunset"), VERSION,
        ki18n("The ICE1712 based cards KDE control panel"), KAboutData::License_GPL,
        ki18n("(C) 2015 Jukka Sirkka"),
        ki18n("Feedback:\njukka.sirkka@iki.fi\n\n(Build Date: " __DATE__ ")"),
        "http://www.github.com/jusirkka/kenvy24-sunset"
    );
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);

   if (!KEnvy24App::start()) return 0;

   KEnvy24App *app = new KEnvy24App();
   int ret = app->exec();
   delete app;
   return ret;
}

