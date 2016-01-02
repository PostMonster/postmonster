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

#include "jsontreeitem.h"

JsonTreeItem::JsonTreeItem(JsonTreeItem *parent) :
    m_parent(parent)
{
}

JsonTreeItem::~JsonTreeItem()
{
    qDeleteAll(m_children);
}

void JsonTreeItem::appendChild(JsonTreeItem *item)
{
    m_children.append(item);
}

JsonTreeItem *JsonTreeItem::child(int row)
{
    return m_children.value(row);
}

JsonTreeItem *JsonTreeItem::parent()
{
    return m_parent;
}

int JsonTreeItem::childCount() const
{
    return m_children.count();
}

int JsonTreeItem::row() const
{
    if (m_parent)
        return m_parent->m_children.indexOf(const_cast<JsonTreeItem *>(this));

    return 0;
}

void JsonTreeItem::setKey(const QString &key)
{
    m_key = key;
}

void JsonTreeItem::setValue(const QString &value)
{
    m_value = value;
}

void JsonTreeItem::setType(const QJsonValue::Type &type)
{
    m_type = type;
}

QString JsonTreeItem::key() const
{
    return m_key;
}

QString JsonTreeItem::value() const
{
    return m_value;
}

QJsonValue::Type JsonTreeItem::type() const
{
    return m_type;
}

JsonTreeItem *JsonTreeItem::load(const QJsonValue &value, JsonTreeItem *parent)
{
    JsonTreeItem *rootItem = new JsonTreeItem(parent);
    rootItem->setKey("root");

    if (value.isObject()) {
        //Get all QJsonValue children
        foreach (const QString &key, value.toObject().keys()) {
            QJsonValue v = value.toObject().value(key);
            JsonTreeItem *child = load(v,rootItem);

            child->setKey(key);
            child->setType(v.type());
            rootItem->appendChild(child);
        }
    } else if (value.isArray()) {
        //Get all QJsonValue children
        int index = 0;
        foreach (const QJsonValue &v, value.toArray()){
            JsonTreeItem *child = load(v,rootItem);

            child->setKey(QString::number(index));
            child->setType(v.type());
            rootItem->appendChild(child);

            ++index;
        }
    } else {
        rootItem->setValue(value.toVariant().toString());
        rootItem->setType(value.type());
    }

    return rootItem;
}

