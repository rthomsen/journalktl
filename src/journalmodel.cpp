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

#include <QColor>
#include <QDebug>

#include <KLocalizedString>

#include <systemd/sd-journal.h>

#include "journalmodel.h"

JournalModel::JournalModel(QObject *parent)
 : QAbstractTableModel(parent)
{
}

JournalModel::JournalModel::JournalModel(QObject *parent, const QList<jrnlEntry> *list)
 : QAbstractTableModel(parent)
{
  listEntries = list;

  // setup metaenum for priority combobox
  int index = metaObject()->indexOfEnumerator("prio");
  metaEnum = metaObject()->enumerator(index);
}

int JournalModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid())
    return 0;
  return listEntries->size();
}

int JournalModel::columnCount(const QModelIndex &parent) const
{
  if (parent.isValid())
    return 0;
  return 3;
}

QVariant JournalModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 0)
    return i18n("Time");
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 1)
    return i18n("Unit");
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 2)
    return i18n("Message");
  //if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 3)
  //  return i18n("Priority");
  return QVariant();
}

QVariant JournalModel::data(const QModelIndex & index, int role) const
{

  if (!index.isValid())
  {
    qDebug() << "index invalid";
    return QVariant();
  }

  if (role == Qt::DisplayRole)
  {
    if (index.column() == 0)
      return listEntries->at(index.row()).date.toString("yyyy.MM.dd hh:mm:ss:zzz");
    else if (index.column() == 1)
      return listEntries->at(index.row()).unit;
    else if (index.column() == 2)
      return listEntries->at(index.row()).msg;
    //else if (index.column() == 3)
      //return listEntries->at(index.row()).priority;
      //return metaEnum.valueToKey(listEntries->at(index.row()).priority);

  }

  else if (role == Qt::ForegroundRole)
  {
    // Color entries according to priority.
    QColor newcolor;

    if (listEntries->at(index.row()).priority == emerg ||
        listEntries->at(index.row()).priority == alert ||
        listEntries->at(index.row()).priority == crit)
      newcolor = Qt::red;
    else if (listEntries->at(index.row()).priority == err)
      newcolor = Qt::darkRed;
    else if (listEntries->at(index.row()).priority == warn)
      newcolor = Qt::darkYellow;
    else if (listEntries->at(index.row()).priority == notice ||
             listEntries->at(index.row()).priority == info)
      newcolor = Qt::black;
    else if (listEntries->at(index.row()).priority == debug)
      newcolor = Qt::darkGray;
    else
      newcolor = Qt::black;

    return QVariant(newcolor);
  }

  return QVariant();
}
