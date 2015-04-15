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

#include "mainwindow.h"

#include <QDebug>
#include <QFile>
#include <qtconcurrentrun.h>

#include <KActionCollection>
#include <KStandardAction>

using namespace QtConcurrent;

sd_journal *MainWindow::jrnlUpdate = NULL;

MainWindow::MainWindow() : KXmlGuiWindow(0)
{
  ui.setupUi(this);

  // Setup statusbar
  lblStats = new QLabel(this);
  this->statusBar()->addPermanentWidget(lblStats, 20);

  lblProcess = new QLabel(this);
  this->statusBar()->addPermanentWidget(lblProcess, 20);
  lblProcess->setText("Idle");

  prgBar = new QProgressBar(this);
  this->statusBar()->addPermanentWidget(prgBar, 20);
  prgBar->setMaximumWidth(100);
  prgBar->setMaximum(100);

  ui.cmbPriority->setEditText("Priorities");

  // setup metaenum for priority combobox
  int index = metaObject()->indexOfEnumerator("prio");
  metaEnum = metaObject()->enumerator(index);
  populatePrio();

  // Setup models
  jrnlModel = new JournalModel(this, &listEntries);
  sortFilterModel = new SortFilterModel(this, &listEntries);
  sortFilterModel->setDynamicSortFilter(false);
  sortFilterModel->setSourceModel(jrnlModel);
  ui.tblLog->setModel(sortFilterModel);
  ui.tblLog->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);

  // Find current boot
  currentBootID = getCurrentBootID();
  currentFilter << QString("_BOOT_ID=" + currentBootID);

  // Read journal entries in another thread
  qRegisterMetaType<jrnlEntry>("jrnlEntry");
  timePopModel = new QElapsedTimer;
  timePopModel->start();
  watchReadJournal = new QFutureWatcher<QList<jrnlEntry> >;
  prgBar->setMaximum(0);
  connect(watchReadJournal, SIGNAL(finished()), this, SLOT(readJournalFinished()));
  char * ptr = NULL;
  futureReadJournal = QtConcurrent::run(this, &MainWindow::readJournal, currentFilter, ptr);
  watchReadJournal->setFuture(futureReadJournal);

  // Get all bootIDs in another thread
  QFutureWatcher<void> *watchReadBootIDs = new QFutureWatcher<void>();
  connect(watchReadBootIDs, SIGNAL(finished()), this, SLOT(getBootIDsFinished()));
  QFuture<void> futureReadBootIDs = QtConcurrent::run(this, &MainWindow::getBootIDs);
  watchReadBootIDs->setFuture(futureReadBootIDs);

  // Connect signals and slots
  connect(ui.btnLoad, SIGNAL(clicked()), this, SLOT(btnLoadClicked()));
  connect(prioModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(cmbPriorityChanged(QStandardItem*)));
  connect(ui.leSearch, SIGNAL(textEdited(QString)), this, SLOT(leSearchChanged(QString)));

  // Setup a file descriptor to poll for journal changes
  int r;
  r = sd_journal_open(&jrnlUpdate, jflags);
  r = sd_journal_get_fd(jrnlUpdate);
  jrnlFd = new QSocketNotifier(r, QSocketNotifier::Read);
  jrnlFd->setEnabled(true);
  connect(jrnlFd, SIGNAL(activated(int)), this, SLOT(journalChanged(int)));

  setupGUI(Default, "journalktlui.rc");
}

QList<jrnlEntry> MainWindow::readJournal(const QStringList &filter, char *cursor)
{
  // Reads journal entries with filter and from cursor

  QList<jrnlEntry> list;
  int r;
  sd_journal *journal;
  r = sd_journal_open(&journal, jflags);
  if (r < 0)
  {
    qDebug() << "Failed to open journal";
    return QList<jrnlEntry>();
    //return -1;
  }

  // set filter
  for (int i = 0; i < filter.size(); ++i)
  {
    // qDebug() << "filter: " << filter;
    r = sd_journal_add_match(journal, filter.at(i).toLatin1(), 0);
    if (r < 0)
    {
      qDebug() << "Failed to set filter";
      return QList<jrnlEntry>();
      //return -1;
    }
  }

  // seek to cursor
  if (cursor)
  {
    // qDebug() << "seeking cursor: " << cursor;
    r = sd_journal_seek_cursor(journal, cursor);
    if (r < 0)
    {
      qDebug() << "Failed to seek cursor:" << r;
      return QList<jrnlEntry>();
      //return -1;
    }
    r = sd_journal_next(journal);

    //r = sd_journal_test_cursor(journal, cursor);
    if (r < 1)
    {
      qDebug() << "Failed to test cursor:" << r;
      return QList<jrnlEntry>();
      //return -1;
    }
  }

  // iterate over filtered entries
  forever
  {
    jrnlEntry entry;

    r = sd_journal_next(journal);
    if (r < 0) {
      qDebug() << "Failed to iterate to next entry";
      break;
    }
    if (r == 0)
    {
      // Reached the end
      break;
    }
    entry = readJournalEntry(journal);
    // qDebug() << entry.msg;

    list.append(entry);
  }

  r = sd_journal_get_cursor(journal, &lastCursor);
  sd_journal_close(journal);
  if (list.size() > 0)
    qDebug() << "Read" << list.size() << "journal entries.";
  return list;
}

