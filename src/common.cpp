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
 
#include "common.h"

#include <QJsonArray>
#include <QGuiApplication>
#include <QScreen>

const QStringList Common::m_encodings
{
    "IBM866",
    "ISO-8859-1",
    "ISO-8859-2",
    "ISO-8859-3",
    "ISO-8859-4",
    "ISO-8859-5",
    "ISO-8859-6",
    "ISO-8859-7",
    "ISO-8859-8",
    "ISO-8859-9",
    "ISO-8859-10",
    "ISO-8859-11",
    "ISO-8859-13",
    "ISO-8859-14",
    "ISO-8859-15",
    "ISO-8859-16",
    "Windows-1250",
    "Windows-1251",
    "Windows-1252",
    "Windows-1253",
    "Windows-1254",
    "Windows-1255",
    "Windows-1256",
    "Windows-1257",
    "Windows-1258",
    "KOI8-R",
    "KOI8-U",
    "UTF-8",
    "UTF-16"
};

const QStringList &Common::encodings()
{
    return m_encodings;
}

QJsonObject Common::serializeRequest(const PostMonster::HttpRequest *request)
{
    QJsonObject jsonRequest;
    QJsonArray jsonHeaders;

    jsonRequest.insert("url", request->url);
    jsonRequest.insert("method", QLatin1String(request->method));
    jsonRequest.insert("encoding", QLatin1String(request->encoding));
    jsonRequest.insert("body", QLatin1String(request->body.toBase64()));

    foreach (const QNetworkReply::RawHeaderPair &header, request->headers) {
        QJsonObject tmp;
        tmp.insert("name", QLatin1String(header.first));
        tmp.insert("value", QLatin1String(header.second));

        jsonHeaders << tmp;
    }
    jsonRequest.insert("headers", jsonHeaders);

    QJsonArray jsonCookies;
    foreach (const QNetworkCookie &cookie, request->cookies) {
        QJsonObject tmp;

        if (cookie.expirationDate().isValid())
            tmp.insert("expire", cookie.expirationDate().toString(Qt::ISODate));
        if (cookie.isHttpOnly())
            tmp.insert("httpOnly", true);
        if (cookie.isSecure())
            tmp.insert("secure", true);
        if (!cookie.path().isEmpty())
            tmp.insert("path", cookie.path());

        tmp.insert("domain", cookie.domain());
        tmp.insert("value", QLatin1String(cookie.value()));
        tmp.insert("name", QLatin1String(cookie.name()));

        jsonCookies << tmp;
    }
    jsonRequest.insert("cookies", jsonCookies);

    return jsonRequest;
}

PostMonster::HttpRequest Common::deserializeRequest(const QJsonObject &jsonRequest)
{
    PostMonster::HttpRequest request;

    request.url = jsonRequest["url"].toString();
    request.method = jsonRequest["method"].toString().toLatin1();
    request.encoding = jsonRequest["encoding"].toString().toLatin1();
    request.body = QByteArray::fromBase64(jsonRequest["body"].toString().toLatin1());

    foreach (const QJsonValue &value, jsonRequest["headers"].toArray()) {
        const QJsonObject &jsonHeader = value.toObject();
        QNetworkReply::RawHeaderPair header;

        header.first = jsonHeader["name"].toString().toLatin1();
        header.second = jsonHeader["value"].toString().toLatin1();

        request.headers << header;
    }

    foreach (const QJsonValue &value, jsonRequest["cookies"].toArray()) {
        const QJsonObject &jsonCookie = value.toObject();
        QNetworkCookie cookie;

        if (jsonCookie.contains("expire"))
            cookie.setExpirationDate(QDateTime::fromString(jsonCookie["expire"].toString(),
                                                           Qt::ISODate));
        if (jsonCookie["httpOnly"].toBool())
            cookie.setHttpOnly(true);
        if (jsonCookie["secure"].toBool())
            cookie.setSecure(true);
        if (jsonCookie.contains("path"))
            cookie.setPath(jsonCookie["path"].toString());

        cookie.setName(jsonCookie["name"].toString().toLatin1());
        cookie.setValue(jsonCookie["value"].toString().toLatin1());

        request.cookies << cookie;
    }

    return request;
}

QJsonObject Common::serializeResponse(const PostMonster::HttpResponse *response)
{
    QJsonObject jsonResponse;
    jsonResponse.insert("mimeType", QLatin1String(response->mimeType));
    jsonResponse.insert("status", response->status);
    jsonResponse.insert("body", QLatin1String(response->body.toBase64()));

    QJsonArray jsonHeaders;
    foreach (const QNetworkReply::RawHeaderPair &header, response->headers) {
        QJsonObject tmp;
        tmp.insert("name", QLatin1String(header.first));
        tmp.insert("value", QLatin1String(header.second));

        jsonHeaders << tmp;
    }
    jsonResponse.insert("headers", jsonHeaders);

    QJsonArray jsonCookies;
    foreach (const QNetworkCookie &cookie, response->cookies) {
        QJsonObject tmp;

        if (cookie.expirationDate().isValid())
            tmp.insert("expire", cookie.expirationDate().toString(Qt::ISODate));
        if (cookie.isHttpOnly())
            tmp.insert("httpOnly", true);
        if (cookie.isSecure())
            tmp.insert("secure", true);
        if (!cookie.path().isEmpty())
            tmp.insert("path", cookie.path());

        tmp.insert("domain", cookie.domain());
        tmp.insert("value", QLatin1String(cookie.value()));
        tmp.insert("name", QLatin1String(cookie.name()));

        jsonCookies << tmp;
    }
    jsonResponse.insert("cookies", jsonCookies);

    return jsonResponse;
}

PostMonster::HttpResponse Common::deserializeResponse(const QJsonObject &jsonResponse)
{
    PostMonster::HttpResponse response;

    response.mimeType = jsonResponse["mimeType"].toString().toLatin1();
    response.status = jsonResponse["status"].toInt();

    QByteArray body;
    body.append(jsonResponse["body"].toString());
    response.body.append(QByteArray::fromBase64(body));

    foreach (const QJsonValue &jsonHeader, jsonResponse["headers"].toArray()) {
        QNetworkReply::RawHeaderPair header;
        header.first.append(jsonHeader.toObject()["name"].toString());
        header.second.append(jsonHeader.toObject()["value"].toString());

        response.headers << header;
    }

    foreach (const QJsonValue &value, jsonResponse["cookies"].toArray()) {
        const QJsonObject &jsonCookie = value.toObject();
        QNetworkCookie cookie;

        if (jsonCookie.contains("expire"))
            cookie.setExpirationDate(QDateTime::fromString(jsonCookie["expire"].toString(),
                                                           Qt::ISODate));
        if (jsonCookie["httpOnly"].toBool())
            cookie.setHttpOnly(true);
        if (jsonCookie["secure"].toBool())
            cookie.setSecure(true);
        if (jsonCookie.contains("path"))
            cookie.setPath(jsonCookie["path"].toString());

        cookie.setName(jsonCookie["name"].toString().toLatin1());
        cookie.setValue(jsonCookie["value"].toString().toLatin1());

        response.cookies << cookie;
    }

    return response;
}

const qreal Common::dpiScaleFactor()
{
    static qreal factor = qApp->primaryScreen()->logicalDotsPerInch() / 96;
    return factor;
}
