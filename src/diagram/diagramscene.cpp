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
 
#include "diagramscene.h"

#include <QGraphicsSceneMouseEvent>
#include <QtDebug>
#include <QMimeData>
#include <QPainter>

#include "arrow.h"
#include "startitem.h"
#include "taskitem.h"

DiagramScene::DiagramScene(QObject *parent)
    : QGraphicsScene(parent)
{
    m_mode = InsertItem;
}

void DiagramScene::setMode(Mode mode, PostMonster::ToolPluginInterface *tool)
{
    m_mode = mode;
    m_tool = tool;
}

/*void DiagramScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    painter->drawTiledPixmap(rect, QPixmap(":/icons/diagram/background"));
}*/

void DiagramScene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    event->acceptProposedAction();
}

void DiagramScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/vnd.request.row"))
        event->acceptProposedAction();
}

void DiagramScene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    emit requestDropped(event->mimeData()->data("application/vnd.request.row").toInt(),
                        event->scenePos());
    event->acceptProposedAction();
}

DiagramItem *DiagramScene::drawStart()
{
    DiagramItem *item = new StartItem();
    addItem(item);

    return item;
}

void DiagramScene::drawLine()
{
    if (m_mode == InsertLine)
        return;

    m_prevMode = m_mode;
    m_mode = InsertLine;
    m_lineStatus = PostMonster::Default;

    m_line = new QGraphicsLineItem(QLineF(m_clickPos, m_clickPos));
    m_line->setPen(QPen(Qt::gray, 1.2, Qt::DashLine));
    addItem(m_line);
}

void DiagramScene::drawOkLine()
{
    drawLine();
    m_lineStatus = PostMonster::Ok;
}

void DiagramScene::drawFailLine()
{
    drawLine();
    m_lineStatus = PostMonster::Fail;
}

void DiagramScene::removeOkLine()
{
    foreach (QGraphicsItem *item, selectedItems()) {
        DiagramItem *diagramItem = qgraphicsitem_cast<DiagramItem *>(item);

        if (diagramItem)
            diagramItem->removeArrow(PostMonster::Ok);
    }
}

void DiagramScene::removeFailLine()
{
    foreach (QGraphicsItem *item, selectedItems()) {
        DiagramItem *diagramItem = qgraphicsitem_cast<DiagramItem *>(item);

        if (diagramItem)
            diagramItem->removeArrow(PostMonster::Fail);
    }
}

void DiagramScene::insertItem(DiagramItem *item, QPointF scenePos)
{
    QMenu *menu = new QMenu;
    QMenu *connectMenu, *disconnectMenu;

    switch (item->diagramType()) {
    case DiagramItem::TypeTask:
        connectMenu = menu->addMenu(tr("Connnect"));
        disconnectMenu = menu->addMenu(tr("Disconnect"));
        foreach (PostMonster::TaskStatus status, static_cast<TaskItem *>(item)->tool()->statuses()) {
            if (status == PostMonster::Ok) {
                connectMenu->addAction(tr("Success"), this, SLOT(drawOkLine()));
                disconnectMenu->addAction(tr("Success"), this, SLOT(removeOkLine()));
            } else if (status == PostMonster::Fail) {
                connectMenu->addAction(tr("Failure"), this, SLOT(drawFailLine()));
                disconnectMenu->addAction(tr("Failure"), this, SLOT(removeFailLine()));
            }
        }

        menu->addAction(tr("Delete"), this, SLOT(destroySelected()));

        connect(static_cast<TaskItem *>(item)->task(), SIGNAL(dataChanged()),
                this, SLOT(update()));
        break;

    case DiagramItem::TypeStart:
        menu->addAction(tr("Connect"), this, SLOT(drawLine()));
        menu->addAction(tr("Disconnect"), this, SLOT(disconnectSelected()));
    }

    menu->addSeparator();
    menu->addAction(tr("To front"), this, SLOT(selectedToFront()));
    menu->addAction(tr("To back"), this, SLOT(selectedToBack()));

    item->setMenu(menu);
    addItem(item);

    item->setPos(scenePos);

    emit itemInserted(item);
}