jrnlEntry MainWindow::readJournalEntry(sd_journal *journal) const
{
  // Reads a single journal entry

  jrnlEntry entry;
  const void *data;
  size_t length;
  uint64_t time;
  int r;

  r = sd_journal_get_realtime_usec(journal, &time);
  if (r == 0)
    entry.date.setMSecsSinceEpoch(time/1000);

  r = sd_journal_get_data(journal, "SYSLOG_IDENTIFIER", &data, &length);
  if (r == 0)
  {
    entry.unit = QString::fromLatin1((const char *)data, length).section("=",1);
  }
  else
  {
    r = sd_journal_get_data(journal, "_SYSTEMD_UNIT", &data, &length);
    if (r == 0)
      entry.unit = QString::fromLatin1((const char *)data, length).section("=",1);
  }
  //qDebug() << QString::fromLatin1((const char *)data, length).section("=",1);

  r = sd_journal_get_data(journal, "MESSAGE", &data, &length);
  if (r == 0)
    entry.msg = QString::fromLatin1((const char *)data, length).section("=",1);

  r = sd_journal_get_data(journal, "PRIORITY", &data, &length);
  if (r == 0)
  {
    //entry.priority = metaEnum.valueToKey(QString::fromLatin1((const char *)data, length).section("=",1).toInt());
    entry.priority = QString::fromLatin1((const char *)data, length).section("=",1).toInt();
  }

  r = sd_journal_get_data(journal, "_BOOT_ID", &data, &length);
  if (r == 0)
    entry.bootID = QString::fromLatin1((const char *)data, length).section("=",1);

  return entry;
}

void MainWindow::populatePrio()
{
  // Populates the priority combobox from an enum

  prioModel = new QStandardItemModel();
  ui.cmbPriority->setModel(prioModel);

  QStandardItem *item = new QStandardItem(i18n("Select priorities"));
  item->setSelectable(false);
  prioModel->appendRow(item);


  for (int i = metaEnum.keyCount()-1; i >= 0 ; i--)
  {
    QStandardItem* item;
    item = new QStandardItem(metaEnum.key(i));

    item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    item->setData(Qt::Checked, Qt::CheckStateRole);
    item->setData(metaEnum.value(i), Qt::UserRole);

    QColor newcolor;
    if (metaEnum.value(i) == emerg ||
        metaEnum.value(i) == alert ||
        metaEnum.value(i) == crit)
      newcolor = Qt::red;
    else if (metaEnum.value(i) == err)
      newcolor = Qt::darkRed;
    else if (metaEnum.value(i) == warn)
      newcolor = Qt::darkYellow;
    else if (metaEnum.value(i) == notice ||
             metaEnum.value(i) == info)
      newcolor = Qt::black;
    else if (metaEnum.value(i) == debug)
      newcolor = Qt::darkGray;
    item->setData(QVariant(newcolor), Qt::ForegroundRole);

    prioModel->appendRow(item);
  }
}

QString MainWindow::getCurrentBootID()
{
  // Gets the most recent bootID. This is used for
  // building the initial model.

  int r;
  sd_journal *journal;
  r = sd_journal_open(&journal, jflags);
  if (r == 0)
  {
    const void *data;
    size_t length;
    r = sd_journal_seek_tail(journal);
    if (r < 0)
    {
      qDebug() << "Failed to seek tail";
      return QString();
    }
    r = sd_journal_previous(journal);
    if (r < 1)
    {
      qDebug() << "Failed to go to previous";
      return QString();
    }
    r = sd_journal_get_data(journal, "_BOOT_ID", &data, &length);
    QString curBoot = QString::fromLatin1((const char *)data, length).section("=",1);
    sd_journal_close(journal);
    //qDebug() << "Current boot ID is: " << curBoot;
    if (r == 0)
      return curBoot;
    else
      return QString();
  }
  qDebug() << "Failed to open journal!";
  return QString();
}

