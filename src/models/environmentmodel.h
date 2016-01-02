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
 
#ifndef ENVIRONMENTMODEL_H
#define ENVIRONMENTMODEL_H

#include <QAbstractItemModel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QIcon>

#include "postmonster.h"
#include "jsontreeitem.h"

class EnvironmentModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Column {
        Name,
        Value,
        _columnCount
    };

    explicit EnvironmentModel(QObject *parent = 0);
    ~EnvironmentModel();

    bool load(const QString& fileName);
    bool load(QIODevice * device);
    bool loadJson(const QByteArray& json);
    bool loadJson(const QJsonObject& json);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QModelIndex index(int row, int column,const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    void setIcon(const QJsonValue::Type& type, const QIcon& icon);
    void setColor(const QModelIndex &index, const QColor &color);

private:
    JsonTreeItem *m_rootItem;
    QStringList m_headers;
    QHash<QJsonValue::Type, QIcon> m_typeIcons;
    QHash<QModelIndex, QColor> m_colors;
};

#endif // ENVIRONMENTMODEL_H
