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

#ifndef JOURNALMODEL_H
#define JOURNALMODEL_H

#include <QAbstractTableModel>
#include <QMetaEnum>

#include "journalentry.h"

class JournalModel : public QAbstractTableModel
{
  Q_OBJECT
  Q_ENUMS(prio)

  friend class MainWindow;
  
public:
  JournalModel(QObject *parent = 0);
  JournalModel(QObject *parent = 0, const QList<jrnlEntry> *list = NULL);
  int rowCount(const QModelIndex & parent = QModelIndex()) const;
  int columnCount(const QModelIndex & parent = QModelIndex()) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
  enum prio {  emerg,
               alert,
               crit,
               err,
               warn,
               notice,
               info,
               debug};

private:
  const QList<jrnlEntry> *listEntries;
  QMetaEnum metaEnum;
};
  
#endif // JOURNALMODEL_H
 
