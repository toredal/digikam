/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : threaded image filter class.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "dimgthreadedfilter.moc"

// Qt includes

#include <QObject>

// KDE includes

#include <kdebug.h>

namespace Digikam
{

DImgThreadedFilter::DImgThreadedFilter(QObject* parent)
                  : DynamicThread()
{
    setOriginalImage(DImg());
    setFilterName(QString());
    setParent(parent);

    m_master          = 0;
    m_slave           = 0;
    m_progressBegin   = 0;
    m_progressSpan    = 100;
    m_progressCurrent = 0;
}

DImgThreadedFilter::DImgThreadedFilter(DImg* orgImage, QObject* parent,
                                       const QString& name)
                  : DynamicThread()
{
    // remove meta data
    setOriginalImage(orgImage->copyImageData());
    setFilterName(name);
    setParent(parent);

    m_master          = 0;
    m_slave           = 0;
    m_progressBegin   = 0;
    m_progressSpan    = 100;
    m_progressCurrent = 0;
}

DImgThreadedFilter::DImgThreadedFilter(DImgThreadedFilter* master, const DImg& orgImage,
                                       const DImg& destImage, int progressBegin, int progressEnd,
                                       const QString& name)
{
    setOriginalImage(orgImage);
    setFilterName(name);
    setParent(0);

    m_destImage       = destImage;
    m_master          = master;
    m_slave           = 0;
    m_progressBegin   = progressBegin;
    m_progressSpan    = progressEnd - progressBegin;
    m_progressCurrent = 0;
    
    m_master->setSlave(this);
}

DImgThreadedFilter::~DImgThreadedFilter()
{
    cancelFilter();
    if (m_master)
        m_master->setSlave(0);
}

void DImgThreadedFilter::setOriginalImage(const DImg& orgImage)
{
    m_orgImage = orgImage;
}

void DImgThreadedFilter::setFilterName(const QString& name)
{
    m_name = QString(name);
}

void DImgThreadedFilter::setParent(QObject* parent)
{
    m_parent = parent;
}

void DImgThreadedFilter::initFilter()
{
    m_destImage.reset();
    m_destImage = DImg(m_orgImage.width(), m_orgImage.height(),
                       m_orgImage.sixteenBit(), m_orgImage.hasAlpha());

    if (m_master)
        startFilterDirectly();
}

void DImgThreadedFilter::startFilter()
{
    if (m_orgImage.width() && m_orgImage.height())
    {
        start();
    }
    else  // No image data
    {
        emit finished(false);
        kDebug() << m_name << "::No valid image data !!! ...";
    }
}

void DImgThreadedFilter::startFilterDirectly()
{
    if (m_orgImage.width() && m_orgImage.height())
    {
        emit started();

        m_wasCancelled = false;
        try
        {
            filterImage();
        }
        catch (std::bad_alloc &ex)
        {
            //TODO: User notification
            kError() << "Caught out-of-memory exception! Aborting operation" << ex.what();
            emit finished(false);
            return;
        }

        emit finished(!m_wasCancelled);
    }
    else  // No image data
    {
        emit finished(false);
        kDebug() << m_name << "::No valid image data !!! ...";
    }
}

void DImgThreadedFilter::run()
{
    startFilterDirectly();
}

void DImgThreadedFilter::cancelFilter()
{
    if (isRunning())
        m_wasCancelled = true;
    stop();
    if (m_slave)
    {
        m_slave->stop();
        // do not wait on slave, it is not running in its own separate thread!
        //m_slave->cleanupFilter();
    }
    wait();
    cleanupFilter();
}

void DImgThreadedFilter::postProgress(int progr)
{
    if (m_master)
    {
        progr = modulateProgress(progr);
        m_master->postProgress(progr);
    }
    else if (m_progressCurrent != progr)
    {
        emit progress(progr);
        m_progressCurrent = progr;
    }
}

void DImgThreadedFilter::setSlave(DImgThreadedFilter* slave)
{
    m_slave = slave;
}

int DImgThreadedFilter::modulateProgress(int progress)
{
    return m_progressBegin + (int)((double)progress * (double)m_progressSpan / 100.0);
}

}  // namespace Digikam
