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
 
#include "models/headersmodel.h"

#include <QTextCodec>

HeadersModel::HeadersModel(QObject *parent) :
    QAbstractTableModel(parent),
    m_headers(0)
{

}

HeadersModel::~HeadersModel()
{

}

QVariant HeadersModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() ||
            index.row() > rowCount() - 1 || index.column() >= _columnCount)
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        const QNetworkReply::RawHeaderPair *header = &m_headers->at(index.row());
        switch (index.column()) {
        case Name:
            return QUrl::fromPercentEncoding(header->first);

        case Value:
            return QUrl::fromPercentEncoding(header->second);
        }
    }

    return QVariant();
}

bool HeadersModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole || !index.isValid() || index.row() > rowCount() - 1)
        return false;

    if (index.column() == Name &&
            !QTextCodec::codecForName("Latin1")->canEncode(value.toString()))
        return false;

    QNetworkReply::RawHeaderPair header = m_headers->at(index.row());
    QByteArray newValue = (index.column() == Name)
            ? value.toByteArray() : QUrl::toPercentEncoding(value.toString());

    switch (index.column()) {
    case Name:
        foreach (const QNetworkReply::RawHeaderPair &pair, *m_headers) {
            const QString &name = QLatin1String(pair.first);
            if (!name.compare(newValue, Qt::CaseInsensitive))
                return false;
        }

        header.first = newValue;
        break;

    case Value:
        header.second = newValue;
        break;

    default:
        return false;
    }

    m_headers->replace(index.row(), header);

    return true;
}

int HeadersModel::rowCount(const QModelIndex &) const
{
    return (m_headers) ? m_headers->count() : 0;
}

int HeadersModel::columnCount(const QModelIndex &) const
{
    return _columnCount;
}

bool HeadersModel::removeRow(int row, const QModelIndex &)
{
    if (row < 0 || row > rowCount() - 1)
        return false;

    beginRemoveRows(QModelIndex(), row, row);
    m_headers->removeAt(row);
    endRemoveRows();

    return true;
}

bool HeadersModel::removeRows(int row, int count, const QModelIndex &)
{
    if (row < 0 || count < 0
            || row > rowCount() - 1 || row + count > rowCount())
        return false;

    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = row; i < row + count; i++)
        m_headers->removeAt(row);
    endRemoveRows();

    return true;
}

QVariant HeadersModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   if (role != Qt::DisplayRole)
       return QVariant();

   if (orientation == Qt::Horizontal) {
       switch (section) {
       case Name:  return tr("Name");
       case Value: return tr("Value");
       }
   } else {
       return section + 1;
   }

   return QVariant();
}

Qt::ItemFlags HeadersModel::flags(const QModelIndex &index) const
{
   if (!index.isValid())
        return Qt::NoItemFlags;

   return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

void HeadersModel::add(const QNetworkReply::RawHeaderPair &header)
{
    foreach (const QNetworkReply::RawHeaderPair &pair, *m_headers)
        if (!QString(pair.first).compare(header.first, Qt::CaseInsensitive))
            return;

    emit beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_headers->append(header);
    emit endInsertRows();
}

void HeadersModel::setHeaders(QList<QNetworkReply::RawHeaderPair> *headers)
{
    beginResetModel();
    m_headers = headers;
    endResetModel();
}
