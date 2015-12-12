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
 
#include "httptoolplugin.h"

#include <QColor>
#include <QBrush>
#include <QPixmap>
#include <QPainter>

#include <QNetworkCookie>
#include <QBuffer>
#include <QEventLoop>

#include <QJsonObject>
#include <QJsonArray>

#include "requestform.h"
#include "resultform.h"
#include "cookiesform.h"

#define TASKNAME_PREFIX "req_"

HttpToolPlugin::HttpToolPlugin()
{
    Q_INIT_RESOURCE(httptool);

    m_requestForm = new RequestForm(0);
    m_resultForm = new ResultForm(0);

    m_icon = QPixmap(":/icons/httptool");

    m_cookieJar = new PublicCookieJar(this);
    m_cookiesForm = new CookiesForm(m_cookieJar, 0);

    m_nam = new QNetworkAccessManager(this);
    m_nam->setCookieJar(m_cookieJar);
}

void HttpToolPlugin::load(const APIFunctions &api)
{
    m_api = api;
}

const QPixmap &HttpToolPlugin::icon() const
{
    return m_icon;
}

QWidget *HttpToolPlugin::widget(TaskInterface *task)
{
    m_requestForm->updateData(static_cast<HttpTask *>(task));
    return m_requestForm;
}

QWidget *HttpToolPlugin::resultWidget(TaskInterface *task)
{
    HttpTask *httpTask = static_cast<HttpTask *>(task);
    return resultWidget(httpTask->request(), httpTask->response());
}

QWidget *HttpToolPlugin::resultWidget(const HttpRequest *request,
                                      const HttpResponse *response)
{
    m_resultForm->updateData(request, response);
    return m_resultForm;
}

QWidget *HttpToolPlugin::cookiesWidget()
{
    return m_cookiesForm;
}

void HttpToolPlugin::updateMaxnum()
{
    m_maxnum = 0;

    foreach (const QString &name, m_names) {
        if (name.startsWith(TASKNAME_PREFIX)) {
            bool ok = false;
            int num = name.mid(QString(TASKNAME_PREFIX).length()).toInt(&ok);
            if (ok && num > m_maxnum)
                m_maxnum = num;
        }
    }

    m_maxnum++;
}

bool HttpToolPlugin::setTaskName(const TaskInterface *task, const QString &name)
{
    if (!m_names.contains(task) || m_names.values().contains(name))
        return false;

    m_names[task] = name;
    return true;
}

TaskInterface *HttpToolPlugin::createTask()
{
    return createTask(HttpRequest());
}

TaskInterface *HttpToolPlugin::createTask(const HttpRequest &request)
{
    updateMaxnum();

    TaskInterface *task = new HttpTask(this, m_api, request);
    m_names[task] = QString(TASKNAME_PREFIX) + QString::number(m_maxnum);

    QUuid uuid;
    bool unique = true;
    do {
        uuid = QUuid::createUuid();
        foreach (const QUuid &u, m_uuids) {
            if (uuid == u) {
                unique = false;
                break;
            }
        }
    } while (!unique);
    m_uuids[task] = uuid;

    return task;
}

const QUuid &HttpToolPlugin::taskUuid(const TaskInterface *task)
{
    return m_uuids[task];
}

void HttpToolPlugin::destroyTask(TaskInterface *task)
{
    if (!m_names.contains(task))
        return;

    m_names.remove(task);
    delete task;
}

HttpToolPlugin::~HttpToolPlugin()
{
    delete m_requestForm;
    delete m_resultForm;
    delete m_cookiesForm;

    Q_CLEANUP_RESOURCE(httptool);
}

QPixmap HttpTask::itemPixmap() const
{
    QPixmap pixmap(":/icons/httpitem");

    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black));
    painter.setFont(QFont("sans", 10, QFont::Bold));

    QFontMetrics fm(painter.font());
    int textWidth = fm.width(m_request.method);

    painter.drawText(QRectF(30, 5, pixmap.width() - 5, 18),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     m_request.method);

    painter.setFont(QFont("sans", 8));
    painter.drawText(QRectF(30 + textWidth + 5, 5,
                            pixmap.width() - textWidth - 45, 18),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     QUrl(m_request.url).host());

    return pixmap;
}

TaskInterface *HttpTask::clone() const
{
    return new HttpTask(const_cast<HttpTask *>(this)->tool(), m_api, m_request);
}

