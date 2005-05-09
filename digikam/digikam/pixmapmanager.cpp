/* ============================================================
 * File  : pixmapmanager.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-04-14
 * Description : 
 * 
 * Copyright 2005 by Renchi Raju

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

#include <qpixmap.h>
#include <qdir.h>
#include <qtimer.h>
#include <qimage.h>

#include <kurl.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include "thumbnailjob.h"
#include "albumiconview.h"
#include "albumiconitem.h"
#include "thumbdb.h"
#include "albumsettings.h"
#include "pixmapmanager.h"

/** @file pixmapmanager.cpp */

PixmapManager::PixmapManager(AlbumIconView* view)
{
    m_view  = view;
    m_cache = new QCache<QPixmap>(101, 211);
    m_cache->setAutoDelete(true);
    m_size  = 0;
    
    m_timer = new QTimer();
    connect(m_timer, SIGNAL(timeout()),
            SLOT(slotCompleted()));

    // -- resource for broken image thumbnail ---------------------------
    KGlobal::dirs()->addResourceType("digikam_imagebroken",
                                     KGlobal::dirs()->kde_default("data")
                                     + "digikam/data");
}

PixmapManager::~PixmapManager()
{
    delete m_timer;
    
    if (!m_thumbJob.isNull())
    {
        m_thumbJob->kill();
    }

    delete m_cache;
}

void PixmapManager::setThumbnailSize(int size)
{
    if (m_size == size)
        return;

    m_size = size;
    m_cache->clear();
    if (!m_thumbJob.isNull())
        m_thumbJob->kill();
}

QPixmap* PixmapManager::find(const KURL& url)
{
    QPixmap* pix = m_cache->find(url.path());
    if (pix)
        return pix;
    
    if (m_thumbJob.isNull())
    {
        m_thumbJob = new ThumbnailJob(url, m_size, true);
        connect(m_thumbJob,
                SIGNAL(signalThumbnail(const KURL&, const QPixmap&)),
                SLOT(slotGotThumbnail(const KURL&, const QPixmap&)));

        connect(m_thumbJob,
                SIGNAL(signalFailed(const KURL&)),
                SLOT(slotFailedThumbnail(const KURL&)));

        connect(m_thumbJob, 
                SIGNAL(signalCompleted()),
                SLOT(slotCompleted()));
    }
    
    return 0;
}

void PixmapManager::remove(const KURL& url)
{
    m_cache->remove(url.path());

    if (!m_thumbJob.isNull())
        m_thumbJob->removeItem(url);
}

void PixmapManager::clear()
{
    if (!m_thumbJob.isNull())
    {
        m_thumbJob->kill();
    }

    m_cache->clear();
}

void PixmapManager::slotGotThumbnail(const KURL& url, const QPixmap& pix)
{
    m_cache->remove(url.path());
    QPixmap* thumb = new QPixmap(pix);
    m_cache->insert(url.path(), thumb);
    emit signalPixmap(url);
}

void PixmapManager::slotFailedThumbnail(const KURL& url)
{
    QString dir = KGlobal::dirs()->findResourceDir("digikam_imagebroken",
                                                   "image_broken.png");
    dir = dir + "/image_broken.png";

    QImage img(dir);
    img = img.smoothScale(m_size, m_size, QImage::ScaleMin);
    
    m_cache->remove(url.path());
    QPixmap* thumb = new QPixmap(img);
    m_cache->insert(url.path(), thumb);
    emit signalPixmap(url);
}

void PixmapManager::slotCompleted()
{
    if (!m_thumbJob.isNull())
        m_thumbJob->kill();

    AlbumIconItem* item = m_view->nextItemToThumbnail();
    if (!item)
        return;

    find(item->imageInfo()->kurl());
}

int PixmapManager::cacheSize() const
{
    return m_cache->maxCost();    
}


#include "pixmapmanager.moc"
