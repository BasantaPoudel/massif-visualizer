/*
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "datatreemodel.h"

#include <QtCore/QDebug>

#include "massifdata/filedata.h"
#include "massifdata/snapshotitem.h"
#include "massifdata/treeleafitem.h"

using namespace Massif;

DataTreeModel::DataTreeModel(QObject* parent)
    : QAbstractItemModel(parent), m_data(0)
{
}

DataTreeModel::~DataTreeModel()
{
}

void DataTreeModel::setSource(const FileData* data)
{
    if (m_data) {
        beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
        m_data = 0;
        m_nodeToRow.clear();
        m_heapRootToSnapshot.clear();
        endRemoveRows();
    }
    if (data) {
        beginInsertRows(QModelIndex(), 0, data->snapshots().size() - 1);
        m_data = data;
        int row = 0;
        foreach(SnapshotItem* item, m_data->snapshots()) {
            if (item->heapTree()) {
                mapNodeToRow(item->heapTree(), row++);
                m_heapRootToSnapshot[item->heapTree()] = item;
            } else {
                row++;
            }
        }
        endInsertRows();
    }
}

void DataTreeModel::mapNodeToRow(TreeLeafItem* node, const int row)
{
    m_nodeToRow[node] = row;
    int r = 0;
    foreach(TreeLeafItem* child, node->children()) {
        mapNodeToRow(child, r++);
    }
}

QVariant DataTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return "Cost";
        } else {
            return "Location";
        }
    }

    return QVariant();
}

QVariant DataTreeModel::data(const QModelIndex& index, int role) const
{
    // FIXME kdchart queries (-1, -1) for empty models
    if ( index.row() == -1 || index.column() == -1 ) {
//         qWarning() << "DataTreeModel::data: FIXME fix kdchart views to not query model data for invalid indices!";
        return QVariant();
    }

    Q_ASSERT(index.row() >= 0 && index.row() < rowCount(index.parent()));
    Q_ASSERT(index.column() >= 0 && index.column() < columnCount(index.parent()));
    Q_ASSERT(m_data);

    if ( role != Qt::DisplayRole ) {
        return QVariant();
    }
    if (!index.parent().isValid()) {
        SnapshotItem* snapshot = m_data->snapshots()[index.row()];
        if (index.column() == 0) {
            return snapshot->memHeap();
        } else {
            return QString("snapshot #%1").arg(snapshot->number());
        }
    } else {
        Q_ASSERT(index.internalPointer());
        TreeLeafItem* item = static_cast<TreeLeafItem*>(index.internalPointer());
        if (index.column() == 0) {
            return item->cost();
        } else {
            return item->label();
        }
    }
    return QVariant();
}

int DataTreeModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        // heap tree item: Cost | Location
        return 2;
    } else {
        // snapshot: Cost | Time
        // TODO: see whether we can have more here
        return 2;
    }
}

int DataTreeModel::rowCount(const QModelIndex& parent) const
{
    if (!m_data || (parent.isValid() && parent.column() != 0)) {
        return 0;
    }

    if (parent.isValid()) {
        if (!parent.internalPointer()) {
            // snapshot without detailed heaptree
            return 0;
        }
        return static_cast<TreeLeafItem*>(parent.internalPointer())->children().size();
    } else {
        return m_data->snapshots().size();
    }
}

QModelIndex DataTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (row < 0 || row >= rowCount(parent) || column < 0 || column >= columnCount(parent)) {
        // invalid
        return QModelIndex();
    }

    if (parent.isValid()) {
        if (parent.column() == 0) {
            Q_ASSERT(parent.internalPointer());
            // parent is a tree leaf item
            return createIndex(row, column, static_cast<void*>(static_cast<TreeLeafItem*>(parent.internalPointer())->children()[row]));
        } else {
            return QModelIndex();
        }
    } else {
        return createIndex(row, column, static_cast<void*>(m_data->snapshots()[row]->heapTree()));
    }
}

QModelIndex DataTreeModel::parent(const QModelIndex& child) const
{
    if (child.internalPointer()) {
        TreeLeafItem* item = static_cast<TreeLeafItem*>(child.internalPointer());
        if (item->parent()) {
            if (!item->parent()->parent()) {
                // top-level
            }
            Q_ASSERT(m_nodeToRow.contains(item));
            // somewhere in the detailed heap tree
            return createIndex(m_nodeToRow[item->parent()], 0, static_cast<void*>(item->parent()));
        } else {
            // snapshot item with heap tree
            return QModelIndex();
        }
    } else {
        // snapshot item without detailed heap tree
        return QModelIndex();
    }
}
