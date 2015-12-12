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
 
#ifndef PLUGINSDIALOG_H
#define PLUGINSDIALOG_H

#include <QDialog>

namespace Ui {
class PluginsDialog;
}

class PluginsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PluginsDialog(QWidget *parent = 0);
    ~PluginsDialog();

private:
    Ui::PluginsDialog *ui;

    enum Columns
    {
        Library,
        Name,
        Description,
        Version
    };

    void populateTable(const QString &file, const QString &name, const QString &descr, const QString &ver);

private slots:
    void save();
};

#endif // PLUGINSDIALOG_H
