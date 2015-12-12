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
 
#ifndef WORKENGINE_H
#define WORKENGINE_H

#include <QObject>
#include <QJsonObject>
#include <QScriptEngine>

#include "diagram/diagramitem.h"

#include "pluginregistry.h"
#include "postmonster.h"
#include "toolplugin.h"

class WorkEngine : public QObject
{
    Q_OBJECT
public:
    explicit WorkEngine(QObject *parent = 0);
    void resetEnvironment();
    const QJsonObject *environment();
    void setActiveItem(DiagramItem *item);
    DiagramItem *activeItem();
    PostMonster::TaskStatus step();
    PostMonster::TaskInterface *task(const QString &toolId, const QString &taskId) const;

    static QByteArray evalScript(const QByteArray &input);

private:
    DiagramItem *m_item;
    PluginRegistry &m_plugins;
    PostMonster::TaskStatus m_status;
    QHash< QPair<QString, QString>, PostMonster::TaskInterface * > m_tasks;

    static QJsonObject m_env;
    static QScriptEngine *m_scriptEngine;
};

#endif // WORKENGINE_H
