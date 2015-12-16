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
class CookiesForm;

class HttpTask: public TaskInterface
{
public:
    HttpTask(ToolPluginInterface *tool, const APIFunctions &api, const HttpRequest &request)
        : TaskInterface(tool, api), m_request(request) {
        m_request.cookies.clear();
    }

    TaskStatus work(QJsonObject &result, const QJsonObject &environment,
                    QScriptEngine &scriptEngine);
    TaskInterface *clone() const;

    QPixmap itemPixmap() const;
    QPolygon itemShape() const { return itemPixmap().rect(); }
    HttpRequest *request() { return &m_request; }
    HttpResponse *response() { return &m_response; }
    const HttpRequest *constRequest() const { return &m_request; }

private:
    void updateResult(QJsonObject &data);

    HttpRequest m_request;
    HttpResponse m_response;
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
    QWidget *cookiesWidget();
    TaskInterface *createTask();
    void destroyTask(TaskInterface *task);
    QJsonObject serializeTask(const TaskInterface *task);
    TaskInterface *deserializeTask(const QJsonObject &data);
    QList<TaskStatus> statuses();

    const QString &taskName(const TaskInterface *task);
    bool setTaskName(const TaskInterface *task, const QString &name);
    const QUuid &taskUuid(const TaskInterface *task);

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
    CookiesForm *m_cookiesForm;
    QHash<const TaskInterface *, QString> m_names;
    QHash<const TaskInterface *, QUuid> m_uuids;
    PublicCookieJar *m_cookieJar;
    QNetworkAccessManager *m_nam;
    int m_maxnum;

    friend class HttpTask;
};

#endif // HTTPTOOLPLUGIN_H
