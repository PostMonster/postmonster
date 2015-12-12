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
 
#include "checkboxdelegate.h"

#include <QApplication>
#include <QPainter>

CheckBoxDelegate::CheckBoxDelegate(QObject *parent):
    QStyledItemDelegate(parent) {}

QRect CheckBoxDelegate::checkBoxRect(
    const QStyleOptionViewItem &viewItemOption
) {
    QStyleOptionButton checkBoxOption;
    QRect rect = qApp->style()->subElementRect(
        QStyle::SE_CheckBoxIndicator,
        &checkBoxOption
    );

    QPoint point(
        viewItemOption.rect.x() + (viewItemOption.rect.width() -
            rect.width()) / 2 + 1,
        viewItemOption.rect.y() + (viewItemOption.rect.height() -
            rect.height()) / 2
    );

    return QRect(point, rect.size());
}

void CheckBoxDelegate::paint(
    QPainter *painter,
    const QStyleOptionViewItem &option,
    const QModelIndex &index
) const {
    if (!(option.state & QStyle::State_Selected) &&
        !index.data(Qt::BackgroundRole).isNull())
        painter->fillRect(option.rect,
                          index.data(Qt::BackgroundRole).value<QBrush>());
    else
        QStyledItemDelegate::paint(painter, option, QModelIndex());

    QStyleOptionButton checkBoxOption;
    checkBoxOption.state |= QStyle::State_Enabled;
    checkBoxOption.rect = checkBoxRect(option);

    QVariant value = index.model()->data(index, Qt::CheckStateRole);
    if (!value.isValid()) return;

    Qt::CheckState checked = static_cast<Qt::CheckState>(value.toInt());
    if (checked == Qt::Checked)
        checkBoxOption.state |= QStyle::State_On;
    else
        checkBoxOption.state |= QStyle::State_Off;

    if (!(index.flags() & Qt::ItemIsEditable))
        checkBoxOption.state |= QStyle::State_ReadOnly;

    qApp->style()->drawControl(QStyle::CE_CheckBox, &checkBoxOption, painter);
}

bool CheckBoxDelegate::editorEvent(
    QEvent *event,
    QAbstractItemModel *model,
    const QStyleOptionViewItem &option,
    const QModelIndex &index
) {
    if (!(index.flags() & Qt::ItemIsEditable))
        return false;

    if ((event->type() == QEvent::MouseButtonRelease) ||
        (event->type() == QEvent::MouseButtonDblClick)) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        if (mouseEvent->button() != Qt::LeftButton ||
            !checkBoxRect(option).contains(mouseEvent->pos()))
            return false;
        if (event->type() == QEvent::MouseButtonDblClick)
            return true;
    } else if (event->type() == QEvent::KeyPress) {
        if (static_cast<QKeyEvent*>(event)->key() != Qt::Key_Space &&
            static_cast<QKeyEvent*>(event)->key() != Qt::Key_Select)
            return false;
    } else
        return false;

    Qt::CheckState checked =
            static_cast<Qt::CheckState>(index.model()->data(index, Qt::CheckStateRole).toInt());
    return model->setData(index,
                          (checked == Qt::Checked) ? Qt::Unchecked : Qt::Checked, Qt::EditRole);
}
