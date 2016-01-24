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
#include <QtSvg/QSvgRenderer>

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

    m_icon = QIcon(":/icons/httptool");
}

void HttpToolPlugin::load(const APIFunctions &api)
{
    m_api = api;
}

const QIcon &HttpToolPlugin::icon() const
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
            int num = name.mid(QStringLiteral(TASKNAME_PREFIX).length()).toInt(&ok);
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
    m_names[task] = QStringLiteral(TASKNAME_PREFIX) + QString::number(m_maxnum);

    return task;
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
    auto chopToWidth = [](const QString &text, const QFont &font, int width) -> QString
    {
        QFontMetrics fm = QFontMetrics(font);

        QString result = text;
        int textWidth = fm.width(text);

        if (textWidth > width) {
            do {
                result.chop(1);
                textWidth = fm.width(result + "...");
            } while (textWidth > width);
            result += "...";
        }

        return result;
    };

    QSvgRenderer renderer(QStringLiteral(":/icons/httpitem"));
    const qreal s = m_api.dpiScaleFactor();

    QPixmap pixmap(renderer.viewBox().width() * s, renderer.viewBox().height() * s);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);

    // Render SVG on pixmap
    renderer.render(&painter);

    painter.setPen(QPen(Qt::black));
    painter.setFont(QFont("sans", 10, QFont::Bold));

    // Draw method
    QFontMetrics fm(painter.font());
    int methodWidth = fm.width(QLatin1String(m_request.method));
    painter.drawText(25 * s, s + fm.height(), QLatin1String(m_request.method));

    // Draw name
    painter.setFont(QFont("sans", 8));
    fm = QFontMetrics(painter.font());

    int nameMaxWidth = pixmap.width() - 37 * s - methodWidth;
    painter.drawText(30 * s + methodWidth, 6 * s, nameMaxWidth, fm.height(),
                     Qt::AlignRight, chopToWidth(name(), painter.font(), nameMaxWidth));

    // Set label font
    QPen labelPen(QColor("#333333"), 1, Qt::DotLine);
    QPen valuePen(Qt::black);

    QString labelName;
    int labelX, labelY, labelWidth;
    QUrl url(m_request.url);
    QString host = url.host().isEmpty() ? QLatin1Literal("...") : url.host();
    QString path = (url.path().isEmpty() ? "/" : url.path()) +
                   (url.query().isEmpty() ? "" : "?" + url.query());
    QString headerCount = QString::number(m_request.headers.count());
    QString cookieCount = QString::number(m_request.cookies.count());

    // Draw domain label
    labelName = QLatin1Literal("Domain:");
    labelX = 10 * s;
    labelY = 25 * s + (fm.height() + 3 * s);
    labelWidth = fm.width(labelName);
    painter.setPen(labelPen);
    painter.drawLine(labelX, labelY + 2 * s, labelX + labelWidth - 3 * s, labelY + 2 * s);
    painter.drawText(labelX, labelY, labelName);

    // Draw domain value
    painter.setPen(valuePen);
    painter.drawText(labelX + labelWidth + 2 * s, labelY,
                     chopToWidth(host, painter.font(), pixmap.width() - 20 * s - labelWidth));

    // Draw path label
    labelName = QLatin1Literal("Path:");
    labelWidth = fm.width(labelName);
    labelY = 25 * s + (fm.height() + 3 * s) * 2;

    painter.setPen(labelPen);
    painter.drawLine(labelX, labelY + 2 * s, labelX + labelWidth - 3 * s, labelY + 2 * s);
    painter.drawText(labelX, labelY, labelName);

    // Draw path value
    painter.setPen(valuePen);
    painter.drawText(labelX + labelWidth + 2 * s, labelY,
                     chopToWidth(path, painter.font(), pixmap.width() - 20 * s - labelWidth));

    // Draw headers label
    labelName = QLatin1Literal("Headers:");
    labelWidth = fm.width(labelName);
    labelY = 25 * s + (fm.height() + 3 * s) * 3;

    painter.setPen(labelPen);
    painter.drawLine(labelX, labelY + 2 * s, labelX + labelWidth - 3 * s, labelY + 2 * s);
    painter.drawText(labelX, labelY, labelName);

    // Draw headers value
    painter.setPen(Qt::black);
    painter.drawText(labelX + labelWidth + 2 * s, labelY, headerCount + ";");

    // Draw cookies label
    labelName = QLatin1Literal("Cookies:");
    labelX += labelWidth + fm.width(headerCount) + 10 * s;
    labelWidth = fm.width(labelName);
    labelY = 25 * s + (fm.height() + 3 * s) * 3;

    painter.setPen(labelPen);
    painter.drawLine(labelX, labelY + 2 * s, labelX + labelWidth - 3 * s, labelY + 2 * s);
    painter.drawText(labelX, labelY, labelName);

    // Draw cookies value
    painter.setPen(Qt::black);
    painter.drawText(labelX + labelWidth + 2 * s, labelY, cookieCount);

    return pixmap;
}

