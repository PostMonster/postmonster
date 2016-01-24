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
 
#include "pluginsmodel.h"

PluginsModel::PluginsModel(QObject *parent)
    : QAbstractTableModel(parent)
{

}

PluginsModel::~PluginsModel()
{
}

QVariant PluginsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() ||
            index.row() > rowCount() - 1 || index.column() >= _columnCount)
        return QVariant();

    if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
        case Version:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        default:
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }

    const int row = index.row();
    const PluginRegistry::PluginData *plugin = m_plugins.at(row);

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case Enabled:
            return m_enabled.contains(row) ? 1 : 0;

        case Name:
            return plugin->info["name"];

        case Description:
            return plugin->info["description"];

        case Version:
            return plugin->info["version"];
        }
    } else if (role == Qt::CheckStateRole && index.column() == Enabled) {
        return (m_enabled.contains(row)) ? Qt::Checked : Qt::Unchecked;
    }

    return QVariant();
}

bool PluginsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole || !index.isValid() ||
            index.row() > rowCount() - 1 || index.column() >= _columnCount)
        return false;

    switch (index.column()) {
    case Enabled:
        if (value.toBool())
            m_enabled.insert(index.row());
        else
            m_enabled.remove(index.row());

        emit dataChanged(index, index);
        return true;
    }

    return false;
}

int PluginsModel::rowCount(const QModelIndex &) const
{
    return m_plugins.count();
}

int PluginsModel::columnCount(const QModelIndex &) const
{
    return _columnCount;
}

QVariant PluginsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   if (role != Qt::DisplayRole)
       return QVariant();

   if (orientation == Qt::Horizontal) {
       switch (section) {
       case Name:        return tr("Name");
       case Description: return tr("Description");
       case Version:     return tr("Version");
       default:          return "";
       }
   } else {
       return section + 1;
   }
}

Qt::ItemFlags PluginsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() > rowCount() - 1 || index.column() >= _columnCount)
        return Qt::NoItemFlags;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (index.column() == Enabled) {
        if (!m_plugins[index.row()]->loader)
            flags ^= Qt::ItemIsEnabled;
        else
            flags |= Qt::ItemIsEditable;
    }

    return flags;
}

void PluginsModel::add(const PluginRegistry::PluginData *plugin, bool enabled)
{
    emit beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_plugins << plugin;
    if (enabled)
        m_enabled << m_plugins.count() - 1;
    emit endInsertRows();
}
