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
 
#include "diagramitem.h"
#include "arrow.h"

#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QGraphicsDropShadowEffect>

#include <QtDebug>

#include "diagramscene.h"

DiagramItem::DiagramItem(QGraphicsItem *parent)
    : QGraphicsItem(parent), m_uuid(QUuid::createUuid()), m_menu(0), m_breakpoint(false)
{
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

DiagramItem::~DiagramItem()
{
    m_menu->deleteLater();
}

const QUuid &DiagramItem::uuid() const
{
    return m_uuid;
}

void DiagramItem::setMenu(QMenu *menu)
{
    m_menu = menu;
}

void DiagramItem::toFront()
{
    QList<QGraphicsItem *> overlapItems = collidingItems();

    qreal zValue = 0;
    foreach (const QGraphicsItem *item, overlapItems) {
        if (item->zValue() >= zValue)
            zValue = item->zValue() + 0.1;
    }
    setZValue(zValue);
}

void DiagramItem::toBack()
{
    QList<QGraphicsItem *> overlapItems = collidingItems();

    qreal zValue = 0;
    foreach (const QGraphicsItem *item, overlapItems) {
        if (item->zValue() <= zValue)
            zValue = item->zValue() - 0.1;
    }
    setZValue(zValue);
}


void DiagramItem::removeArrow(Arrow *arrow)
{
    int index = m_arrows.indexOf(arrow);
    if (index < 0) return;

    const QList<Arrow *> *endArrows = arrow->endItem()->arrows();
    for (QList<Arrow *>::const_iterator i = endArrows->constBegin(),
         end = endArrows->constEnd(); i != end; ++i) {
        if ((*i)->status() == arrow->status())
            (*i)->restorePen();
    }

    m_arrows.removeAt(index);
    scene()->removeItem(arrow);

    delete arrow;
}

void DiagramItem::removeArrow(PostMonster::TaskStatus status)
{
    foreach (Arrow *arrow, m_arrows) {
        if (arrow->status() == status) {
            removeArrow(arrow);
            break;
        }
    }
}

const Arrow *DiagramItem::arrow(PostMonster::TaskStatus status)
{
    foreach (const Arrow *arrow, m_arrows)
        if (arrow->status() == status)
            return arrow;

    return 0;
}


const QList<Arrow *> *DiagramItem::arrows() const
{
    return &m_arrows;
}

void DiagramItem::removeArrows()
{
    foreach (Arrow *arrow, m_arrows) {
        removeArrow(arrow);
    }
}

void DiagramItem::addArrow(Arrow *arrow)
{
    foreach (Arrow *a, m_arrows) {
        if (a->status() == arrow->status())
            removeArrow(a);
    }

    m_arrows.append(arrow);
}

void DiagramItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    scene()->clearSelection();
    setSelected(true);

    if (m_menu)
        m_menu->exec(event->screenPos());
}

QVariant DiagramItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionChange) {
        foreach (Arrow *arrow, m_arrows)
            arrow->updatePosition();
    }

    return value;
}

void DiagramItem::setBreakpoint(bool flag)
{
    m_breakpoint = flag;

    if (m_breakpoint) {
        QGraphicsDropShadowEffect *glow = new QGraphicsDropShadowEffect;
        glow->setBlurRadius(10);
        glow->setColor(Qt::green);
        glow->setXOffset(0);
        glow->setYOffset(0);
        setGraphicsEffect(glow);
    } else {
        setGraphicsEffect(nullptr);
    }
}

bool DiagramItem::hasBreakpoint()
{
    return m_breakpoint;
}
