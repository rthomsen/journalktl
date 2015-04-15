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

#ifndef JOURNALENTRY_H
#define JOURNALENTRY_H

#include <QDateTime>
#include <QDebug>

struct jrnlEntry
{
  QDateTime date;
  QString unit;
  QString msg;
  int priority;
  QString bootID;
};

inline QDebug operator<<(QDebug debug, const jrnlEntry& entry)
{
  debug.nospace() << "jrnlEntry("
                  << entry.date << ","
                  << entry.unit << ","
                  << entry.msg << ","
                  << entry.priority << ","
                  << entry.bootID << ")";
  return debug.space();
}

Q_DECLARE_METATYPE(jrnlEntry)
  
#endif // JOURNALENTRY_H
