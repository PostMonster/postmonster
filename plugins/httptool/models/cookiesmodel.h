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
 
#ifndef COOKIESMODEL_H
#define COOKIESMODEL_H

#include <QAbstractTableModel>
#include <QNetworkReply>
#include <QNetworkCookie>

class CookiesModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Column {
        Domain,
        Name,
        Value,
        _columnCount
    };

    explicit CookiesModel(QObject *parent = 0);
    ~CookiesModel();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    int rowCount(const QModelIndex & = QModelIndex()) const;
    int columnCount(const QModelIndex & = QModelIndex()) const;

    bool removeRow(int row, const QModelIndex & = QModelIndex());
    bool removeRows(int row, int count, const QModelIndex & = QModelIndex());

    QVariant headerData(int section,Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    void add(const QNetworkCookie &cookie);
    void clear();
    void setCookies(QList<QNetworkCookie> *cookies);
    const QList<QNetworkCookie> *cookies();

private:
    QList<QNetworkCookie> *m_cookies;
};

#endif // COOKIESMODEL_H
