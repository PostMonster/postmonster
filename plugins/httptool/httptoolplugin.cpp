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

#include <QTextCodec>

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
    m_requestForm->updateData(static_cast<HttpTask *>(task), m_api.defaultEncodings());
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
    m_resultForm->updateData(request, response, m_api.defaultEncodings());
    return m_resultForm;
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
    if (name.startsWith("_") || !m_names.contains(task) || m_names.values().contains(name))
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

TaskStatus HttpTask::work(const QJsonObject &environment, QJsonObject &toolSection,
                          QScriptEngine &scriptEngine, TaskInterface **processed)
{
    const APIFunctions &api = m_api;

    HttpRequest request;
    request.method = m_request.method;
    request.url = api.evalScript(m_request.url, environment, scriptEngine);

    QTextCodec *codec = QTextCodec::codecForName(m_request.encoding);
    if (!codec) {
        codec = QTextCodec::codecForName("ISO-8859-1");
    }

    request.body = codec->fromUnicode(api.evalScript(codec->toUnicode(m_request.body),
                                                     environment, scriptEngine));

    for (QList<QNetworkReply::RawHeaderPair>::iterator i = m_request.headers.begin(),
         end = m_request.headers.end(); i != end; ++i) {
        QNetworkReply::RawHeaderPair header = *i;
        header.second = api.evalScript(QLatin1String(header.second), environment,
                                       scriptEngine).toLatin1();

        request.headers << header;
    }

    for (QList<QNetworkCookie>::iterator i = m_request.cookies.begin(),
         end = m_request.cookies.end(); i != end; ++i) {
        QNetworkCookie cookie = *i;
        cookie.setValue(api.evalScript(QLatin1String(cookie.value()), environment,
                                       scriptEngine).toLatin1());

        request.cookies << cookie;
    }

    QNetworkRequest networkRequest;
    networkRequest.setUrl(m_request.url);
    foreach (QNetworkReply::RawHeaderPair header, m_request.headers)
        networkRequest.setRawHeader(header.first, header.second);

    QBuffer buffer(&request.body);
    buffer.open(QIODevice::ReadOnly);

    QJsonObject jsonCookieJar;
    if (toolSection.contains("_cookies"))
        jsonCookieJar = toolSection.value("_cookies").toObject();

    QList<QNetworkCookie> cookies;
    foreach (const QString &domain, jsonCookieJar.keys()) {
        QNetworkCookie cookie;
        cookie.setDomain(domain);

        QJsonObject jsonDomain = jsonCookieJar.value(domain).toObject();
        foreach (const QString &name, jsonDomain.keys()) {
            QJsonObject jsonCookie = jsonDomain.value(name).toObject();

            if (jsonCookie.contains("expire"))
                cookie.setExpirationDate(QDateTime::fromString(jsonCookie.value("expire").toString(), Qt::ISODate));
            if (jsonCookie.value("httpOnly").toBool())
                cookie.setHttpOnly(true);
            if (jsonCookie.value("secure").toBool())
                cookie.setSecure(true);
            if (jsonCookie.contains("path"))
                cookie.setPath(jsonCookie.value("path").toString());

            cookie.setName(name.toLatin1());
            cookie.setValue(jsonCookie.value("value").toString().toLatin1());

            cookies << cookie;
        }
    }

    QNetworkCookieJar cookieJar;
    cookieJar.setCookiesFromUrl(cookies, networkRequest.url());
    cookieJar.setCookiesFromUrl(request.cookies, networkRequest.url());

    request.cookies = cookieJar.cookiesForUrl(networkRequest.url());

    QNetworkAccessManager nam;
    nam.setCookieJar(&cookieJar);
    cookieJar.setParent(nullptr);

    QNetworkReply *reply = nam.sendCustomRequest(networkRequest, request.method, &buffer);
    QEventLoop eventLoop;
    QObject::connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();

    HttpResponse response;
    QJsonObject jsonHeaders, jsonCookies;
    foreach (QNetworkReply::RawHeaderPair header, reply->rawHeaderPairs()) {
        if (!QString(header.first).compare("set-cookie", Qt::CaseInsensitive)) {
            QList<QNetworkCookie> cookies = QNetworkCookie::parseCookies(header.second);

            foreach (QNetworkCookie cookie, cookies) {
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

            continue;
        }

        jsonHeaders.insert(QString(header.first), QString(header.second));
        response.headers << header;
    }

    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray body = reply->readAll();

    response.status = status;
    response.body = body;
    response.mimeType = reply->header(QNetworkRequest::ContentTypeHeader).toByteArray();
    response.cookies = reply->header(QNetworkRequest::SetCookieHeader).
            value< QList<QNetworkCookie> >();

    QJsonObject jsonResult;
    jsonResult.insert("status", status);
    if (!body.isEmpty())
        jsonResult.insert("body", QString("base64:") + body.toBase64());
    if (!jsonCookies.isEmpty())
        jsonResult.insert("cookies", jsonCookies);
    if (!jsonHeaders.isEmpty())
        jsonResult.insert("headers", jsonHeaders);
    toolSection.insert(name(), jsonResult);

    foreach (QNetworkCookie cookie, cookieJar.cookiesForUrl(networkRequest.url())) {
        QJsonObject jsonCookie;

        if (cookie.expirationDate().isValid())
            jsonCookie.insert("expire", cookie.expirationDate().toString(Qt::ISODate));
        if (cookie.isHttpOnly())
            jsonCookie.insert("httpOnly", true);
        if (cookie.isSecure())
            jsonCookie.insert("secure", true);
        if (!cookie.path().isEmpty())
            jsonCookie.insert("path", cookie.path());

        jsonCookie.insert("value", QString(cookie.value()));

        QJsonObject jsonCookieDomain;
        if (jsonCookieJar.contains(cookie.domain()))
            jsonCookieDomain = jsonCookieJar.value(cookie.domain()).toObject();

        jsonCookieDomain.insert(cookie.name(), jsonCookie);
        jsonCookieJar.insert(cookie.domain(), jsonCookieDomain);
    }
    toolSection.insert("_cookies", jsonCookieJar);

    if (processed) {
        *processed = new HttpTask(tool(), m_api, request, response);
    }

    if (status == 200)
        return Ok;
    else
        return Fail;
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
