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
 
#ifndef ARROW_H
#define ARROW_H

#include <QGraphicsLineItem>

#include "postmonster.h"
#include "diagramitem.h"

class Arrow : public QGraphicsLineItem
{
public:
    enum { Type = UserType + 2 };

    Arrow(PostMonster::TaskStatus status, DiagramItem *startItem, DiagramItem *endItem,
          QGraphicsItem *parent = 0);

    int type() const Q_DECL_OVERRIDE { return Type; }
    PostMonster::TaskStatus status() const { return m_status; }
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;
    DiagramItem *startItem() const { return m_startItem; }
    DiagramItem *endItem() const { return m_endItem; }
    void restorePen();

    void updatePosition();

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) Q_DECL_OVERRIDE;

private:
    DiagramItem *m_startItem;
    DiagramItem *m_endItem;
    QColor m_color;
    QPolygonF arrowHead;
    PostMonster::TaskStatus m_status;
};

#endif // ARROW_H
