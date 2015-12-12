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
 
#include "requestsproxymodel.h"
#include <QDebug>
RequestsProxyModel::RequestsProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

bool RequestsProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    QString leftString = sourceModel()->data(left).toString();
    QString rightString = sourceModel()->data(right).toString();

    return QString::localeAwareCompare(leftString, rightString) < 0;
}

bool RequestsProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    bool result = true;
    int columnCount = sourceModel()->columnCount();
    for (int column = 0; column < columnCount && result; column++) {
        QVariant data = sourceModel()->data(sourceModel()->index(sourceRow, column, sourceParent));

        if (m_filters.contains(column) && !m_filters[column].exactMatch(data.toString()))
            result = false;
    }

    return result;
}

void RequestsProxyModel::setFilter(int column, QString value)
{
    if (value.isEmpty()) {
        removeFilter(column);
        return;
    }

    m_filters.insert(column, QRegExp(value, Qt::CaseInsensitive, QRegExp::Wildcard));
}

void RequestsProxyModel::removeFilter(int column)
{
    m_filters.remove(column);
}
