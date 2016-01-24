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

#include "pluginregistry.h"
#include "widgets/checkboxdelegate.h"

PluginsDialog::PluginsDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint),
    ui(new Ui::PluginsDialog)
{
    ui->setupUi(this);

    QDir pluginsDir = QDir::current();
    pluginsDir.cd("plugins");

    int i = 0;
    PluginRegistry &plugins = PluginRegistry::instance();

    foreach (const PluginRegistry::PluginData *plugin, plugins.plugins()) {
        m_model.add(plugin, true);
    }

    //ui->pluginsTree->set
    /*foreach (const PluginRegistry::PluginData<PostMonster::ToolPluginInterface> *tool, plugins.tools()) {
        QTableWidgetItem *id      = new QTableWidgetItem(tool->info["id"].toString());
        QTableWidgetItem *name    = new QTableWidgetItem(tool->info["name"].toString());
        QTableWidgetItem *descr   = new QTableWidgetItem(tool->info["description"].toString());
        QTableWidgetItem *version = new QTableWidgetItem(tool->info["version"].toString());

        name->setCheckState(Qt::Checked);
        version->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        ui->tableWidget->setRowCount(++i);
        ui->tableWidget->setItem(i - 1, Id, id);
        ui->tableWidget->setItem(i - 1, Name, name);
        ui->tableWidget->setItem(i - 1, Description, descr);
        ui->tableWidget->setItem(i - 1, Version, version);
    }

    ui->tableWidget->hideColumn(Id);
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(Description, QHeaderView::Stretch);*/

    ui->pluginsTable->setModel(&m_model);
    ui->pluginsTable->resizeColumnsToContents();
    ui->pluginsTable->horizontalHeader()->setSectionResizeMode(PluginsModel::Description,
                                                               QHeaderView::Stretch);

    CheckBoxDelegate *delegate = new CheckBoxDelegate(ui->pluginsTable);
    ui->pluginsTable->setItemDelegateForColumn(PluginsModel::Enabled, delegate);

    connect(this, SIGNAL(accepted()), this, SLOT(save()));
}

void PluginsDialog::save()
{
    QSettings settings;
    settings.beginWriteArray("Plugins");
    /*for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        QTableWidgetItem *name = ui->tableWidget->item(i, Name);
        QTableWidgetItem *id = ui->tableWidget->item(i, Id);

        settings.setArrayIndex(i);
        settings.setValue("filename", name->text());
        settings.setValue("enabled", (name->checkState() == Qt::Checked)
                          ? "true" : "false");
    }*/
    settings.endArray();
}

PluginsDialog::~PluginsDialog()
{
    delete ui;
}
