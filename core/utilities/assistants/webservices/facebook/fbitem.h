/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-26
 * Description : a tool to export items to Facebook web service
 *
 * Copyright (C) 2008-2009 by Luka Renko <lure at kubuntu dot org>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_FB_ITEM_H
#define DIGIKAM_FB_ITEM_H

// Qt includes

#include <QString>

// Local includes

#include "wsitem.h"

namespace Digikam
{

class FbUser
{
public:

    FbUser()
    {
        uploadPerm = false;
    }

    void clear()
    {
        id.clear();
        name.clear();
        profileURL = QStringLiteral("https://www.facebook.com");
        uploadPerm = true;
    }

    QString   id;

    QString   name;
    QString   profileURL;
    bool      uploadPerm;
};

// ---------------------------------------------------------------

enum FbPrivacy
{
    FB_ME = 0,
    FB_FRIENDS = 1,
    FB_FRIENDS_OF_FRIENDS,
//     FB_NETWORKS, //NETWORK is deprecated in latest version of Graph API
    FB_EVERYONE,
    FB_CUSTOM
};

// ---------------------------------------------------------------

class FbAlbum: public WSAlbum
{
public:

    explicit FbAlbum()
      : WSAlbum()
    {
        privacy = FB_FRIENDS;
    }
    
    explicit FbAlbum(const WSAlbum& baseAlbum)
      : WSAlbum(baseAlbum)
    {
        privacy = FB_FRIENDS;
    }

    FbPrivacy privacy;
};

// ---------------------------------------------------------------

class FbPhoto
{
public:

    FbPhoto()
    {
    }

    QString id;

    QString caption;
    QString thumbURL;
    QString originalURL;
};

} // namespace Digikam

#endif // DIGIKAM_FB_ITEM_H
