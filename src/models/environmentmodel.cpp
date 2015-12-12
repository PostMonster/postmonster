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
 
/***********************************************
    Copyright (C) 2014  Schutz Sacha
    This file is part of EnvironmentModel (https://github.com/dridk/EnvironmentModel).

    EnvironmentModel is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    EnvironmentModel is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with EnvironmentModel.  If not, see <http://www.gnu.org/licenses/>.

**********************************************/

#include "environmentmodel.h"

#include <QFile>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QIcon>
#include <QFont>

EnvironmentModel::EnvironmentModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    mRootItem = new JsonTreeItem;
}

bool EnvironmentModel::load(const QString &fileName)
{
    QFile file(fileName);
    bool success = false;
    if (file.open(QIODevice::ReadOnly)) {
        success = load(&file);
        file.close();
    }
    else success = false;

    return success;
}

bool EnvironmentModel::load(QIODevice *device)
{
    return loadJson(device->readAll());
}

bool EnvironmentModel::loadJson(const QByteArray &json)
{
    QJsonDocument document = QJsonDocument::fromJson(json);

    if (!document.isNull())
    {
        beginResetModel();
        mRootItem = JsonTreeItem::load(QJsonValue(document.object()));
        endResetModel();
        return true;
    }
    return false;
}

bool EnvironmentModel::loadJson(const QJsonObject &json)
{
    beginResetModel();
    mRootItem = JsonTreeItem::load(QJsonValue(json));
    endResetModel();
    return true;
}

QVariant EnvironmentModel::data(const QModelIndex &index, int role) const
{

    if (!index.isValid())
        return QVariant();


    JsonTreeItem *item = static_cast<JsonTreeItem*>(index.internalPointer());


    if ((role == Qt::DecorationRole) && (index.column() == 0)){

        return mTypeIcons.value(item->type());
    }

    if (role == Qt::BackgroundColorRole && m_colors.contains(index))
        return m_colors[index];


    if (role == Qt::DisplayRole) {

        if (index.column() == Name)
            return item->key();

        if (index.column() == Value) {
            if (item->value().startsWith("base64:"))
                return "BASE64";

            return item->value();
        }
    }



    return QVariant();

}

QVariant EnvironmentModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
            case Name:  return tr("Name");
            case Value: return tr("Value");
        }
    }

    return section + 1;
}

QModelIndex EnvironmentModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    JsonTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = mRootItem;
    else
        parentItem = static_cast<JsonTreeItem*>(parent.internalPointer());

    JsonTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex EnvironmentModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    JsonTreeItem *childItem = static_cast<JsonTreeItem*>(index.internalPointer());
    JsonTreeItem *parentItem = childItem->parent();

    if (parentItem == mRootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int EnvironmentModel::rowCount(const QModelIndex &parent) const
{
    JsonTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = mRootItem;
    else
        parentItem = static_cast<JsonTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int EnvironmentModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 2;
}

void EnvironmentModel::setIcon(const QJsonValue::Type &type, const QIcon &icon)
{
    mTypeIcons.insert(type,icon);
}

void EnvironmentModel::setColor(const QModelIndex &index, const QColor &color)
{
    m_colors.insert(index, color);
}
