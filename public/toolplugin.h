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
 
#ifndef TOOLPLUGIN_H
#define TOOLPLUGIN_H

#include "postmonster.h"

#include <QUuid>
#include <QPolygon>
#include <QWidget>
#include <QPixmap>
#include <QJsonObject>

namespace PostMonster
{

class TaskInterface;
class ToolPluginInterface: public PluginInterface
{
public:
    ToolPluginInterface(): PluginInterface() {}
    virtual ~ToolPluginInterface() {}

    virtual const QPixmap &icon() const = 0;

    virtual QWidget *widget(TaskInterface *task) = 0;
    virtual QWidget *resultWidget(TaskInterface *task) = 0;

    virtual TaskInterface *createTask() = 0;
    virtual void destroyTask(TaskInterface *task) = 0;
    virtual QJsonObject serializeTask(const TaskInterface *task) = 0;
    virtual TaskInterface *deserializeTask(const QJsonObject &data) = 0;

    virtual QList<TaskStatus> statuses() = 0;

    virtual const QUuid &taskUuid(const TaskInterface *task) = 0;
    virtual const QString &taskName(const TaskInterface *task) = 0;
    virtual bool setTaskName(const TaskInterface *task, const QString &name) = 0;
};

class HttpToolPluginInterface: public ToolPluginInterface
{
public:
    virtual TaskInterface *createTask(const HttpRequest &request) = 0;
    virtual QWidget *resultWidget(const HttpRequest *request,
                                  const HttpResponse *response) = 0;
    virtual QWidget *cookiesWidget() = 0;
};

class TaskInterface: public QObject
{
Q_OBJECT

public:
    TaskInterface(ToolPluginInterface *tool, const APIFunctions &api)
        : m_tool(tool), m_api(api)
    {
    }
    virtual ~TaskInterface() {}

    virtual TaskStatus work(QJsonObject *data) = 0;
    virtual TaskInterface *clone() const = 0;

    virtual QPixmap itemPixmap() const = 0;
    virtual QPolygon itemShape() const = 0;

    const QUuid &uuid() const
    {
        return m_tool->taskUuid(this);
    }

    QWidget *widget()
    {
        return m_tool->widget(this);
    }

    QWidget *resultWidget()
    {
        return m_tool->resultWidget(this);
    }

    QJsonObject serialize() const
    {
        return m_tool->serializeTask(this);
    }

    ToolPluginInterface *tool()
    {
        return m_tool;
    }

    const QString &name() const
    {
        return m_tool->taskName(this);
    }

    bool setName(const QString &name) const
    {
        return m_tool->setTaskName(this, name);
    }

signals:
    void dataChanged();

private:
    ToolPluginInterface *m_tool;

protected:
    const APIFunctions m_api;
};

}

#define ToolPluginInterface_iid "org.postmonster.ToolPluginInterface"
Q_DECLARE_INTERFACE(PostMonster::ToolPluginInterface, ToolPluginInterface_iid)


#endif // TOOLPLUGIN_H
