/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-18
 * Description : Sync QItemSelectionModel of ImageFilterModel and TableViewModel
 *
 * Copyright (C) 2013 by Michael G. Hansen <mike at mghansen dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "tableview_selection_model_syncer.moc"

// KDE includes

#include <kdebug.h>

// local includes

#include "imagefiltermodel.h"
#include "tableview_shared.h"
#include "tableview_model.h"

namespace Digikam
{

class TableViewSelectionModelSyncer::Private
{
public:
    Private()
      : syncing(false)
    {
    }

    bool syncing;
};

TableViewSelectionModelSyncer::TableViewSelectionModelSyncer(TableViewShared* const sharedObject, QObject* const parent)
  : QObject(parent),
    d(new Private()),
    s(sharedObject)
{
    connect(s->imageFilterSelectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotSourceCurrentChanged(QModelIndex,QModelIndex)));
    connect(s->imageFilterSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(slotSourceSelectionChanged(QItemSelection,QItemSelection)));
    connect(s->tableViewSelectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotTargetCurrentChanged(QModelIndex,QModelIndex)));
    connect(s->tableViewSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(slotTargetSelectionChanged(QItemSelection,QItemSelection)));

    doInitialSync();
}

TableViewSelectionModelSyncer::~TableViewSelectionModelSyncer()
{

}

QModelIndex TableViewSelectionModelSyncer::toSource(const QModelIndex& targetIndex) const
{
    return s->imageFilterModel->index(targetIndex.row(), 0, QModelIndex());
}

QModelIndex TableViewSelectionModelSyncer::toTarget(const QModelIndex& sourceIndex) const
{
    return s->tableViewModel->index(sourceIndex.row(), 0, QModelIndex());
}

int TableViewSelectionModelSyncer::targetModelColumnCount() const
{
    return s->tableViewModel->columnCount(QModelIndex());
}

QItemSelection TableViewSelectionModelSyncer::targetIndexToRowItemSelection(const QModelIndex targetIndex) const
{
    const int row = targetIndex.row();

    const QModelIndex topLeft = s->tableViewModel->index(row, 0, QModelIndex());
    const QModelIndex bottomRight = s->tableViewModel->index(row, targetModelColumnCount()-1, QModelIndex());
    const QItemSelection mySelection(topLeft, bottomRight);

    return mySelection;
}

void TableViewSelectionModelSyncer::doInitialSync()
{
    d->syncing = true;

    s->tableViewSelectionModel->clearSelection();

    const QItemSelection sourceSelection = s->imageFilterSelectionModel->selection();
    const QItemSelection targetSelection = itemSelectionToTarget(sourceSelection);
    s->tableViewSelectionModel->select(targetSelection, QItemSelectionModel::Select);

    const QModelIndex targetIndexCurrent = toTarget(s->imageFilterSelectionModel->currentIndex());
    s->tableViewSelectionModel->setCurrentIndex(targetIndexCurrent, QItemSelectionModel::NoUpdate);

    d->syncing = false;
}

void TableViewSelectionModelSyncer::slotSourceCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    if (d->syncing)
    {
        return;
    }
    d->syncing = true;

    // we have to select the whole row of the target index
    const QModelIndex targetIndexCurrent = toTarget(current);
    s->tableViewSelectionModel->setCurrentIndex(targetIndexCurrent, QItemSelectionModel::Select);

    d->syncing = false;
}

QItemSelection TableViewSelectionModelSyncer::itemSelectionToSource(const QItemSelection& selection) const
{
    QItemSelection sourceSelection;
    Q_FOREACH(const QItemSelectionRange& range, selection)
    {
        const int firstRow = range.top();
        const int lastRow = range.bottom();

        const QModelIndex sourceTopLeft = s->imageFilterModel->index(firstRow, 0);
        const QModelIndex sourceBottomRight = s->imageFilterModel->index(lastRow, 0);
        sourceSelection.select(sourceTopLeft, sourceBottomRight);
    }

    return sourceSelection;
}

QItemSelection TableViewSelectionModelSyncer::itemSelectionToTarget(const QItemSelection& selection) const
{
    const int targetColumnCount = targetModelColumnCount();

    QItemSelection targetSelection;
    Q_FOREACH(const QItemSelectionRange& range, selection)
    {
        const int firstRow = range.top();
        const int lastRow = range.bottom();

        const QModelIndex targetTopLeft = s->tableViewModel->index(firstRow, 0);
        const QModelIndex targetBottomRight = s->tableViewModel->index(lastRow, targetColumnCount-1);
        targetSelection.select(targetTopLeft, targetBottomRight);
    }

    return targetSelection;
}


void TableViewSelectionModelSyncer::slotSourceSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    if (d->syncing)
    {
        return;
    }
    d->syncing = true;

    const QItemSelection targetSelection = itemSelectionToTarget(selected);
    s->tableViewSelectionModel->select(targetSelection, QItemSelectionModel::Select);

    const QItemSelection targetDeselection = itemSelectionToTarget(deselected);
    s->tableViewSelectionModel->select(targetDeselection, QItemSelectionModel::Deselect);

    d->syncing = false;
}

void TableViewSelectionModelSyncer::slotTargetCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    if (d->syncing)
    {
        return;
    }
    d->syncing = true;

    const QModelIndex sourceIndexCurrent = toSource(current);
    s->imageFilterSelectionModel->setCurrentIndex(sourceIndexCurrent, QItemSelectionModel::Select);

    d->syncing = false;
}

void TableViewSelectionModelSyncer::slotTargetSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    if (d->syncing)
    {
        return;
    }
    d->syncing = true;

    const QItemSelection sourceSelection = itemSelectionToSource(selected);
    s->imageFilterSelectionModel->select(sourceSelection, QItemSelectionModel::Select);

    const QItemSelection sourceDeselection = itemSelectionToSource(deselected);
    s->imageFilterSelectionModel->select(sourceDeselection, QItemSelectionModel::Deselect);

    d->syncing = false;
}

} /* namespace Digikam */

