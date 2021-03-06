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
    : QGraphicsItem(parent), m_uuid(QUuid::createUuid()), m_menu(0),
      m_breakpoint(false), m_current(false)
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

    setZValue(zValue >= 0 ? zValue : 0);
}

void DiagramItem::destroyArrow(Arrow *arrow)
{
    arrow->startItem()->m_arrows.removeAll(arrow);
    arrow->endItem()->m_arrows.removeAll(arrow);

    scene()->removeItem(arrow);
    delete arrow;
}

void DiagramItem::removeArrow(PostMonster::TaskStatus status)
{
    foreach (Arrow *arrow, m_arrows) {
        if (arrow->status() == status && arrow->startItem() == this) {
            const QList<Arrow *> *endArrows = arrow->endItem()->arrows();
            for (QList<Arrow *>::const_iterator i = endArrows->constBegin(),
                 end = endArrows->constEnd(); i != end; ++i) {
                if ((*i)->status() == arrow->status() && (*i)->startItem() == arrow->endItem()) {
                    (*i)->restorePen();
                    break;
                }
            }

            destroyArrow(arrow);

            break;
        }
    }
}

const Arrow *DiagramItem::arrow(PostMonster::TaskStatus status)
{
    foreach (const Arrow *arrow, m_arrows)
        if (arrow->status() == status && arrow->startItem() == this)
            return arrow;

    return 0;
}


const QList<Arrow *> *DiagramItem::arrows() const
{
    return &m_arrows;
}

void DiagramItem::removeArrows()
{
    while (!m_arrows.empty())
        destroyArrow(m_arrows.first());
}

void DiagramItem::addArrow(Arrow *arrow)
{
    removeArrow(arrow->status());

    arrow->endItem()->m_arrows.append(arrow);
    m_arrows.append(arrow);
}

void DiagramItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (!isSelected())
        scene()->clearSelection();
    setSelected(true);

    if (m_menu)
        m_menu->exec(event->screenPos());
}

void DiagramItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    static bool busy = false;
    if (busy) return;

    busy = true;

    QGraphicsItem::mouseMoveEvent(event);

    foreach (Arrow *arrow, m_arrows)
        arrow->updatePosition();

    DiagramScene *diagramScene = qobject_cast<DiagramScene *>(scene());
    if (diagramScene) {
        diagramScene->updateCanvas();
    }

    busy = false;
}

void DiagramItem::setBreakpoint(bool flag)
{
    m_breakpoint = flag;
    update();
}

bool DiagramItem::hasBreakpoint()
{
    return m_breakpoint;
}

void DiagramItem::setCurrent(bool flag)
{
    m_current = flag;

    if (m_current) {
        QGraphicsDropShadowEffect *glow = new QGraphicsDropShadowEffect;
        glow->setBlurRadius(15 * Common::dpiScaleFactor());
        glow->setColor(Qt::black);
        glow->setXOffset(0);
        glow->setYOffset(0);
        setGraphicsEffect(glow);
    } else {
        setGraphicsEffect(nullptr);
    }
}

bool DiagramItem::isCurrent()
{
    return m_current;
}

void DiagramItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (m_breakpoint) {
        const qreal width = 1 * Common::dpiScaleFactor();
        const qreal scaleX = (shape().boundingRect().width() - width * 2) / shape().boundingRect().width();
        const qreal scaleY = (shape().boundingRect().height() - width * 2) / shape().boundingRect().height();

        painter->translate(width, width);
        painter->scale(scaleX, scaleY);
        painter->setPen(QPen(QColor("#D9283D"), width));
        painter->drawPath(shape());
    }
}

