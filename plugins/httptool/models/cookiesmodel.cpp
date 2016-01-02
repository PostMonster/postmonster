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
 
#include "models/cookiesmodel.h"

CookiesModel::CookiesModel(QObject *parent) :
    QAbstractTableModel(parent),
    m_cookies(0)
{

}

CookiesModel::~CookiesModel()
{

}

QVariant CookiesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() > rowCount() - 1)
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        const QNetworkCookie *cookie = &m_cookies->at(index.row());
        switch (index.column()) {
        case Domain:
            return cookie->domain();

        case Name:
            return QUrl::fromPercentEncoding(cookie->name());

        case Value:
            return QUrl::fromPercentEncoding(cookie->value());
        }
    }

    return QVariant();
}

bool CookiesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole || !index.isValid() || index.row() > rowCount() - 1)
        return false;

    QNetworkCookie cookie = m_cookies->at(index.row());
    QByteArray newValue = QUrl::toPercentEncoding(value.toByteArray());

    switch (index.column()) {
    case Name:
        foreach (const QNetworkCookie &c, *m_cookies) {
            const QString &name = QLatin1String(c.name());
            if (!name.compare(newValue, Qt::CaseInsensitive))
                return false;
        }

        cookie.setName(newValue);
        break;

    case Domain:
        cookie.setDomain(newValue);
        break;

    case Value:
        cookie.setValue(newValue);
        break;

    default:
        return false;
    }

    m_cookies->replace(index.row(), cookie);

    return true;
}

int CookiesModel::rowCount(const QModelIndex &) const
{
    return (m_cookies) ? m_cookies->count() : 0;
}

int CookiesModel::columnCount(const QModelIndex &) const
{
    return _columnCount;
}

bool CookiesModel::removeRow(int row, const QModelIndex &)
{
    if (row < 0 || row > rowCount() - 1)
        return false;

    beginRemoveRows(QModelIndex(), row, row);
    m_cookies->removeAt(row);
    endRemoveRows();

    return true;
}

bool CookiesModel::removeRows(int row, int count, const QModelIndex &)
{
    if (row < 0 || count < 0
            || row > rowCount() - 1 || row + count > rowCount())
        return false;

    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = row; i < row + count; i++)
        m_cookies->removeAt(row);
    endRemoveRows();

    return true;
}

QVariant CookiesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   if (role != Qt::DisplayRole)
       return QVariant();

   if (orientation == Qt::Horizontal) {
       switch (section) {
       case Domain: return tr("Domain");
       case Name:   return tr("Name");
       case Value:  return tr("Value");
       }
   } else {
       return section + 1;
   }

   return QVariant();
}

Qt::ItemFlags CookiesModel::flags(const QModelIndex &index) const
{
   if (!index.isValid())
        return Qt::NoItemFlags;

   return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

void CookiesModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, rowCount());
    m_cookies->clear();
    endRemoveRows();
}

void CookiesModel::add(const QNetworkCookie &cookie)
{
    foreach (const QNetworkCookie &c, *m_cookies)
        if (!QString(c.name()).compare(cookie.name(), Qt::CaseInsensitive))
            return;

    emit beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_cookies->append(cookie);
    emit endInsertRows();
}

void CookiesModel::setCookies(QList<QNetworkCookie> *cookies)
{
    beginResetModel();
    m_cookies = cookies;
    endResetModel();
}
