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
 
#include "pluginregistry.h"

#include <QSettings>
#include <QPluginLoader>
#include <QDir>

PluginRegistry &PluginRegistry::instance()
{
    static PluginRegistry m_instance;
    return m_instance;
}

void PluginRegistry::loadPlugins(const PostMonster::APIFunctions &api)
{
    QDir pluginsDir = QDir::current();
    pluginsDir.cd("plugins");

    QSettings settings;
    int size = settings.beginReadArray("Plugins");

    // Load static plugins
    QMap<QObject *, QJsonObject> plugins;
    foreach (QStaticPlugin plugin, QPluginLoader::staticPlugins())
        plugins[plugin.instance()] = plugin.metaData();

    // Load dynamic plugins
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);

        QPluginLoader loader(pluginsDir.absoluteFilePath(settings.value("filename").toString()));
        plugins[loader.instance()] = loader.metaData();
    }

    // Activate plugins
    QMapIterator<QObject *, QJsonObject> i(plugins);
    while (i.hasNext()) {
        i.next();
        QObject *plugin = i.key();
        QJsonObject metaData = i.value().value("MetaData").toObject();

        // Add tool plugins to toolbar
        PostMonster::ToolPluginInterface *tool = qobject_cast<PostMonster::ToolPluginInterface *>(plugin);
        if (tool) {
            //TODO Check for existent plugins with the same id
            m_tools[metaData.value("id").toString()] = tool;
            m_metaData[tool] = metaData;

            tool->load(api);

            emit toolPluginLoaded(plugin);
        }
    }
}

PostMonster::ToolPluginInterface *PluginRegistry::tool(const QString &name)
{
    if (m_tools.contains(name))
        return m_tools[name];

    return 0;
}

const QJsonObject &PluginRegistry::info(const PostMonster::PluginInterface *plugin)
{
    return m_metaData[const_cast<PostMonster::PluginInterface *>(plugin)];
}