void HttpTask::stop()
{
    emit stopRequested();
}

TaskStatus HttpTask::work(const QJsonObject &environment, QJsonObject &toolSection,
                          QScriptEngine &scriptEngine, TaskInterface **processed)
{
    const APIFunctions &api = m_api;

    QNetworkRequest networkRequest;
    networkRequest.setUrl(api.evalScript(m_request.url, environment, scriptEngine));

    HttpRequest processedRequest;
    processedRequest.method = m_request.method;
    processedRequest.body = m_request.body;
    processedRequest.encoding = m_request.encoding;
    processedRequest.url = networkRequest.url().toString();

    for (QList<QNetworkReply::RawHeaderPair>::iterator i = m_request.headers.begin(),
         end = m_request.headers.end(); i != end; ++i) {
        QNetworkReply::RawHeaderPair header = *i;
        header.second = api.evalScript(header.second, environment, scriptEngine).toLatin1();

        networkRequest.setRawHeader(header.first, header.second);
        processedRequest.headers << header;
    }

    foreach (QNetworkCookie cookie, m_request.cookies) {
        QString value = api.evalScript(QUrl::fromPercentEncoding(cookie.value()),
                                       environment, scriptEngine);

        cookie.setValue(QUrl::toPercentEncoding(value));
        processedRequest.cookies << cookie;
    }

    QJsonObject jsonCookieJar;
    if (toolSection.contains("_cookies")) {
        jsonCookieJar = toolSection["_cookies"].toObject();
    }

    QList<QNetworkCookie> cookies;
    foreach (const QString &domain, jsonCookieJar.keys()) {
        QNetworkCookie cookie;
        cookie.setDomain(domain);

        QJsonObject jsonDomain = jsonCookieJar.value(domain).toObject();
        foreach (const QString &name, jsonDomain.keys()) {
            QJsonObject jsonCookie = jsonDomain.value(name).toObject();

            if (jsonCookie.contains("expire"))
                cookie.setExpirationDate(QDateTime::fromString(
                                             jsonCookie["expire"].toString(), Qt::ISODate));
            if (jsonCookie["httpOnly"].toBool())
                cookie.setHttpOnly(true);
            if (jsonCookie["secure"].toBool())
                cookie.setSecure(true);
            if (jsonCookie.contains("path"))
                cookie.setPath(jsonCookie["path"].toString());

            cookie.setName(name.toLatin1());
            cookie.setValue(jsonCookie["value"].toString().toLatin1());

            cookies << cookie;
        }
    }

    QNetworkCookieJar cookieJar;
    cookieJar.setCookiesFromUrl(cookies, networkRequest.url());
    cookieJar.setCookiesFromUrl(processedRequest.cookies, networkRequest.url());

    processedRequest.cookies = cookieJar.cookiesForUrl(networkRequest.url());

    QNetworkAccessManager nam;
    nam.setCookieJar(&cookieJar);
    cookieJar.setParent(nullptr);

    QNetworkReply *reply = nam.sendCustomRequest(networkRequest, m_request.method);
    QEventLoop eventLoop;
    connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    connect(reply, &QNetworkReply::downloadProgress, [this](qint64 got, qint64 total) {
        emit progress(100.0 * got / (total < 0 ? got * 2 : total));
    });

    bool taskStopped = false;
    connect(this, &HttpTask::stopRequested, [&eventLoop, &taskStopped] {
        eventLoop.exit();
        taskStopped = true;
    });
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    if (taskStopped) {
        return Fail;
    }

    HttpResponse response;
    QJsonObject jsonHeaders, jsonCookies;
    foreach (const QNetworkReply::RawHeaderPair &header, reply->rawHeaderPairs()) {
        if (!QString::fromLatin1(header.first).compare("set-cookie", Qt::CaseInsensitive)) {
            QList<QNetworkCookie> cookies = QNetworkCookie::parseCookies(header.second);

            foreach (const QNetworkCookie &cookie, cookies) {
                if (cookie.domain().isEmpty()) continue;

                QJsonObject jsonCookie;
                if (cookie.expirationDate().isValid())
                    jsonCookie.insert("expire", cookie.expirationDate().toString(Qt::ISODate));
                if (cookie.isHttpOnly())
                    jsonCookie.insert("httpOnly", true);
                if (cookie.isSecure())
                    jsonCookie.insert("secure", true);
                if (!cookie.path().isEmpty())
                    jsonCookie.insert("path", cookie.path());

                QJsonObject jsonCookieDomain;
                if (jsonCookieJar.contains(cookie.domain()))
                    jsonCookieDomain = jsonCookieJar[cookie.domain()].toObject();
                jsonCookieDomain.insert(cookie.name(), jsonCookie);
                jsonCookieJar.insert(cookie.domain(), jsonCookieDomain);

                jsonCookie.insert("domain", cookie.domain());
                jsonCookie.insert("value", QLatin1String(cookie.value()));
                jsonCookies.insert(cookie.name(), jsonCookie);
            }

            continue;
        }

        jsonHeaders.insert(QLatin1String(header.first), QLatin1String(header.second));
        response.headers << header;
    }

    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray body = reply->readAll();

    response.status = status;
    response.body = body;
    response.mimeType = reply->header(QNetworkRequest::ContentTypeHeader).toByteArray();
    response.cookies = reply->header(QNetworkRequest::SetCookieHeader).
            value< QList<QNetworkCookie> >();

    delete reply;

    QJsonObject jsonResult;
    jsonResult.insert("status", status);
    if (!body.isEmpty())
        jsonResult.insert("body", QLatin1String("base64:") + body.toBase64());
    if (!jsonCookies.isEmpty())
        jsonResult.insert("cookies", jsonCookies);
    if (!jsonHeaders.isEmpty())
        jsonResult.insert("headers", jsonHeaders);

    toolSection.insert(name(), jsonResult);
    if (!jsonCookieJar.isEmpty())
        toolSection.insert("_cookies", jsonCookieJar);

    if (processed) {
        *processed = new HttpTask(tool(), m_api, processedRequest, response);
    }

    return (status == 200) ? Ok : Fail;
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
    QJsonObject jsonRequest = data["request"].toObject();
    QString name = data["name"].toString();

    if (m_names.values().contains(name))
        return 0;

    HttpRequest request = m_api.deserializeRequest(jsonRequest);

    TaskInterface *task = new HttpTask(this, m_api, request);
    m_names[task] = name;

    return task;
}

QList<TaskStatus> HttpToolPlugin::statuses()
{
    return { Ok, Fail };
}

const QString &HttpToolPlugin::taskName(const TaskInterface *task)
{
    return m_names[task];
}
