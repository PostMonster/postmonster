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
 
#ifndef DIAGRAMSCENE_H
#define DIAGRAMSCENE_H

#include <QGraphicsScene>
#include <QMenu>

#include "postmonster.h"
#include "toolplugin.h"

#include "diagramitem.h"

class DiagramScene : public QGraphicsScene
{
    Q_OBJECT

public:
    enum Mode { InsertItem, InsertLine, MoveItem };

    explicit DiagramScene(QSizeF canvasSize, QObject *parent = 0);

    void insertItem(DiagramItem *item);
    void insertArrow(DiagramItem *start, DiagramItem *end);
    void insertArrow(PostMonster::TaskStatus status, DiagramItem *start, DiagramItem *end);

    void setCurrentItem(DiagramItem *item);
    DiagramItem *currentItem();

    void drawBackground(QPainter * painter, const QRectF & rect) override;
    QSizeF canvasSize();

public slots:
    void destroyItems();
    void destroyItem(DiagramItem *item);
    Mode mode();
    void setMode(Mode mode, PostMonster::ToolPluginInterface *tool = 0);
    DiagramItem *drawStart();
    void updateCanvas();

signals:
    void itemInserted(DiagramItem *item);
    void clickedOnBackground();
    void requestDropped(int requestRow, QPointF scenePos);
    void currentItemChanged();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) Q_DECL_OVERRIDE;
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QGraphicsSceneDragDropEvent *event) Q_DECL_OVERRIDE;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) Q_DECL_OVERRIDE;

protected slots:
    void removeLine(PostMonster::TaskStatus status);

    void menuConnect();
    void menuConnectOk();
    void menuConnectFail();

    void menuDisconnect();
    void menuDisconnectOk();
    void menuDisconnectFail();

    void menuDelete();
    void menuToFront();
    void menuToBack();

private:
    Mode m_mode, m_prevMode;
    PostMonster::TaskStatus m_lineStatus;
    QMenu m_itemMenu;
    QGraphicsLineItem *m_line;
    QPointF m_clickPos;
    PostMonster::ToolPluginInterface *m_tool;
    DiagramItem *m_currentItem;
    QSizeF m_canvasSize;
};

#endif // DIAGRAMSCENE_H