void DiagramScene::destroyItem(QGraphicsItem *item)
{
    DiagramItem *diagramItem = qgraphicsitem_cast<DiagramItem *>(item);
    if (diagramItem) {
        diagramItem->removeArrows();

        QHash<Arrow *, DiagramItem *> arrows;
        foreach (QGraphicsItem *i, items()) {
            DiagramItem *di = qgraphicsitem_cast<DiagramItem *>(i);
            if (!di) continue;

            for (QList<Arrow *>::const_iterator j = di->arrows()->begin(),
                 end = di->arrows()->end(); j != end; ++j)
                if ((*j)->endItem() == item)
                    arrows[(*j)] = di;

        }

        for (QHash<Arrow *, DiagramItem *>::iterator i = arrows.begin(),
             end = arrows.end(); i != end; ++i)
            i.value()->removeArrow(i.key());

        if (diagramItem->diagramType() == DiagramItem::TypeTask) {
            TaskItem *taskItem = static_cast<TaskItem *>(diagramItem);
            taskItem->tool()->destroyTask(taskItem->task());
        }
    }

    removeItem(item);
    delete item;
}

void DiagramScene::destroySelected()
{
    foreach (QGraphicsItem *item, selectedItems())
        destroyItem(item);
}

void DiagramScene::destroyItems()
{
    while (!items().empty())
        destroyItem(items().first());
}

void DiagramScene::disconnectSelected()
{
    foreach (QGraphicsItem *item, selectedItems()) {
        DiagramItem *diagramItem = qgraphicsitem_cast<DiagramItem *>(item);

        if (diagramItem)
            diagramItem->removeArrows();
    }
}

void DiagramScene::selectedToFront()
{
    foreach (QGraphicsItem *item, selectedItems()) {
        DiagramItem *diagramItem = qgraphicsitem_cast<DiagramItem *>(item);
        if (!diagramItem) continue;

        diagramItem->toFront();
    }
}

void DiagramScene::selectedToBack()
{
    foreach (QGraphicsItem *item, selectedItems()) {
        DiagramItem *diagramItem = qgraphicsitem_cast<DiagramItem *>(item);
        if (!diagramItem) continue;

        diagramItem->toBack();
    }
}

void DiagramScene::insertArrow(DiagramItem *start, DiagramItem *end)
{
    Arrow *arrow = new Arrow(m_lineStatus, start, end);

    for (QListIterator<Arrow *> i(*end->arrows()); i.hasNext(); ) {
        Arrow *a = i.next();
        if (a->status() == m_lineStatus && a->endItem() == start) {
            arrow->setPen(QPen(Qt::transparent));
            break;
        }
    }

    start->addArrow(arrow);
    arrow->setZValue(-1000.0);

    addItem(arrow);
    arrow->updatePosition();
}

void DiagramScene::insertArrow(PostMonster::TaskStatus status, DiagramItem *start, DiagramItem *end)
{
    m_lineStatus = status;
    insertArrow(start, end);
}

void DiagramScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_clickPos = event->scenePos();
    if (event->button() != Qt::LeftButton)
        return;

    DiagramItem *item;
    switch (m_mode) {
    case InsertItem:
        item = new TaskItem(m_tool->createTask());
        insertItem(item, event->scenePos());
        break;

    case InsertLine:
        if (m_line != 0 && m_mode == InsertLine) {
            QList<QGraphicsItem *> startItems = items(m_line->line().p1());
            if (startItems.count() && startItems.first() == m_line)
                startItems.removeFirst();
            QList<QGraphicsItem *> endItems = items(m_line->line().p2());
            if (endItems.count() && endItems.first() == m_line)
                endItems.removeFirst();

            removeItem(m_line);
            delete m_line;
            if (startItems.count() > 0 && endItems.count() > 0 &&
                startItems.first() != endItems.first()) {
                DiagramItem *startItem = qgraphicsitem_cast<DiagramItem *>(startItems.first());
                DiagramItem *endItem = qgraphicsitem_cast<DiagramItem *>(endItems.first());

                if (startItem && endItem) {
                    if (startItem->diagramType() == DiagramItem::TypeStart)
                        startItem->removeArrows();

                    insertArrow(startItem, endItem);
                }
            }
        }

        m_line = 0;
        m_mode = m_prevMode;
        break;

    default:
        if (!itemAt(event->scenePos(), QTransform()))
            emit clickedOnBackground();
    }

    QGraphicsScene::mousePressEvent(event);
}

void DiagramScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (m_mode == InsertLine && m_line != 0) {
        QLineF newLine(m_line->line().p1(), mouseEvent->scenePos());
        m_line->setLine(newLine);
    } else if (m_mode == MoveItem) {
        QGraphicsScene::mouseMoveEvent(mouseEvent);
    }
}

void DiagramScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{

    QGraphicsScene::mouseReleaseEvent(mouseEvent);
}
