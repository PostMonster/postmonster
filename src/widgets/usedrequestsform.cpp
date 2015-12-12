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
 
#include "usedrequestsform.h"

#include <QHeaderView>

UsedRequestsForm::UsedRequestsForm(QWidget *parent)
    : RequestsForm(parent)
{
    requestsTable()->setDragEnabled(true);
    requestsTable()->verticalHeader()->hide();
}

void UsedRequestsForm::setRequestsModel(RequestsModel *model)
{
    RequestsForm::setRequestsModel(model);
    m_proxyModel.setFilter(RequestsModel::Used, "1");

    hideUsedColumn();
    connect(&m_proxyModel, SIGNAL(layoutChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)),
            this, SLOT(hideUsedColumn()), Qt::UniqueConnection);
}

void UsedRequestsForm::hideUsedColumn()
{
    requestsTable()->hideColumn(RequestsModel::Used);
}

void UsedRequestsForm::removeRequests(const QModelIndexList &indexes)
{
    QModelIndexList::const_iterator i = indexes.constEnd();
    while (i != indexes.constBegin()) {
        qDebug() << "HIDE";
        --i;
        m_proxyModel.setData(m_proxyModel.index((*i).row(), RequestsModel::Used), false);
    }
}

bool UsedRequestsForm::isRowUsed(int row)
{
    RequestsModel *model = static_cast<RequestsModel *>(m_proxyModel.sourceModel());

    if (model->data(model->index(row, RequestsModel::Used)).toInt() == 0)
        return false;

    return true;
}
