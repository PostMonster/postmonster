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
 
#ifndef COMMON_H
#define COMMON_H

#include <QJsonObject>

#include "postmonster.h"

class Common {
public:
    static QJsonObject serializeRequest(const PostMonster::HttpRequest *request);
    static PostMonster::HttpRequest deserializeRequest(const QJsonObject &jsonRequest);

    static QJsonObject serializeResponse(const PostMonster::HttpResponse *response);
    static PostMonster::HttpResponse deserializeResponse(const QJsonObject &jsonResponse);
};

#endif // COMMON_H

