/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-28
 * Description : Table view column helpers: Digikam properties
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

#include "tableview_column_digikam.moc"

// Qt includes

#include <QFormLayout>
#include <QModelIndex>

// KDE includes

#include <kdebug.h>
#include <klocale.h>

// local includes

#include "globals.h"
#include "imageinfo.h"

namespace Digikam
{

namespace TableViewColumns
{

ColumnDigikamProperties::ColumnDigikamProperties(
        TableViewShared* const tableViewShared,
        const TableViewColumnConfiguration& pConfiguration,
        QObject* const parent)
  : TableViewColumn(tableViewShared, pConfiguration, parent),
    subColumn(SubColumnRating)
{
    const QString& subColumnSetting = configuration.getSetting("subcolumn");
    subColumn = getSubColumnIndex<ColumnDigikamProperties>(subColumnSetting, SubColumnRating);
}

ColumnDigikamProperties::~ColumnDigikamProperties()
{

}

QStringList ColumnDigikamProperties::getSubColumns()
{
    QStringList columns;
    columns << QLatin1String("rating") << QLatin1String("picklabel")
            << QLatin1String("colorlabel") << QLatin1String("title")
            << QLatin1String("caption");

    return columns;
}

TableViewColumnDescription ColumnDigikamProperties::getDescription()
{
    TableViewColumnDescription description(QLatin1String("digikam-properties"), i18n("digiKam properties"));
    description.setIcon("imagecomment");

    description.addSubColumn(
        TableViewColumnDescription("digikam-properties", i18n("Rating"), "subcolumn", "rating")
            .setIcon("draw-star")
    );

    description.addSubColumn(
        TableViewColumnDescription("digikam-properties", i18n("Pick label"), "subcolumn", "picklabel")
            .setIcon("flag-red")
    );

    description.addSubColumn(
        TableViewColumnDescription("digikam-properties", i18n("Color label"), "subcolumn", "colorlabel")
    );

    /// @todo This column will show the 'default' title. Add a configuration dialog to choose different languages.
    description.addSubColumn(
        TableViewColumnDescription("digikam-properties", i18n("Title"), "subcolumn", "title")
    );

    /// @todo This column will show the 'default' caption. Add a configuration dialog to choose different languages.
    description.addSubColumn(
        TableViewColumnDescription("digikam-properties", i18n("Caption"), "subcolumn", "caption")
    );

    return description;
}

QString ColumnDigikamProperties::getTitle() const
{
    switch (subColumn)
    {
    case SubColumnRating:
        return i18n("Rating");
    case SubColumnPickLabel:
        return i18n("Pick label");
    case SubColumnColorLabel:
        return i18n("Color label");
    case SubColumnTitle:
        return i18n("Title");
    case SubColumnCaption:
        return i18n("Caption");
    }

    return QString();
}

TableViewColumn::ColumnFlags ColumnDigikamProperties::getColumnFlags() const
{
    ColumnFlags flags(ColumnNoFlags);

    if (  (subColumn == SubColumnRating)
       || (subColumn == SubColumnPickLabel)
       || (subColumn == SubColumnColorLabel) )
    {
        flags|=ColumnCustomSorting;
    }

    return flags;
}

QVariant ColumnDigikamProperties::data(const QModelIndex& sourceIndex, const int role) const
{
    if ( (role != Qt::DisplayRole) &&
         (role != Qt::TextAlignmentRole) &&
         (role != Qt::ForegroundRole ) )
    {
        return sourceIndex.data(role);
    }

    if (role==Qt::TextAlignmentRole)
    {
        switch (subColumn)
        {
        case SubColumnRating:
            return QVariant(Qt::Alignment(Qt::AlignRight | Qt::AlignVCenter));

        default:
            return sourceIndex.data(role);
        }
    }

    if (role==Qt::ForegroundRole)
    {
        switch (subColumn)
        {
        case SubColumnPickLabel:
            {
                const ImageInfo info = getImageInfo(sourceIndex);
                const PickLabel pickLabel = PickLabel(info.pickLabel());

                QColor labelColor;
                switch (pickLabel)
                {
                    case NoPickLabel:
                        labelColor = Qt::darkGray;
                        break;

                    case RejectedLabel:
                        labelColor = Qt::red;
                        break;

                    case PendingLabel:
                        // yellow is too hard to read
                        labelColor = Qt::darkYellow;
                        break;

                    case AcceptedLabel:
                        // green is too hard to read
                        labelColor = Qt::darkGreen;
                        break;

                    default:
                        break;
                }

                QBrush labelBrush = sourceIndex.data(role).value<QBrush>();
                labelBrush.setColor(labelColor);

                return QVariant::fromValue(labelBrush);
            }

        case SubColumnColorLabel:
            {
                const ImageInfo info = getImageInfo(sourceIndex);
                const ColorLabel colorLabel = ColorLabel(info.colorLabel());

                QColor labelColor;
                switch (colorLabel)
                {
                    case NoColorLabel:
                        labelColor = Qt::lightGray;
                        break;

                    case RedLabel:
                        labelColor = Qt::red;
                        break;

                    case OrangeLabel:
                        labelColor = QColor(0xff, 0x80, 0x00);
                        break;

                    case YellowLabel:
                        labelColor = Qt::darkYellow;
                        break;

                    case GreenLabel:
                        labelColor = Qt::darkGreen;
                        break;

                    case BlueLabel:
                        labelColor = Qt::darkBlue;
                        break;

                    case MagentaLabel:
                        labelColor = Qt::magenta;
                        break;

                    case GrayLabel:
                        labelColor = Qt::darkGray;
                        break;

                    case BlackLabel:
                        labelColor = Qt::black;
                        break;

                    case WhiteLabel:
                        labelColor = Qt::white;
                        break;

                    default:
                        break;
                }

                QBrush labelBrush = sourceIndex.data(role).value<QBrush>();
                labelBrush.setColor(labelColor);

                return QVariant::fromValue(labelBrush);
            }
        default:
            return sourceIndex.data(role);
        }
    }

    const ImageInfo info = getImageInfo(sourceIndex);

    /// @todo Also display the pick label icon?
    /// @todo Make display of text/icon configurable.
    switch (subColumn)
    {
    case SubColumnRating:
        {
            const int itemRating = info.rating();
            if (itemRating<=0)
            {
                // no rating
                return QString();
            }

            return KGlobal::locale()->formatNumber(itemRating, 0);
        }

    case SubColumnPickLabel:
        {
            const PickLabel pickLabel = PickLabel(info.pickLabel());

            QString labelString;
            switch (pickLabel)
            {
                case NoPickLabel:
                    labelString = i18n("None");
                    break;

                case RejectedLabel:
                    labelString = i18n("Rejected");
                    break;

                case PendingLabel:
                    labelString = i18n("Pending");
                    break;

                case AcceptedLabel:
                    labelString = i18n("Accepted");
                    break;

                default:
                    break;
            }

            return labelString;
        }

    case SubColumnColorLabel:
        {
            const ColorLabel colorLabel = ColorLabel(info.colorLabel());

            QString labelString;
            switch (colorLabel)
            {
                case NoColorLabel:
                    labelString = i18n("None");
                    break;

                case RedLabel:
                    labelString = i18n("Red");
                    break;

                case OrangeLabel:
                    labelString = i18n("Orange");
                    break;

                case YellowLabel:
                    labelString = i18n("Yellow");
                    break;

                case GreenLabel:
                    labelString = i18n("Green");
                    break;

                case BlueLabel:
                    labelString = i18n("Blue");
                    break;

                case MagentaLabel:
                    labelString = i18n("Magenta");
                    break;

                case GrayLabel:
                    labelString = i18n("Gray");
                    break;

                case BlackLabel:
                    labelString = i18n("Black");
                    break;

                case WhiteLabel:
                    labelString = i18n("White");
                    break;

                default:
                    break;
            }

            return labelString;
        }

    case SubColumnTitle:
        {
            const QString title = info.title();

            return title;
        }

    case SubColumnCaption:
        {
            const QString caption = info.comment();

            return caption;
        }
    }

    return QVariant();
}

TableViewColumn::ColumnCompareResult ColumnDigikamProperties::compare(const QModelIndex& sourceA, const QModelIndex& sourceB) const
{
    const ImageInfo infoA = getImageInfo(sourceA);
    const ImageInfo infoB = getImageInfo(sourceB);

    switch (subColumn)
    {
    case SubColumnRating:
        {
            /// @todo Handle un-rated vs rated items differently?
            const int ratingA = infoA.rating();
            const int ratingB = infoB.rating();

            return compareHelper<int>(ratingA, ratingB);
        }

    case SubColumnPickLabel:
        {
            /// @todo Handle un-rated vs rated items differently?
            const int pickLabelA = infoA.pickLabel();
            const int pickLabelB = infoB.pickLabel();

            return compareHelper<int>(pickLabelA, pickLabelB);
        }

    case SubColumnColorLabel:
        {
            /// @todo Handle un-rated vs rated items differently?
            const int colorLabelA = infoA.colorLabel();
            const int colorLabelB = infoB.colorLabel();

            return compareHelper<int>(colorLabelA, colorLabelB);
        }

    default:
        kWarning() << "item: unimplemented comparison, subColumn=" << subColumn;
        return CmpEqual;
    }
}

} /* namespace TableViewColumns */

} /* namespace Digikam */