TaskStatus HttpTask::work(QJsonObject *data)
{
    const APIFunctions &api = m_api;

    m_request.url = api.evalScript(m_request.url.toUtf8());
    m_request.body = api.evalScript(m_request.body);

    for (QList<QNetworkReply::RawHeaderPair>::iterator i = m_request.headers.begin(),
         end = m_request.headers.end(); i != end; ++i)
        (*i).second = api.evalScript((*i).second);

    for (QList<QNetworkCookie>::iterator i = m_request.cookies.begin(),
         end = m_request.cookies.end(); i != end; ++i)
        (*i).setValue(api.evalScript((*i).value()));

    qDebug() << "BODY: " << m_request.body;

    QNetworkRequest request;
    request.setUrl(m_request.url);
    foreach (QNetworkReply::RawHeaderPair header, m_request.headers)
        request.setRawHeader(header.first, header.second);

    QBuffer buffer(&m_request.body);
    buffer.open(QIODevice::ReadOnly);

    QNetworkAccessManager *nam = static_cast<HttpToolPlugin *>(tool())->m_nam;
    nam->cookieJar()->setCookiesFromUrl(m_request.cookies, request.url());
    QNetworkReply *reply = nam->sendCustomRequest(request, m_request.method, &buffer);
    QEventLoop eventLoop;
    QObject::connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();

    m_response.headers = QList<QNetworkReply::RawHeaderPair>();
    foreach (QNetworkReply::RawHeaderPair header, reply->rawHeaderPairs()) {
        if (!QString(header.first).compare("set-cookie", Qt::CaseInsensitive))
            continue;

        m_response.headers << header;
    }

    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    QByteArray body = reply->readAll();

    m_response.status = status;
    m_response.body = body;
    m_response.mimeType = reply->header(QNetworkRequest::ContentTypeHeader).toByteArray();
    m_response.cookies = reply->header(QNetworkRequest::SetCookieHeader).
            value< QList<QNetworkCookie> >();

    updateEnvironment(data);

    if (status == 200)
        return Ok;
    else
        return Fail;
}

void HttpTask::updateEnvironment(QJsonObject *data)
{
    QJsonObject jsonHeaders;
    foreach (QNetworkReply::RawHeaderPair header, m_response.headers)
        jsonHeaders.insert(header.first, QString(header.second));

    if (!jsonHeaders.isEmpty())
        data->insert("headers", jsonHeaders);

    QJsonObject jsonCookies;
    const QNetworkAccessManager *nam = static_cast<HttpToolPlugin *>(tool())->m_nam;
    foreach (QNetworkCookie cookie, nam->cookieJar()->cookiesForUrl(m_request.url)) {
        QJsonObject jsonCookie;

        if (cookie.expirationDate().isValid())
            jsonCookie.insert("expire", cookie.expirationDate().toString(Qt::ISODate));
        if (cookie.isHttpOnly())
            jsonCookie.insert("httpOnly", true);
        if (cookie.isSecure())
            jsonCookie.insert("secure", true);
        if (!cookie.path().isEmpty())
            jsonCookie.insert("path", cookie.path());

        jsonCookie.insert("domain", cookie.domain());
        jsonCookie.insert("value", QString(cookie.value()));

        jsonCookies.insert(cookie.name(), jsonCookie);
    }
    if (!jsonCookies.isEmpty())
        data->insert("cookies", jsonCookies);

    data->insert("status", m_response.status);

    if (!m_response.body.isEmpty())
        data->insert("body", QString("base64:") + m_response.body.toBase64());
}

QJsonObject HttpToolPlugin::serializeTask(const TaskInterface *task)
{
    const HttpTask *httpTask = static_cast<const HttpTask *>(task);
    const HttpRequest *httpRequest = httpTask->constRequest();

    QJsonObject result;
    result.insert("name", task->name());
    result.insert("request", m_api.serializeRequest(httpRequest));

    return result;
}

TaskInterface *HttpToolPlugin::deserializeTask(const QJsonObject &data)
{
    QJsonObject jsonRequest = data.value("request").toObject();
    QString name = data.value("name").toString();

    if (m_names.values().contains(name))
        return 0;

    HttpRequest request = m_api.deserializeRequest(jsonRequest);

    TaskInterface *task = new HttpTask(this, m_api, request);
    m_names[task] = name;

    return task;
}

QList<TaskStatus> HttpToolPlugin::statuses()
{
    QList<TaskStatus> statuses;
    statuses << Ok << Fail;

    return statuses;
}

const QString &HttpToolPlugin::taskName(const TaskInterface *task)
{
    return m_names[task];
}
