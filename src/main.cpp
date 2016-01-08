/* PostMonster, universal HTTP automation tool
 * Copyright (C) 2015 by Paul Artsishevsky <polter.rnd@gmail.com>
 *
 * This file is part of PostMonster.
 *
 * PostMonster is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PostMonster is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PostMonster.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"

#include <QtPlugin>
#include <QApplication>
#include <QNetworkProxyFactory>

Q_IMPORT_PLUGIN(HttpToolPlugin)

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setOrganizationDomain("postmonster.org");
    QApplication::setApplicationName("PostMonster");

    QNetworkProxyFactory::setUseSystemConfiguration(true);

    MainWindow mainWin;
    mainWin.show();

    return app.exec();
}
