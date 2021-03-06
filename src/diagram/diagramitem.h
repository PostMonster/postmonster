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
 
#ifndef DIAGRAMITEM_H
#define DIAGRAMITEM_H

#include <QGraphicsPixmapItem>
#include <QList>
#include <QMenu>
#include <QUuid>

#include "postmonster.h"

class Arrow;
class DiagramItem : public QGraphicsItem
{
public:
    enum { Type = UserType + 1 };
    enum DiagramType { TypeStart, TypeTask };
    enum ArrowDirection { In, Out };

    DiagramItem(QGraphicsItem *parent = 0);
    ~DiagramItem();

    void setMenu(QMenu *menu);
    const Arrow *arrow(PostMonster::TaskStatus status);
    void removeArrow(PostMonster::TaskStatus status);
    void addArrow(Arrow *arrow);
    const QList<Arrow *> *arrows() const;
    const QUuid &uuid() const;
    int type() const { return Type; }
    virtual DiagramType diagramType() const = 0;
    void setUuid(const QUuid &uuid) { m_uuid = uuid; }
    bool hasBreakpoint();
    void setBreakpoint(bool flag);
    bool isCurrent();
    void setCurrent(bool flag);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);

protected:
    void destroyArrow(Arrow *arrow);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

public slots:
    void removeArrows();
    void toFront();
    void toBack();

private:
    QUuid m_uuid;
    QMenu *m_menu;
    QList<Arrow *> m_arrows;
    bool m_breakpoint;
    bool m_current;
};

#endif // DIAGRAMITEM_H
