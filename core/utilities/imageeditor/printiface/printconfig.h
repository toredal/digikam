/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : image editor printing interface.
 *
 * Copyright (C) 2009 by Angelo Naselli <anaselli at linux dot it>
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

#ifndef DIGIKAM_PRINT_CONFIG_H
#define DIGIKAM_PRINT_CONFIG_H

// Qt includes

#include <QCoreApplication>
#include <QDebug>

// KDE includes

#include <kconfigskeleton.h>

// Local includes

#include "digikam_export.h"
#include "printoptionspage.h"

namespace Digikam
{

class DIGIKAM_EXPORT PrintConfig : public KConfigSkeleton
{
public:

    static PrintConfig* self();
    ~PrintConfig();

public:

    static void setPrintPosition( int v )
    {
        if (!self()->isImmutable( QLatin1String( "PrintPosition" ) ))
            self()->mPrintPosition = v;
    }

    static int printPosition()
    {
        return self()->mPrintPosition;
    }

    static void setPrintScaleMode(Digikam::PrintOptionsPage::ScaleMode v)
    {
        if (!self()->isImmutable( QLatin1String( "PrintScaleMode" ) ))
            self()->mPrintScaleMode = v;
    }

    static PrintOptionsPage::ScaleMode printScaleMode()
    {
        return static_cast<PrintOptionsPage::ScaleMode>(self()->mPrintScaleMode);
    }

    static void setPrintEnlargeSmallerImages(bool v)
    {
        if (!self()->isImmutable( QLatin1String( "PrintEnlargeSmallerImages" ) ))
            self()->mPrintEnlargeSmallerImages = v;
    }

    static bool printEnlargeSmallerImages()
    {
        return self()->mPrintEnlargeSmallerImages;
    }

    static void setPrintWidth(double v)
    {
        if (!self()->isImmutable( QLatin1String( "PrintWidth" ) ))
            self()->mPrintWidth = v;
    }

    static double printWidth()
    {
        return self()->mPrintWidth;
    }

    static void setPrintHeight( double v )
    {
        if (!self()->isImmutable( QLatin1String( "PrintHeight" ) ))
            self()->mPrintHeight = v;
    }

    static double printHeight()
    {
        return self()->mPrintHeight;
    }

    static void setPrintUnit( PrintOptionsPage::Unit v )
    {
        if (!self()->isImmutable( QLatin1String( "PrintUnit" ) ))
            self()->mPrintUnit = v;
    }

    static PrintOptionsPage::Unit printUnit()
    {
        return static_cast<PrintOptionsPage::Unit>(self()->mPrintUnit);
    }

    static void setPrintKeepRatio( bool v )
    {
        if (!self()->isImmutable( QLatin1String( "PrintKeepRatio" ) ))
            self()->mPrintKeepRatio = v;
    }

    static bool printKeepRatio()
    {
        return self()->mPrintKeepRatio;
    }

    static void setPrintColorManaged( bool v )
    {
        if (!self()->isImmutable( QLatin1String( "PrintColorManaged" ) ))
            self()->mPrintColorManaged = v;
    }

    static bool printColorManaged()
    {
        return self()->mPrintColorManaged;
    }

    static void setPrintAutoRotate( bool v )
    {
        if (!self()->isImmutable( QLatin1String( "PrintAutoRotate" ) ))
            self()->mPrintAutoRotate = v;
    }

    static bool printAutoRotate()
    {
        return self()->mPrintAutoRotate;
    }

protected:

    PrintConfig();
    friend class PrintConfigHelper;

protected:

    int    mPrintPosition;
    int    mPrintScaleMode;
    bool   mPrintEnlargeSmallerImages;
    double mPrintWidth;
    double mPrintHeight;
    int    mPrintUnit;
    bool   mPrintKeepRatio;
    bool   mPrintColorManaged;
    bool   mPrintAutoRotate;
};

} // namespace Digikam

#endif // DIGIKAM_PRINT_CONFIG_H
