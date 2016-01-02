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
 
#include "workengine.h"

#include <QJsonDocument>
#include <QStack>

#include "diagram/taskitem.h"
#include "diagram/arrow.h"

WorkEngine::WorkEngine(QObject *parent)
    : QObject(parent), m_item(0),
      m_plugins(PluginRegistry::instance()), m_status(PostMonster::Default)
{

}

WorkEngine::~WorkEngine()
{
    qDeleteAll(m_tasks.values());
}

QString WorkEngine::evalScript(const QString &input, const QJsonObject &environment,
                               QScriptEngine &engine) {
    QString result = input;
    int pos = 0, start = 0, delta = 0;

    while ((pos = input.indexOf("${", pos)) != -1) {
        QStack<char> brackets;
        brackets.push(1);
        bool inDoubleQuotes = false, inSingleQuotes = false;

        pos += 2;
        start = pos;
        while (pos < input.length() && !brackets.isEmpty()) {
            if (input[pos] == '{' && !inDoubleQuotes && !inSingleQuotes)
                brackets.push(1);
            else if (input[pos] == '}' && !inDoubleQuotes && !inSingleQuotes)
                brackets.pop();
            else if (input[pos] == '"' && input[pos - 1] != '\\' && !inSingleQuotes)
                inDoubleQuotes = !inDoubleQuotes;
            else if (input[pos] == '\'' && input[pos - 1] != '\\' && !inDoubleQuotes)
                inSingleQuotes = !inSingleQuotes;

            pos++;
        }

        if (brackets.isEmpty()) {
            qDebug() << "VAR=" << input.mid(start, pos - start - 1);
            QJsonDocument doc(environment);
            QScriptValue env = engine.evaluate("[" + doc.toJson() + "]");

            engine.globalObject().setProperty("Env", env.property(0));
            QScriptValue value = engine.evaluate(input.mid(start, pos - start - 1));

            if (value.isString() || value.isNumber() || value.isDate() || value.isBool()) {
                QByteArray str = value.toString().toUtf8(); //TODO utf8???

                result = result.replace(start - delta - 2, pos - start + 2, str);
                delta += pos - start - str.length() + 2;
            }
        }
    }

    qDebug() << result;
    return result;
}

PostMonster::TaskStatus WorkEngine::step() {
    if (!m_item)
        return PostMonster::Default;

    PostMonster::TaskStatus result = PostMonster::Default;
    if (m_item->diagramType() == DiagramItem::TypeStart) {
        m_item = m_item->arrows()->first()->endItem();
    } else if (m_item->diagramType() == DiagramItem::TypeTask) {
        TaskItem *taskItem = static_cast<TaskItem *>(m_item);

        QString toolName = m_plugins.info(taskItem->task()->tool()).value("id").toString();
        QString taskName = taskItem->task()->name();
        QJsonObject jsonTool = m_env.contains(toolName) ? m_env.value(toolName).toObject() : QJsonObject();

        PostMonster::TaskInterface *processedTask;
        result = taskItem->task()->work(m_env, jsonTool, m_scriptEngine, &processedTask);

        //jsonTool.insert(taskName, jsonTask);
        qDebug() << "WORKER TASKNAME=" << taskName;
        m_env.insert(toolName, jsonTool);

        QPair<QString, QString> taskKey(toolName, taskName);
        if (m_tasks.contains(taskKey)) {
            delete m_tasks[taskKey];
            m_tasks.remove(taskKey);
        }
        m_tasks.insert(taskKey, processedTask);

        const Arrow *arrow = taskItem->arrow(result);
        if (arrow)
            m_item = arrow->endItem();
        else
            m_item = 0;
    }

    return result;
}

const QJsonObject *WorkEngine::environment()
{
    return &m_env;
}

void WorkEngine::reset()
{
    m_env = QJsonObject();

    qDeleteAll(m_tasks.values());
    m_tasks.clear();
}

void WorkEngine::setActiveItem(DiagramItem *item)
{
    m_item = item;
}

DiagramItem *WorkEngine::activeItem()
{
    return m_item;
}

PostMonster::TaskInterface *WorkEngine::task(const QString &toolId, const QString &taskId) const
{
    QPair<QString, QString> pair(toolId, taskId);
    if (!m_tasks.contains(pair))
        return 0;

    return m_tasks[pair];
}
