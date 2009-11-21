/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an option to add a sequence number to the parser
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef SEQUENCENUMBEROPTION_H
#define SEQUENCENUMBEROPTION_H

// Local includes

#include "option.h"
#include "parseobjectdialog.h"

class KIntNumInput;

namespace Digikam
{

class SequenceNumberDialog : public ParseObjectDialog
{
    Q_OBJECT

public:

    SequenceNumberDialog(ParseObject* parent);
    ~SequenceNumberDialog();

    KIntNumInput* digits;
    KIntNumInput* start;
    KIntNumInput* step;
};

// --------------------------------------------------------

class SequenceNumberOption : public Option
{
    Q_OBJECT

public:

    SequenceNumberOption();
    ~SequenceNumberOption() {};

protected:

    virtual void parseOperation(const QString& parseString, ParseInformation& info, ParseResults& results);

private Q_SLOTS:

    void slotTokenTriggered(const QString& token);

};

} // namespace Digikam

#endif /* SEQUENCENUMBEROPTION_H */
