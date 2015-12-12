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
 
#include "checkboxheader.h"

#include <QMouseEvent>
#include <QDebug>

CheckBoxHeader::CheckBoxHeader(int checkableColumn, Qt::Orientation orientation, QWidget* parent)
    : QHeaderView(orientation, parent), m_column(checkableColumn), m_checked(false)
{
    setSectionsClickable(true);
    connect(this, SIGNAL(sectionPressed(int)), this, SLOT(onSectionPressed(int)));
}

void CheckBoxHeader::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();

    if (logicalIndex == 0) {
        QStyleOptionButton option;

        QRect checkBoxRect = style()->subElementRect(
            QStyle::SE_CheckBoxIndicator,
            &option
        );
        QPoint point(
            rect.x() + rect.width() / 2 -
                checkBoxRect.width() / 2,
            rect.y() + rect.height() / 2 -
                checkBoxRect.height() / 2
        );

        option.rect = QRect(point, checkBoxRect.size());;
        option.state |= QStyle::State_Enabled;
        option.state |= m_checked ? QStyle::State_On : QStyle::State_Off;

        style()->drawControl(QStyle::CE_CheckBox, &option, painter);
    }
}

void CheckBoxHeader::onSectionPressed(int logicalIndex)
{
    if (logicalIndex == m_column) {
        setChecked(!isChecked());
        emit checkBoxClicked(isChecked());
    }
}

void CheckBoxHeader::setChecked(bool checked)
{
    if (m_checked != checked) {
        m_checked = checked;
        viewport()->update();
    }
}
