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
 
#ifndef REQUESTSMODEL_H
#define REQUESTSMODEL_H

#include <QAbstractTableModel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMimeData>
#include <QMutex>
#include <QHash>

#include "postmonster.h"

class RequestsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Column {
        Used,
        Method,
        URL,
        Type,
        Code,
        _columnCount
    };

    explicit RequestsModel(QObject *parent = 0);
    ~RequestsModel();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    int rowCount(const QModelIndex & = QModelIndex()) const;
    int columnCount(const QModelIndex & = QModelIndex()) const;

    bool removeRow(int row, const QModelIndex & = QModelIndex());
    bool removeRows(int row, int count, const QModelIndex & = QModelIndex());

    QVariant headerData(int section,Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    void add(PostMonster::HttpRequest *request,
             PostMonster::HttpResponse *response, bool used = false);
    void clear();
    PostMonster::HttpRequest *request(int row) const;
    PostMonster::HttpResponse *response(int row) const;
    bool used(int row) const;
    int findRow(const PostMonster::HttpRequest *request);
    QList<QNetworkReply::RawHeaderPair> *headers(int row);

    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;

private:
    QList< QPair<PostMonster::HttpRequest *, PostMonster::HttpResponse *> > m_requests;
    QHash<int, bool> m_used;
    QMutex m_addMutex;
};

#endif // REQUESTSMODEL_H
