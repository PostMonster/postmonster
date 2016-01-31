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
 
#include "arrow.h"
#include "taskitem.h"

#include <math.h>

#include <QtDebug>
#include <QPen>
#include <QPainter>

const qreal Pi = 3.14;

Arrow::Arrow(PostMonster::TaskStatus status, DiagramItem *startItem, DiagramItem *endItem, QGraphicsItem *parent)
    : QGraphicsLineItem(parent), m_startItem(startItem), m_endItem(endItem), m_status(status)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    restorePen();
}

void Arrow::restorePen()
{
    switch (m_status) {
    case PostMonster::Ok:
        m_color = Qt::darkGreen;
        break;

    case PostMonster::Fail:
        m_color = Qt::darkRed;
        break;

    default:
        m_color = QColor("#666666");
    }

    setPen(QPen(m_color, 1.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
}

QRectF Arrow::boundingRect() const
{
    qreal extra = (pen().width() + 20) / 2.0;

    return QRectF(line().p1(), QSizeF(line().p2().x() - line().p1().x(),
                                      line().p2().y() - line().p1().y()))
        .normalized()
        .adjusted(-extra, -extra, extra, extra);
}

QPainterPath Arrow::shape() const
{
    QPainterPath path = QGraphicsLineItem::shape();
    path.addPolygon(arrowHead);
    return path;
}

void Arrow::updatePosition()
{
    //QLineF line(mapFromItem(m_startItem, 0, 0), mapFromItem(m_endItem, 0, 0));
    //setLine(line);

    const QPointF startPos = m_startItem->pos();
    const QPointF endPos = m_endItem->pos();

    const qreal startScale = m_startItem->scale();
    const qreal endScale = m_endItem->scale();

    const int deltaX = qAbs(startPos.x() - endPos.x());
    const int deltaY = qAbs(startPos.y() - endPos.y());
    const QPointF pointDeltaX = QPointF(m_startItem->boundingRect().width() / 4, 0) * startScale;
    const QPointF pointDeltaY = QPointF(0, m_startItem->boundingRect().height() / 4) * startScale;

    QLineF centerLine(startPos + m_startItem->boundingRect().center() * startScale,
                      endPos + m_endItem->boundingRect().center() * endScale);

    switch (m_status) {
    case PostMonster::Ok:
    case PostMonster::True:
        if (deltaX < deltaY)
            centerLine = QLineF(centerLine.p1() + pointDeltaX, centerLine.p2() + pointDeltaX);
        else
            centerLine = QLineF(centerLine.p1() - pointDeltaY, centerLine.p2() - pointDeltaY);
        break;

    case PostMonster::Fail:
    case PostMonster::False:
        if (deltaX < deltaY)
            centerLine = QLineF(centerLine.p1() - pointDeltaX, centerLine.p2() - pointDeltaX);
        else
            centerLine = QLineF(centerLine.p1() + pointDeltaY, centerLine.p2() + pointDeltaY);
        break;

    case PostMonster::Default:
        break;
    }

    QPolygonF itemPolygon = m_endItem->shape().toFillPolygon();
    QPointF p1 = itemPolygon.first() + endPos;
    QPointF p2;
    QPointF startPoint, endPoint;
    QLineF polyLine;
    for (int i = 1; i < itemPolygon.count(); ++i) {
        p2 = itemPolygon.at(i) * endScale + endPos;
        polyLine = QLineF(p1, p2);
        QLineF::IntersectType intersectType =
            polyLine.intersect(centerLine, &endPoint);
        if (intersectType == QLineF::BoundedIntersection)
            break;
        p1 = p2;
    }

    itemPolygon = m_startItem->shape().toFillPolygon();
    p1 = itemPolygon.first() + startPos;
    for (int i = 1; i < itemPolygon.count(); ++i) {
        p2 = itemPolygon.at(i) * startScale + startPos;
        polyLine = QLineF(p1, p2);
        QLineF::IntersectType intersectType =
            polyLine.intersect(centerLine, &startPoint);
        if (intersectType == QLineF::BoundedIntersection)
            break;
        p1 = p2;
    }

    setLine(QLineF(endPoint, startPoint));
}

void Arrow::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (m_startItem->collidesWithItem(m_endItem))
        return;

    painter->setBrush(m_color);

    qreal arrowSize = 12;
    double angle = ::acos(line().dx() / line().length());
    if (line().dy() >= 0)
        angle = (Pi * 2) - angle;

    QPointF arrowP1 = line().p1() + QPointF(sin(angle + Pi / 3) * arrowSize,
                                    cos(angle + Pi / 3) * arrowSize);
    QPointF arrowP2 = line().p1() + QPointF(sin(angle + Pi - Pi / 3) * arrowSize,
                                    cos(angle + Pi - Pi / 3) * arrowSize);

    arrowHead.clear();
    arrowHead << line().p1() << arrowP1 << arrowP2;

    painter->setPen(pen());
    painter->drawLine(line());
    painter->setPen(QPen(Qt::transparent));
    painter->drawPolygon(arrowHead);
    painter->setPen(pen());

    QString text;
    switch (m_status) {
    case PostMonster::Ok:
        text = QStringLiteral("OK");
        break;

    case PostMonster::Fail:
        text = QStringLiteral("FAIL");
        break;

    default:
        break;
    }

    painter->setFont(QFont("sans", 8));

    QFontMetrics fm(painter->font());
    int textWidth = fm.width(text);

    if (textWidth * 3 > line().length())
        return;

    if (angle > Pi / 2 && angle < 3 * Pi / 2)
        angle += Pi;

    QPointF textP = (line().p1() + line().p2()) / 2;
    painter->save();
    painter->translate(textP);
    painter->rotate(-angle * 180 / Pi);
    painter->drawText(QPointF(-textWidth / 2, -5), text);
    painter->restore();

    /*if (isSelected()) {
        painter->setPen(QPen(m_color, 1, Qt::DashLine));
        QLineF myLine = line();
        myLine.translate(0, 4.0);
        painter->drawLine(myLine);
        myLine.translate(0,-8.0);
        painter->drawLine(myLine);
    }*/
}
