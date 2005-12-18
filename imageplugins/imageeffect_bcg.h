/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-06-05
 * Description : digiKam image editor Brightness/Contrast/Gamma 
 *               correction tool
 * 
 * Copyright 2004 by Renchi Raju
 * Copyright 2005 by Gilles Caulier
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

#ifndef IMAGEEFFECT_BCG_H
#define IMAGEEFFECT_BCG_H

// Digikam include.

#include "imagedlgbase.h"

class QCheckBox;
class QComboBox;
class QHButtonGroup;

class KDoubleNumInput;

namespace Digikam
{
class HistogramWidget;
class ColorGradientWidget;
class ImageGuideWidget;
class DColor;
}

class ImageEffect_BCG : public Digikam::ImageDlgBase
{
    Q_OBJECT

public:

    ImageEffect_BCG(QWidget *parent);
    ~ImageEffect_BCG();

private:

    enum HistogramScale
    {
    Linear=0,
    Logarithmic
    };

    enum ColorChannel
    {
    LuminosityChannel=0,
    RedChannel,
    GreenChannel,
    BlueChannel
    };

    uchar                        *m_destinationPreviewData;

    QComboBox                    *m_channelCB;    
    
    QHButtonGroup                *m_scaleBG;  

    QCheckBox                    *m_overExposureIndicatorBox;
    
    KDoubleNumInput              *m_bInput;
    KDoubleNumInput              *m_cInput;
    KDoubleNumInput              *m_gInput;
    
    Digikam::ImageGuideWidget    *m_previewWidget;

    Digikam::ColorGradientWidget *m_hGradient;
    
    Digikam::HistogramWidget     *m_histogramWidget;

private slots:

    void slotDefault();
    void slotEffect();
    void slotOk();
    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotColorSelectedFromTarget( const Digikam::DColor &color );
};

#endif /* IMAGEEFFECT_BCG_H */
