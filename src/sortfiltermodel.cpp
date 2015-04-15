/*******************************************************************************
 * Copyright (C) 2015 Ragnar Thomsen <rthomsen6@gmail.com>                     *
 *                                                                             *
 * This program is free software: you can redistribute it and/or modify it     *
 * under the terms of the GNU General Public License as published by the Free  *
 * Software Foundation, either version 2 of the License, or (at your option)   *
 * any later version.                                                          *
 *                                                                             *
 * This program is distributed in the hope that it will be useful, but WITHOUT *
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for    *
 * more details.                                                               *
 *                                                                             *
 * You should have received a copy of the GNU General Public License along     *
 * with this program. If not, see <http://www.gnu.org/licenses/>.              *
 *******************************************************************************/

#include "sortfiltermodel.h"

SortFilterModel::SortFilterModel(QObject *parent)
     : QSortFilterProxyModel(parent)
{
}

SortFilterModel::SortFilterModel(QObject *parent, const QList<jrnlEntry> *list)
     : QSortFilterProxyModel(parent)
{
  listEntries = list;
  filterPrio << 0 << 1 << 2 << 3 << 4 << 5 << 6 << 7;
}

bool SortFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
  if (filterPrio.contains(listEntries->at(sourceRow).priority) &&
      (sourceModel()->index(sourceRow, 1, sourceParent).data().toString().contains(filterMsgOrUnit, Qt::CaseInsensitive) ||
      sourceModel()->index(sourceRow, 2, sourceParent).data().toString().contains(filterMsgOrUnit, Qt::CaseInsensitive)))
  {
    return true;
  }

  return false;
}
