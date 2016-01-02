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

    jsonRequest.insert("method", QString(request->method));
    jsonRequest.insert("url", request->url);
    jsonRequest.insert("body", QString(request->body.toBase64()));

    foreach (QNetworkReply::RawHeaderPair header, request->headers) {
        QJsonObject tmp;
        tmp.insert("name", QString(header.first));
        tmp.insert("value", QString(header.second));

        jsonHeaders << tmp;
    }
    jsonRequest.insert("headers", jsonHeaders);

    return jsonRequest;
}

PostMonster::HttpRequest Common::deserializeRequest(const QJsonObject &jsonRequest)
{
    PostMonster::HttpRequest request;

    request.method = jsonRequest.value("method").toString().toLatin1();
    request.url = jsonRequest.value("url").toString();

    QByteArray body;
    body.append(jsonRequest.value("body").toString());
    request.body.append(QByteArray::fromBase64(body));

    foreach (QJsonValue jsonHeader, jsonRequest.value("headers").toArray()) {
        QNetworkReply::RawHeaderPair header;
        header.first.append(jsonHeader.toObject().value("name").toString());
        header.second.append(jsonHeader.toObject().value("value").toString());

        request.headers << header;
    }

    return request;
}

QJsonObject Common::serializeResponse(const PostMonster::HttpResponse *response)
{
    QJsonObject jsonResponse;
    QJsonArray jsonHeaders;

    jsonResponse.insert("mimeType", QLatin1String(response->mimeType));
    jsonResponse.insert("status", response->status);
    jsonResponse.insert("body", QLatin1String(response->body.toBase64()));

    foreach (QNetworkReply::RawHeaderPair header, response->headers) {
        QJsonObject tmp;
        tmp.insert("name", QLatin1String(header.first));
        tmp.insert("value", QLatin1String(header.second));

        jsonHeaders << tmp;
    }
    jsonResponse.insert("headers", jsonHeaders);

    return jsonResponse;
}

PostMonster::HttpResponse Common::deserializeResponse(const QJsonObject &jsonResponse)
{
    PostMonster::HttpResponse response;

    response.mimeType = jsonResponse.value("mimeType").toString().toLatin1();
    response.status = jsonResponse.value("status").toInt();

    QByteArray body;
    body.append(jsonResponse.value("body").toString());
    response.body.append(QByteArray::fromBase64(body));

    foreach (QJsonValue jsonHeader, jsonResponse.value("headers").toArray()) {
        QNetworkReply::RawHeaderPair header;
        header.first.append(jsonHeader.toObject().value("name").toString());
        header.second.append(jsonHeader.toObject().value("value").toString());

        response.headers << header;
    }

    return response;
}
