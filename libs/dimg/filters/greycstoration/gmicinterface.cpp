/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-20-06
 * Description : Gmic interface for digikam.
 *
 * Copyright (C) 2014 by Veaceslav Munteanu<veaceslav dot munteanu90 at gmail dot com>
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

#include "gmicinterface.h"
#include <QString>
#include <kdebug.h>
#include "gmic.h"

using namespace cimg_library;
namespace Digikam
{

class GMicInterface::GMicInterfacePriv
{
public:
    GMicInterfacePriv()
    {
        p_cancel = 0;
        p_progress = 0.0f;
    }
    gmic_list<> images;
    QString command;
    int    p_cancel;
    float  p_progress;
};

GMicInterface::GMicInterface() : QObject(), d(new GMicInterfacePriv())
{

}

GMicInterface::~GMicInterface()
{
    delete d;
}

void GMicInterface::addImg(CImg<> image)
{
    d->images.assign(image);
}

void GMicInterface::addImg(CImg<> image, CImg<> mask)
{
   d->images.assign(image, mask, false);
}

void GMicInterface::setParallelCommand(QString command)
{
    d->command = QString();
    d->command.append(QString("-apply_parallel_overlap \""));
    d->command.append(command);
    d->command.append(QString("\""));
}

void GMicInterface::setCommand(QString command)
{
    d->command = QString();
    d->command.append(command);
}

void GMicInterface::runGmic()
{
    kDebug() << "Command++++ " << d->command;

    try{
        gmic_list<char> image_name;
        char* extended_args = 0;
        gmic(d->command.toAscii().data(), d->images, image_name, extended_args, true, &(d->p_progress), &(d->p_cancel));

        kDebug() << "Progress" << d->p_progress;
    }
    catch (gmic_exception& e)
    {
        kDebug() << "Error encountered when calling G'MIC: " << e.what();
        emit signalResultReady(false);
        return;
    }
    emit signalResultReady(d->p_cancel == 0);
}

CImg< > GMicInterface::getImg()
{
    return d->images[0];
}

void GMicInterface::cancel()
{
    kDebug() << "Cancel+++++";
    d->p_cancel = 1;
}

float GMicInterface::getProgress()
{
    return d->p_progress;
}

} // namepsace Digikam

