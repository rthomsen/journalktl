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

#ifndef SORTFILTERMODEL_H
#define SORTFILTERMODEL_H

#include <QSortFilterProxyModel>
#include <QRegularExpression>
#include <QDebug>

#include "journalentry.h"

class SortFilterModel : public QSortFilterProxyModel
{
  Q_OBJECT
  enum filterType
  {
   unit, message, priority
  };

public:
  SortFilterModel(QObject *parent = 0);
  SortFilterModel(QObject *parent = 0, const QList<jrnlEntry> *list = NULL);
  QString filterMsgOrUnit;
  QList<int> filterPrio;

protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
  const QList<jrnlEntry> *listEntries;
};

#endif // SORTFILTERMODEL_H
