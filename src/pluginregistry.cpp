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
    QHash<QObject *, QPair<QJsonObject, QPluginLoader *> > plugins;
    foreach (const QStaticPlugin &plugin, QPluginLoader::staticPlugins()) {
        QObject *pluginInstance = plugin.instance();

        plugins[pluginInstance] = QPair<QJsonObject, QPluginLoader *>(plugin.metaData(), nullptr);
    }

    // Load dynamic plugins
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);

        QPluginLoader *loader = new QPluginLoader(pluginsDir.absoluteFilePath(
                                                      settings.value("filename").toString()));
        QObject *pluginInstance = loader->instance();

        plugins[pluginInstance] = QPair<QJsonObject, QPluginLoader *>(loader->metaData(), loader);
    }

    // Activate plugins
    QHashIterator<QObject *, QPair<QJsonObject, QPluginLoader *>> i(plugins);
    while (i.hasNext()) {
        i.next();

        QObject *instance = i.key();

        QJsonObject metaData = i.value().first["MetaData"].toObject();
        QPluginLoader *loader = i.value().second;

        PostMonster::ToolPluginInterface *tool = qobject_cast<PostMonster::ToolPluginInterface *>(instance);
        if (tool) {
            PluginData *pluginData = new PluginData;
            pluginData->type = PostMonster::Tool;
            pluginData->instance = tool;
            pluginData->loader = loader;
            pluginData->info = metaData;

            //TODO Check for existent plugins with the same id
            m_plugins[metaData["id"].toString()] = pluginData;
            m_info[tool] = &pluginData->info;

            // Add tool plugins to toolbar
            tool->load(api);

            emit toolPluginLoaded(instance);
        }
    }
}

PostMonster::ToolPluginInterface *PluginRegistry::tool(const QString &name)
{
    if (m_plugins.contains(name) && m_plugins[name]->type == PostMonster::Tool) {
        return dynamic_cast<PostMonster::ToolPluginInterface *>(m_plugins[name]->instance);
    }

    return nullptr;
}

const QJsonObject &PluginRegistry::info(const PostMonster::PluginInterface *plugin)
{
    return *m_info[plugin];
}

const QList<PluginRegistry::PluginData *> PluginRegistry::plugins(PostMonster::PluginType type)
{
    QList<PluginData *> result;
    foreach (PluginData *plugin, m_plugins.values()) {
        if (plugin->type & type)
            result << plugin;
    }

    return result;
}

PluginRegistry::~PluginRegistry()
{
    qDeleteAll(m_plugins.values());
}
