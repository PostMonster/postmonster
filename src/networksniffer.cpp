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
 
#include "networksniffer.h"

NetworkSniffer::NetworkSniffer(QObject *parent)
    : QNetworkAccessManager(parent)
{

}

void NetworkSniffer::dataAvailable()
{
    QNetworkReply *reply = static_cast<QNetworkReply *> (sender());

    QByteArray responseData;
    if (reply->bytesAvailable() > 0)
        responseData = reply->peek(reply->bytesAvailable());

    if (m_data.contains(reply))
        m_data[reply].second = responseData;
}

void NetworkSniffer::requestFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *> (sender());

    QByteArray requestData, responseData;
    if (m_data.contains(reply)) {
        requestData = m_data[reply].first;
        responseData = m_data[reply].second;
    }

    emit replyReceived(reply, requestData, responseData);
}

QNetworkReply *NetworkSniffer::createRequest(
        Operation op, const QNetworkRequest& request, QIODevice *outgoingData)
{
    QByteArray requestData;
    if (outgoingData)
        requestData = outgoingData->peek(request.header(QNetworkRequest::ContentLengthHeader).toLongLong());

    QNetworkReply *reply = QNetworkAccessManager::createRequest(op, request, outgoingData);
    connect(reply, SIGNAL(finished()), this, SLOT(requestFinished()));
    connect(reply, SIGNAL(readyRead()), this, SLOT(dataAvailable()), Qt::UniqueConnection);

    m_data[reply].first = requestData;

    return reply;
}
