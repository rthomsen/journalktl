/*******************************************************************************
 * Copyright (C) 2015 Ragnar Thomsen <rthomsen6@gmail.com>                     *
 *                                                                             *
 * This program is free software: you can redistribute it and/or modify it     *
 * under the terms of the GNU General Public License as published by the Free  *
 * Software Foundation, either version 2 of the License, or (at your option)   *
 * any later version.                                                          *
 *                                                                             *
 * This program is distributed in the hope that it will be useful, but WITHOUT *
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for    *
 * more details.                                                               *
 *                                                                             *
 * You should have received a copy of the GNU General Public License along     *
 * with this program. If not, see <http://www.gnu.org/licenses/>.              *
 *******************************************************************************/

#include "mainwindow.h"
#include <QApplication>
#include <KAboutData>
#include <KLocalizedString>
#include <config.h>

int main(int argc, char *argv[])
{
  KLocalizedString::setApplicationDomain("Journalktl");

  KAboutData aboutData( QStringLiteral("Journalktl"), i18n("Journalktl"),
      JOURNALKTL_VERSION, i18n("Journald log viewer"), KAboutLicense::GPL,
      i18n("(c) 2015, Ragnar Thomsen"));
  aboutData.addAuthor("Ragnar Thomsen", i18n("Main Developer"), "rthomsen6@gmail.com");
  KAboutData::setApplicationData(aboutData);

  QApplication app(argc, argv);
  MainWindow* window = new MainWindow();
  window->show();
  return app.exec();
}
