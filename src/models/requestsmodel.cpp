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
 
#include "requestsmodel.h"

#include <QColor>
#include <QFont>
#include <QMutexLocker>

using namespace PostMonster;

RequestsModel::RequestsModel(QObject *parent)
    : QAbstractTableModel(parent)
{

}

RequestsModel::~RequestsModel()
{
    clear();
}

QVariant RequestsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() ||
            index.row() > rowCount() - 1 || index.column() >= _columnCount)
        return QVariant();

    const HttpRequest *request = m_requests[index.row()].first;
    const HttpResponse *response = m_requests[index.row()].second;

    if (role == Qt::TextAlignmentRole)
        switch (index.column()) {
        case Method:
        case Code:
            return Qt::AlignCenter;
        default:
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }

    if (role == Qt::BackgroundColorRole && index.column() == Code) {
        int code = response->status;

        if (code >= 200 && code < 300)
            return QColor(Qt::green);
        if (code >= 400)
            return QColor(Qt::red);

        return QColor("#FFDB4C");
    }

    if (role == Qt::TextColorRole && index.column() == Code) {
        if (response->status >= 400)
            return QColor(Qt::white);

        return QVariant();
    }

    if (role == Qt::FontRole && index.column() == Method) {
        QFont font;
        font.setBold(true);
        return font;
    }

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case Used:
            return m_used.contains(index.row()) ? 1 : 0;

        case Method:
            return request->method;

        case URL:
            return request->url;

        case Type:
            return response->mimeType;

        case Code:
            return (response->status) ? response->status : QVariant("---");
        }
    } else if (role == Qt::CheckStateRole && index.column() == Used) {
        return m_used.contains(index.row()) ? Qt::Checked : Qt::Unchecked;
    }

    return QVariant();
}

bool RequestsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() ||
            index.row() > rowCount() - 1 || index.column() >= _columnCount)
        return false;

    if (role != Qt::EditRole)
        return false;

    switch (index.column()) {
    case Used:
        if (value.toBool())
            m_used.insert(index.row(), true);
        else
            m_used.remove(index.row());

        emit dataChanged(index, index);
        return true;
    }

    return false;
}

int RequestsModel::rowCount(const QModelIndex &) const
{
    return m_requests.count();
}

int RequestsModel::columnCount(const QModelIndex &) const
{
    return _columnCount;
}

bool RequestsModel::removeRow(int row, const QModelIndex &)
{
    return removeRows(row, 1);
}

bool RequestsModel::removeRows(int row, int count, const QModelIndex &)
{
    if (row < 0 || count < 0
            || row > rowCount() - 1 || row + count > rowCount())
        return false;

    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = row; i < row + count; i++) {
        HttpRequest *request = m_requests[row].first;
        HttpResponse *response = m_requests[row].second;

        m_requests.removeAt(row);
        m_used.remove(row);

        delete request;
        delete response;
    }
    endRemoveRows();

    return true;
}

QVariant RequestsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   if (role != Qt::DisplayRole)
       return QVariant();

   if (orientation == Qt::Horizontal) {
       switch (section) {
       case Method: return tr("Method");
       case URL:    return tr("URL");
       case Type:   return tr("Type");
       case Code:   return tr("Status");
       default:     return "";
       }
   } else {
       return section + 1;
   }
}

Qt::ItemFlags RequestsModel::flags(const QModelIndex &index) const
{
   if (!index.isValid())
        return Qt::NoItemFlags;

   Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
   if (index.column() == Used) flags |= Qt::ItemIsEditable;

   return flags;
}

void RequestsModel::add(HttpRequest *request, HttpResponse *response, bool used)
{
    QMutexLocker locker(&m_addMutex); //TODO is mutex necessary??

    emit beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_requests << QPair<HttpRequest *, HttpResponse *>(request, response);
    if (used)
        m_used[m_requests.count() - 1] = true;
    emit endInsertRows();
}

bool RequestsModel::used(int row) const
{
    if (m_used.contains(row) && m_used[row])
        return true;

    return false;
}

void RequestsModel::clear()
{
    removeRows(0, rowCount());
}

QList<QNetworkReply::RawHeaderPair> *RequestsModel::headers(int row)
{
    if (row < 0 || row > rowCount() - 1)
        return 0;

    return &m_requests[row].first->headers;
}

/*RequestsModel *RequestsModel::clone(QObject *parent) const
{
    return new RequestsModel(m_requests, parent);
}*/

HttpRequest *RequestsModel::request(int row) const
{
    if (row < 0 || row > rowCount() - 1)
        return 0;

    return m_requests[row].first;
}

HttpResponse *RequestsModel::response(int row) const
{
    if (row < 0 || row > rowCount() - 1)
        return 0;

    return m_requests[row].second;
}

int RequestsModel::findRow(const HttpRequest *request)
{
    for (int i = 0; i < rowCount(); i++)
        if (m_requests[i].first == request)
            return i;

    return -1;
}

QStringList RequestsModel::mimeTypes() const
{
    QStringList types;
    types << "application/vnd.request.row";

    return types;
}

QMimeData *RequestsModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    mimeData->setData("application/vnd.request.row", QByteArray::number(indexes[0].row()));

    return mimeData;
}