void MainWindow::getBootIDs()
{
  // Fetches a list of all bootIDs in the journal.
  // This is used to build the model for bootID combobox.
  // Runs on a different thread.

  int count = 0;
  int r;
  sd_journal *journal;
  r = sd_journal_open(&journal, jflags);
  if (r == 0)
  {  
    const void *data;
    size_t length;
    r = sd_journal_query_unique(journal, "_BOOT_ID");
    SD_JOURNAL_FOREACH_UNIQUE(journal, data, length)
    {
      // qDebug() << "Found: " << QString::fromLatin1((const char *)data, length).section("=",1);
      jrnlBootIDs.append(QString::fromLatin1((const char *)data, length).section("=",1));
      count++;
    }
    sd_journal_close(journal);
  }
  else
    qDebug() << "Failed to open journal!";
}

void MainWindow::populateBootIDs()
{
  // Populates the bootID combobox by iterating through jrnlBootIDs.
  // This slot gets called after getBootIDs() is done.

  bootModel = new QStandardItemModel(jrnlBootIDs.size(), 1);
  bootSortModel = new QSortFilterProxyModel(this);
  bootSortModel->setSourceModel(bootModel);
  ui.cmbBootIDs->setModel(bootSortModel);

  int r;
  QString itemText;
  QDateTime date;
  sd_journal *journal;
  r = sd_journal_open(&journal, jflags);

  // We iterate over the boot IDs:
  for (int i = 0; i < jrnlBootIDs.size(); ++i)
  {
    // Flush previous journal filters
    sd_journal_flush_matches(journal);

    //qDebug() << "flushed";

    // Filter the bootID
    r = sd_journal_add_match(journal, QString("_BOOT_ID=" + jrnlBootIDs.at(i)).toLatin1(), 0);
    if (r < 0)
      qDebug() << "Failed to set filter";

    // Find the oldest entry within this bootID
    r = sd_journal_seek_head(journal);
    if (r < 0)
      qDebug() << "Failed to seek head";
    r = sd_journal_next(journal);
    if (r < 1)
      qDebug() << "Failed to go to next entry";

    // Get the date for this entry
    uint64_t time;
    r = sd_journal_get_realtime_usec(journal, &time);
    if (r == 0)
    {
      date.setMSecsSinceEpoch(time/1000);
      // itemText = QString(QString::number(i+1) + ": " + date.toString("yyyy.MM.dd"));
      itemText = QString(date.toString("yyyy.MM.dd"));
    }

    QStandardItem* item;
    item = new QStandardItem(itemText);
    // Set the UserRole to the bootID
    item->setData(jrnlBootIDs.at(i), Qt::UserRole);
    item->setData(date.toString("yyyy.MM.dd hh:mm:ss"), Qt::UserRole+1);
    item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);

    // Select only the current bootID
    // if (i == jrnlBootIDs.size()-1)
    if (jrnlBootIDs[i] == currentBootID)
      item->setData(Qt::Checked, Qt::CheckStateRole);
    else
      item->setData(Qt::Unchecked, Qt::CheckStateRole);

    // Add the item to the model (in reverse order):
    bootModel->setItem(jrnlBootIDs.size()-1-i, 0, item);

  }
  sd_journal_close(journal);

  bootSortModel->setSortRole(Qt::UserRole+1);
  bootSortModel->sort(0, Qt::DescendingOrder);
  int j = 0;
  for (int i = bootSortModel->rowCount() - 1; i >= 0; i--)
  {
    // qDebug() << "loop: " << i << "( " << bootSortModel->data(bootSortModel->index(j, 0)).toString() << ")";
    QString txt = QString(bootSortModel->data(bootSortModel->index(j, 0)).toString() + " (" + QString::number(i+1) + ")");
    bootSortModel->setData(bootSortModel->index(j, 0), txt);
    j++;
  }
  ui.cmbBootIDs->setCurrentIndex(0);
}

void MainWindow::cmbPriorityChanged(QStandardItem* item)
{
  // This slot gets called whenever the priority filter is changed
  prgBar->setMaximum(0);

  if (item->checkState() == Qt::Checked)
    sortFilterModel->filterPrio.append(item->data(Qt::UserRole).toInt());
  else
    sortFilterModel->filterPrio.removeAll(item->data(Qt::UserRole).toInt());

  QElapsedTimer *timer = new QElapsedTimer;
  timer->start();
  ui.tblLog->setUpdatesEnabled(false);
  sortFilterModel->invalidate();
  ui.tblLog->setUpdatesEnabled(true);
  qDebug() << "invalidate after prioFilter" << timer->elapsed() << "ms";

  ui.tblLog->sortByColumn(ui.tblLog->horizontalHeader()->sortIndicatorSection(), ui.tblLog->horizontalHeader()->sortIndicatorOrder());
  updateStats();
  prgBar->setMaximum(100);
}

