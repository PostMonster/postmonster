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
 
#ifndef TASKITEM_H
#define TASKITEM_H

#include <QGraphicsPixmapItem>

#include "diagramitem.h"
#include "toolplugin.h"

class TaskItem : public DiagramItem, public QGraphicsPixmapItem
{
public:
    TaskItem(PostMonster::TaskInterface *task, QGraphicsItem *parent = 0);
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);
    PostMonster::TaskInterface *task() { return m_task; }
    PostMonster::ToolPluginInterface *tool() { return m_task->tool(); }
    DiagramType diagramType() const { return DiagramItem::TypeTask; }

private:
    PostMonster::TaskInterface *m_task;
};

#endif // TASKITEM_H
