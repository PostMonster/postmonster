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

#include <QMap>

#include "postmonster.h"
#include "toolplugin.h"

class PluginRegistry: public QObject
{
    Q_OBJECT

public:
    static PluginRegistry &instance();

    void loadPlugins(const PostMonster::APIFunctions &api);
    PostMonster::ToolPluginInterface *tool(const QString &name);
    const QJsonObject &info(const PostMonster::PluginInterface *plugin);

signals:
    void toolPluginLoaded(QObject *plugin);

protected:
    QHash<QString, PostMonster::ToolPluginInterface *> m_tools;
    QMap<PostMonster::PluginInterface *, QJsonObject> m_metaData;

private:
    PluginRegistry() {}
    ~PluginRegistry() {}
    PluginRegistry(const PluginRegistry &);

    PluginRegistry &operator=(const PluginRegistry &);
};

#endif // PLUGINREGISTRY_H