void MainWindow::btnLoadClicked()
{
  // Slot used to update the model with selected bootIDs
  currentFilter.clear();

  // Create a filter containing the selected bootIDs
  for(int i = 0; i < ui.cmbBootIDs->count(); i++)
  {
    QModelIndex index = ui.cmbBootIDs->model()->index(i, 0);
    if(index.data(Qt::CheckStateRole).toInt() == Qt::Checked)
      currentFilter << QString("_BOOT_ID=" + index.data(Qt::UserRole).toString());
  }

  // Clear the model
  jrnlModel->beginRemoveRows(QModelIndex(), 0, listEntries.size()-1);
  listEntries.clear();
  jrnlModel->endRemoveRows();


  lblStats->setText("");
  lblProcess->setText("Reading entries...");
  prgBar->setMaximum(0);
  ui.tblLog->setEnabled(false);
  ui.btnLoad->setEnabled(false);

  // Read journal entries in another thread
  timePopModel = new QElapsedTimer;
  timePopModel->start();
  watchReadJournal = new QFutureWatcher<QList<jrnlEntry> >();
  connect(watchReadJournal, SIGNAL(finished()), this, SLOT(readJournalFinished()));
  char * ptr = NULL;
  futureReadJournal = QtConcurrent::run(this, &MainWindow::readJournal, currentFilter, ptr);
  watchReadJournal->setFuture(futureReadJournal);
}

void MainWindow::leSearchChanged(QString searchTxt)
{
  // Gets called whenever the searchbox is modified

  sortFilterModel->filterMsgOrUnit = searchTxt;

  QElapsedTimer *timer = new QElapsedTimer;
  timer->start();
  sortFilterModel->invalidate();
  qDebug() << "invalidate after unit/msg filter" << timer->elapsed() << "ms";

  updateStats();
}

void MainWindow::getBootIDsFinished()
{
  // Gets called when reading bootIDs is done

  //qDebug() << "Finished getting boot IDs.";
  populateBootIDs();
  ui.cmbBootIDs->setEnabled(true);
}

void MainWindow::readJournalFinished()
{
  // Gets called when journal entries have been read

  //qDebug() << "listEntries is before:" << listEntries.size();
  if (futureReadJournal.result().size() > 0)
  {
    ui.tblLog->setUpdatesEnabled(false);
    jrnlModel->beginInsertRows(QModelIndex(), listEntries.size(), listEntries.size()+futureReadJournal.result().size()-1);
    listEntries = futureReadJournal.result();
    jrnlModel->endInsertRows();
    ui.tblLog->setUpdatesEnabled(true);
  }
  //qDebug() << "listEntries is after:" << listEntries.size();

  qDebug() << "Done getting entries (" << listEntries.size() << "entries, time:" << timePopModel->elapsed() << "ms)";

  ui.tblLog->sortByColumn(0, Qt::DescendingOrder);
  ui.tblLog->setEnabled(true);

  QElapsedTimer *timer = new QElapsedTimer;
  timer->start();
  sortFilterModel->invalidate();
  qDebug() << "invalidate after readJournal" << timer->elapsed() << "ms";

  ui.tblLog->resizeColumnsToContents();

  updateStats();
  lblProcess->setText("Idle");
  prgBar->setMaximum(100);
  ui.btnLoad->setEnabled(true);
}

void MainWindow::readJournalNewFinished()
{
  // Gets called when journal has changed and the new entries
  // have been read.

  if (futureReadJournal.result().size() > 0)
  {
    jrnlModel->beginInsertRows(QModelIndex(), listEntries.size(), listEntries.size()+futureReadJournal.result().size()-1);
    listEntries.append(futureReadJournal.result());
    jrnlModel->endInsertRows();
  }

  sortFilterModel->invalidate();

  lblProcess->setText("Idle");
  prgBar->setMaximum(100);
  updateStats();
}

void MainWindow::updateStats()
{
  // Updates the stats in the statusbar
  lblStats->setText(QString::number(jrnlModel->rowCount()) + " entries, " + QString::number(sortFilterModel->rowCount()) + " filtered");
}

void MainWindow::journalChanged(int i)
{
  // Gets called whenever the journal is changed

  QFile file("test");
  file.open(i, QIODevice::ReadOnly);
  file.readAll();
  file.close();

  // Read new entries in another thread
  if (!watchReadJournal->isRunning())
  {
    // Read journal entries in another thread
    timePopModel = new QElapsedTimer;
    timePopModel->start();
    watchReadJournal = new QFutureWatcher<QList<jrnlEntry> >;
    connect(watchReadJournal, SIGNAL(finished()), this, SLOT(readJournalNewFinished()));
    futureReadJournal = QtConcurrent::run(this, &MainWindow::readJournal, currentFilter, lastCursor);
    watchReadJournal->setFuture(futureReadJournal);
  }
}

MainWindow::~MainWindow()
{
  jrnlFd->setEnabled(false);
  free(lastCursor);
  sd_journal_close(jrnlUpdate);
}
