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
 
#include "pluginsdialog.h"
#include "ui_pluginsdialog.h"

#include <QtDebug>
#include <QDir>
#include <QPluginLoader>
#include <QSettings>

PluginsDialog::PluginsDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint),
    ui(new Ui::PluginsDialog)
{
    ui->setupUi(this);

    QDir pluginsDir = QDir::current();
    pluginsDir.cd("plugins");

    int i = 0;
    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QJsonObject info = loader.metaData().value("MetaData").toObject();

        if (!info.isEmpty()) {
            QTableWidgetItem *library = new QTableWidgetItem(fileName);
            QTableWidgetItem *name = new QTableWidgetItem(info.value("name").toString());
            QTableWidgetItem *descr = new QTableWidgetItem(info.value("description").toString());
            QTableWidgetItem *version = new QTableWidgetItem(info.value("version").toString());

            library->setCheckState(Qt::Checked);

            ui->tableWidget->setRowCount(++i);
            ui->tableWidget->setItem(i - 1, Library, library);
            ui->tableWidget->setItem(i - 1, Name, name);
            ui->tableWidget->setItem(i - 1, Description, descr);
            ui->tableWidget->setItem(i - 1, Version, version);
        }
    }

    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(Description, QHeaderView::Stretch);

    connect(this, SIGNAL(accepted()), this, SLOT(save()));
}

void PluginsDialog::save()
{
    QSettings settings;
    settings.beginWriteArray("Plugins");
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        QTableWidgetItem *library = ui->tableWidget->item(i, Library);

        settings.setArrayIndex(i);
        settings.setValue("filename", library->text());
        settings.setValue("enabled", (library->checkState() == Qt::Checked)
                          ? "true" : "false");
    }
    settings.endArray();
}

PluginsDialog::~PluginsDialog()
{
    delete ui;
}
