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
    This file is part of QJsonModel (https://github.com/dridk/QJsonmodel).

    QJsonModel is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    QJsonModel is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QJsonModel.  If not, see <http://www.gnu.org/licenses/>.

**********************************************/

#include "jsontreeitem.h"

JsonTreeItem::JsonTreeItem(JsonTreeItem *parent)
{

    mParent = parent;


}

JsonTreeItem::~JsonTreeItem()
{
    qDeleteAll(mChilds);

}

void JsonTreeItem::appendChild(JsonTreeItem *item)
{
    mChilds.append(item);
}

JsonTreeItem *JsonTreeItem::child(int row)
{
    return mChilds.value(row);
}

JsonTreeItem *JsonTreeItem::parent()
{
    return mParent;
}

int JsonTreeItem::childCount() const
{
    return mChilds.count();
}

int JsonTreeItem::row() const
{
    if (mParent)
        return mParent->mChilds.indexOf(const_cast<JsonTreeItem*>(this));

    return 0;
}

void JsonTreeItem::setKey(const QString &key)
{
    mKey = key;
}

void JsonTreeItem::setValue(const QString &value)
{
    mValue = value;
}

void JsonTreeItem::setType(const QJsonValue::Type &type)
{
    mType = type;
}

QString JsonTreeItem::key() const
{
    return mKey;
}

QString JsonTreeItem::value() const
{
    return mValue;
}

QJsonValue::Type JsonTreeItem::type() const
{
    return mType;
}

JsonTreeItem* JsonTreeItem::load(const QJsonValue& value, JsonTreeItem* parent)
{


    JsonTreeItem * rootItem = new JsonTreeItem(parent);
    rootItem->setKey("root");

    if ( value.isObject())
    {

        //Get all QJsonValue childs
        foreach (QString key , value.toObject().keys()){
            QJsonValue v = value.toObject().value(key);
            JsonTreeItem * child = load(v,rootItem);
            child->setKey(key);
            child->setType(v.type());
            rootItem->appendChild(child);

        }

    }

    else if ( value.isArray())
    {
        //Get all QJsonValue childs
        int index = 0;
        foreach (QJsonValue v , value.toArray()){

            JsonTreeItem * child = load(v,rootItem);
            child->setKey(QString::number(index));
            child->setType(v.type());
            rootItem->appendChild(child);
            ++index;
        }
    }
    else
    {
        rootItem->setValue(value.toVariant().toString());
        rootItem->setType(value.type());
    }

    return rootItem;
}

