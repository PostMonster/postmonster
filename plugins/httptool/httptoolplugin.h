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
 
#ifndef HTTPTOOLPLUGIN_H
#define HTTPTOOLPLUGIN_H

#include <QtPlugin>
#include <QtScript/QScriptEngine>
#include <QNetworkCookieJar>

#include <postmonster.h>
#include <toolplugin.h>

#include "publiccookiejar.h"

using namespace PostMonster;

class RequestForm;
class ResultForm;

class HttpTask: public TaskInterface
{
    Q_OBJECT

public:
    HttpTask(ToolPluginInterface *tool, const APIFunctions &api, const HttpRequest &request)
        : TaskInterface(tool, api), m_request(request), m_pixmapScale(1)
    {
    }

    HttpTask(ToolPluginInterface *tool, const APIFunctions &api,
             const HttpRequest &request, const HttpResponse &response)
        : TaskInterface(tool, api), m_request(request), m_response(response), m_pixmapScale(1)
    {
    }

    TaskStatus work(const QJsonObject &environment, QJsonObject &toolSection,
                    QScriptEngine &scriptEngine, TaskInterface **processed = 0);

    QPixmap itemPixmap() const;
    QPainterPath itemShape() const {
        QPainterPath path;
        path.addRoundRect(itemPixmap().rect(), 6, 12);
        return path;
    }
    HttpRequest *request() { return &m_request; }
    HttpResponse *response() { return &m_response; }
    const HttpRequest *processedRequest() { return &m_processedRequest; }
    const HttpRequest *constRequest() const { return &m_request; }

public slots:
    void stop();

signals:
    void stopRequested();

private:
    HttpRequest m_request;
    HttpRequest m_processedRequest;
    HttpResponse m_response;
    qreal m_pixmapScale;
};

class HttpToolPlugin: public QObject, public HttpToolPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ToolPluginInterface_iid FILE "httptool.json")
    Q_INTERFACES(PostMonster::ToolPluginInterface)

public:
    HttpToolPlugin();

    void load(const APIFunctions &api);

    const QPixmap &icon() const;
    QWidget *widget(TaskInterface *task);
    QWidget *resultWidget(TaskInterface *task);
    TaskInterface *createTask();
    void destroyTask(TaskInterface *task);
    QJsonObject serializeTask(const TaskInterface *task);
    TaskInterface *deserializeTask(const QJsonObject &data);
    QList<TaskStatus> statuses();

    const QString &taskName(const TaskInterface *task);
    bool setTaskName(const TaskInterface *task, const QString &name);

    TaskInterface *createTask(const HttpRequest &request);
    QWidget *resultWidget(const HttpRequest *request,
                          const HttpResponse *response);

    ~HttpToolPlugin();

protected:
    void updateMaxnum();

private:    
    APIFunctions m_api;

    QPixmap m_icon;
    RequestForm *m_requestForm;
    ResultForm *m_resultForm;
    QHash<const TaskInterface *, QString> m_names;
    int m_maxnum;

    friend class HttpTask;
};

#endif // HTTPTOOLPLUGIN_H
