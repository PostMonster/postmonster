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
 
#ifndef HELPERS_H
#define HELPERS_H

#include <QString>
#include <QNetworkAccessManager>

class Helpers
{
public:
    static QString methodToString(QNetworkAccessManager::Operation operation)
    {
        QByteArray method;

        if (operation == QNetworkAccessManager::HeadOperation)
            method = "HEAD";
        else if (operation == QNetworkAccessManager::GetOperation)
            method = "GET";
        else if (operation == QNetworkAccessManager::PutOperation)
            method = "PUT";
        else if (operation == QNetworkAccessManager::PostOperation)
            method = "POST";
        else if (operation == QNetworkAccessManager::DeleteOperation)
            method = "DELETE";
        else if (operation == QNetworkAccessManager::CustomOperation)
            method = "CUSTOM";

        return method;
    }

    static QNetworkAccessManager::Operation stringToMethod(const QString &method)
    {
        QNetworkAccessManager::Operation operation;

        if (method == "HEAD")
            operation = QNetworkAccessManager::HeadOperation;
        else if (method == "GET")
            operation = QNetworkAccessManager::GetOperation;
        else if (method == "PUT")
            operation = QNetworkAccessManager::PutOperation;
        else if (method == "POST")
            operation = QNetworkAccessManager::PostOperation;
        else if (method == "DELETE")
            operation = QNetworkAccessManager::DeleteOperation;
        else if (method == "CUSTOM")
            operation = QNetworkAccessManager::CustomOperation;

        return operation;
    }


};

#endif // HELPERS_H
