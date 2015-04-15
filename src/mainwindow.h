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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QMetaEnum>
#include <QFutureWatcher>
#include <QProgressBar>
#include <QLabel>
#include <QElapsedTimer>
#include <QSocketNotifier>

#include <KXmlGuiWindow>

#include <systemd/sd-journal.h>

#include "journalentry.h"
#include "journalmodel.h"
#include "sortfiltermodel.h"
#include "ui_mainwindow.h"

class MainWindow : public KXmlGuiWindow
{
  Q_OBJECT
  Q_ENUMS(prio)

public:
  MainWindow();
  ~MainWindow();
  enum prio {  emerg,
               alert,
               crit,
               err,
               warn,
               notice,
               info,
               debug};
  QList<jrnlEntry> listEntries;
  char *lastCursor = NULL;

private:
  Ui::MainWindow ui;

  // Variables
  int jflags = SD_JOURNAL_LOCAL_ONLY | SD_JOURNAL_SYSTEM;
  static sd_journal *jrnlUpdate;
  QProgressBar *prgBar;
  QLabel *lblStats, *lblProcess;
  JournalModel *jrnlModel;
  SortFilterModel *sortFilterModel;
  QStandardItemModel *prioModel, *bootModel;
  QSortFilterProxyModel *bootSortModel;
  QElapsedTimer *timePopModel;
  QMetaEnum metaEnum;
  QFutureWatcher<QList<jrnlEntry> > *watchReadJournal;
  QFuture<QList<jrnlEntry> > futureReadJournal;
  QSocketNotifier *jrnlFd;
  QString currentBootID;
  QStringList jrnlBootIDs, currentFilter;

  // Functions
  void getBootIDs();
  void populateBootIDs();
  void populatePrio();
  void updateStats();
  QList<jrnlEntry> readJournal(const QStringList &filter, char *cursor = NULL);
  jrnlEntry readJournalEntry(sd_journal *journal) const;

private slots:
  void cmbPriorityChanged(QStandardItem*);
  void leSearchChanged(QString);
  void btnLoadClicked();
  void readJournalFinished();
  void readJournalNewFinished();
  void journalChanged(int);
};

#endif // MAINWINDOW_H
