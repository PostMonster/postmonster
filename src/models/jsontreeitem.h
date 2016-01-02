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
 
#ifndef JSONTREEITEM_H
#define JSONTREEITEM_H

#include <QtCore>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>

class JsonTreeItem
{
public:
    JsonTreeItem(JsonTreeItem *parent = 0);
    ~JsonTreeItem();

    void appendChild(JsonTreeItem *item);
    JsonTreeItem *child(int row);
    JsonTreeItem *parent();
    int childCount() const;
    int row() const;
    void setKey(const QString &key);
    void setValue(const QString &value);
    void setType(const QJsonValue::Type &type);
    QString key() const;
    QString value() const;
    QJsonValue::Type type() const;

    static JsonTreeItem *load(const QJsonValue &value, JsonTreeItem *parent = 0);

private:
    QString m_key;
    QString m_value;
    QJsonValue::Type m_type;

    QList<JsonTreeItem *> m_children;
    JsonTreeItem *m_parent;
};

#endif // JSONTREEITEM_H
