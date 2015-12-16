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
 
#ifndef POSTMONSTER_H
#define POSTMONSTER_H

#include <QString>
#include <QByteArray>
#include <QList>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkCookie>

#include <QtScript/QScriptEngine>

namespace PostMonster
{

enum TaskStatus {
    Default,
    Ok,
    Fail,
    True,
    False
};

struct HttpRequest
{
    HttpRequest(): method("GET") {}

    QString url;
    QByteArray body;
    QByteArray method;
    QList<QNetworkReply::RawHeaderPair> headers;
    QList<QNetworkCookie> cookies;
};

struct HttpResponse
{
    HttpResponse(): status(0) {}

    QString mimeType;
    int status;
    QByteArray body;
    QList<QNetworkReply::RawHeaderPair> headers;
    QList<QNetworkCookie> cookies;
};

struct APIFunctions
{
    QJsonObject (*serializeRequest)(const PostMonster::HttpRequest *request);
    PostMonster::HttpRequest (*deserializeRequest)(const QJsonObject &jsonRequest);

    QJsonObject (*serializeResponse)(const PostMonster::HttpResponse *response);
    PostMonster::HttpResponse (*deserializeResponse)(const QJsonObject &jsonResponse);

    QByteArray (*evalScript)(const QByteArray &input,
                             const QJsonObject &environment,
                             QScriptEngine &scriptEngine);
};

class PluginInterface
{
public:
    PluginInterface() {}
    virtual ~PluginInterface() {}

    virtual void load(const APIFunctions &api) = 0;
};

}

#endif // POSTMONSTER_H
