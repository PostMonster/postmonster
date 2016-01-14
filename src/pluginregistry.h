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
 
#ifndef PLUGINREGISTRY_H
#define PLUGINREGISTRY_H

#include <type_traits>

#include <QHash>
#include <QPluginLoader>

#include "postmonster.h"
#include "toolplugin.h"

class PluginRegistry: public QObject
{
    Q_OBJECT

public:
    struct PluginData
    {
        PostMonster::PluginType type;
        PostMonster::PluginInterface *instance;
        QPluginLoader *loader;
        QJsonObject info;
    };

    static PluginRegistry &instance();
    ~PluginRegistry();

    void loadPlugins(const PostMonster::APIFunctions &api);
    PostMonster::ToolPluginInterface *tool(const QString &name);
    const QJsonObject &info(const PostMonster::PluginInterface *plugin);
    const QList<PluginData *> plugins(PostMonster::PluginType type = PostMonster::All);

signals:
    void toolPluginLoaded(QObject *plugin);

private:
    PluginRegistry() {}
    PluginRegistry(const PluginRegistry &);
    PluginRegistry &operator=(const PluginRegistry &);

    QHash<QString, PluginData *> m_plugins;
    QHash<const PostMonster::PluginInterface *, const QJsonObject *> m_info;
};

#endif // PLUGINREGISTRY_H
