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
 
#include "common.h"
#include "taskitem.h"
#include "diagramscene.h"

#include <QtDebug>
#include <QPainter>
#include <QJsonObject>

TaskItem::TaskItem(PostMonster::TaskInterface *task, QGraphicsItem *parent)
    : DiagramItem(parent), QGraphicsPixmapItem(task->itemPixmap(), parent),
      m_task(task)
{
    DiagramItem::setScale(1 / Common::dpiScaleFactor());
}

QRectF TaskItem::boundingRect() const
{
    return QGraphicsPixmapItem::boundingRect();
}

QPainterPath TaskItem::shape() const
{
    return m_task->itemShape();
}

void TaskItem::updatePixmap()
{
    setPixmap(m_task->itemPixmap());
}

void TaskItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsPixmapItem::paint(painter, option, widget);
    DiagramItem::paint(painter, option, widget);
}
