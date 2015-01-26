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

#include "kenvy24app.h"
#include "version.h"

static const char description[] =
    I18N_NOOP("VIA Envy24 based sound cards control utility. See http://kenvy24.wiki.sourceforge.net/ for the documentation");

static const char version[] = VERSION;

static KCmdLineOptions options[] = {KCmdLineLastOption};

int main(int argc, char **argv)
{
    KAboutData about("kenvy24", I18N_NOOP("kenvy24"), version, description,
		     KAboutData::License_GPL, "(C) 2007 Valentin Rusu", 0, 0, "kenvy24@rusu.info");
    about.addAuthor("Valentin Rusu", 0, "kenvy24@rusu.info");
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);

   if (!KEnvy24App::start()) return 0;

   KEnvy24App *app = new KEnvy24App();
   int ret = app->exec();
   delete app;
   return ret;
}
