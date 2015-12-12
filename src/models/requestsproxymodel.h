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
 
#ifndef REQUESTSPROXYMODEL_H
#define REQUESTSPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QHash>

class RequestsProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit RequestsProxyModel(QObject *parent = 0);
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    void setFilter(int column, QString value);
    void removeFilter(int column);

private:
    QHash<int, QRegExp> m_filters;
};

#endif // REQUESTSPROXYMODEL_H
