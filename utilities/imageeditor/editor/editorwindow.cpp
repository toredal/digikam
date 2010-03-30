/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-20
 * Description : main image editor GUI implementation
 *
 * Copyright (C) 2006-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "editorwindow_p.h"
#include "editorwindow.moc"

// C++ includes

#include <cmath>

// Qt includes

#include <QByteArray>
#include <QCursor>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QLabel>
#include <QLayout>
#include <QPointer>
#include <QProgressBar>
#include <QSignalMapper>
#include <QSplitter>
#include <QTimer>
#include <QToolButton>
#include <QWidgetAction>
#include <QButtonGroup>

// KDE includes

#include <kdeversion.h>
#include <kaboutdata.h>
#include <kaction.h>
#if KDE_IS_VERSION(4,1,68)
#include <kactioncategory.h>
#endif
#include <kactioncollection.h>
#include <kactionmenu.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kedittoolbar.h>
#include <kfiledialog.h>
#include <kfilefiltercombo.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kimageio.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <knotifyconfigwidget.h>
#include <kprotocolinfo.h>
#include <kselectaction.h>
#include <kservice.h>
#include <kservicetype.h>
#include <kservicetypetrader.h>
#include <kshortcutsdialog.h>
#include <kstandardaction.h>
#include <kstandarddirs.h>
#include <kstandardshortcut.h>
#include <kstatusbar.h>
#include <ktoggleaction.h>
#include <ktogglefullscreenaction.h>
#include <ktoolbar.h>
#include <ktoolbarpopupaction.h>
#include <ktoolinvocation.h>
#include <kurlcombobox.h>
#include <kwindowsystem.h>
#include <kxmlguifactory.h>
#include <kde_file.h>
#include <kdebug.h>
#include <ksqueezedtextlabel.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <kio/copyjob.h>

// LibKDcraw includes

#include <libkdcraw/version.h>

// Local includes

#include "buttonicondisabler.h"
#include "canvas.h"
#include "colorcorrectiondlg.h"
#include "dimginterface.h"
#include "dlogoaction.h"
#include "dpopupmenu.h"
#include "dzoombar.h"
#include "editorstackview.h"
#include "editortooliface.h"
#include "exposurecontainer.h"
#include "filesaveoptionsbox.h"
#include "filesaveoptionsdlg.h"
#include "iccmanager.h"
#include "iccsettings.h"
#include "iccsettingscontainer.h"
#include "icctransform.h"
#include "imagedialog.h"
#include "imageplugin.h"
#include "imagepluginloader.h"
#include "iofilesettingscontainer.h"
#include "libsinfodlg.h"
#include "loadingcacheinterface.h"
#include "printhelper.h"
#include "jpegsettings.h"
#include "pngsettings.h"
#include "rawcameradlg.h"
#include "savingcontextcontainer.h"
#include "sidebar.h"
#include "slideshowsettings.h"
#include "softproofdialog.h"
#include "statusprogressbar.h"
#include "themeengine.h"
#include "thumbbar.h"
#include "thumbnailsize.h"

namespace Digikam
{

const QString EditorWindow::CONFIG_GROUP_NAME = "ImageViewer Settings";

EditorWindow::EditorWindow(const char *name)
            : KXmlGuiWindow(0), d(new EditorWindowPriv)
{
    setObjectName(name);
    setWindowFlags(Qt::Window);

    m_themeMenuAction        = 0;
    m_contextMenu            = 0;
    m_canvas                 = 0;
    m_imagePluginLoader      = 0;
    m_undoAction             = 0;
    m_redoAction             = 0;
    m_fullScreenAction       = 0;
    m_saveAction             = 0;
    m_saveAsAction           = 0;
    m_revertAction           = 0;
    m_fileDeleteAction       = 0;
    m_forwardAction          = 0;
    m_backwardAction         = 0;
    m_firstAction            = 0;
    m_lastAction             = 0;
    m_undoAction             = 0;
    m_redoAction             = 0;
    m_showBarAction          = 0;
    m_splitter               = 0;
    m_vSplitter              = 0;
    m_stackView              = 0;
    m_animLogo               = 0;
    m_fullScreen             = false;
    m_rotatedOrFlipped       = false;
    m_setExifOrientationTag  = true;
    m_cancelSlideShow        = false;
    m_fullScreenHideThumbBar = true;

    // Settings containers instance.

    d->ICCSettings      = new ICCSettingsContainer();
    d->exposureSettings = new ExposureSettingsContainer();
    d->toolIface        = new EditorToolIface(this);
    m_IOFileSettings    = new IOFileSettingsContainer();
    m_savingContext     = new SavingContextContainer();
    d->waitingLoop      = new QEventLoop(this);
}

EditorWindow::~EditorWindow()
{
    delete m_canvas;
    delete m_IOFileSettings;
    delete m_savingContext;
    delete d->ICCSettings;
    delete d->exposureSettings;
    delete d;
}

EditorStackView* EditorWindow::editorStackView() const
{
    return m_stackView;
}

ExposureSettingsContainer* EditorWindow::exposureSettings() const
{
    return d->exposureSettings;
}

ICCSettingsContainer* EditorWindow::cmSettings() const
{
    return d->ICCSettings;
}

void EditorWindow::setupContextMenu()
{
    m_contextMenu         = new DPopupMenu(this);
    KActionCollection *ac = actionCollection();
    if (ac->action("editorwindow_backward"))
        m_contextMenu->addAction(ac->action("editorwindow_backward"));
    if (ac->action("editorwindow_forward"))
        m_contextMenu->addAction(ac->action("editorwindow_forward"));
    m_contextMenu->addSeparator();
    if (ac->action("editorwindow_slideshow"))
        m_contextMenu->addAction(ac->action("editorwindow_slideshow"));
    if (ac->action("editorwindow_rotate_left"))
        m_contextMenu->addAction(ac->action("editorwindow_rotate_left"));
    if (ac->action("editorwindow_rotate_right"))
        m_contextMenu->addAction(ac->action("editorwindow_rotate_right"));
    if (ac->action("editorwindow_crop"))
        m_contextMenu->addAction(ac->action("editorwindow_crop"));
    m_contextMenu->addSeparator();
    if (ac->action("editorwindow_delete"))
        m_contextMenu->addAction(ac->action("editorwindow_delete"));
}

void EditorWindow::setupStandardConnections()
{
    connect(m_stackView, SIGNAL(signalToggleOffFitToWindow()),
            this, SLOT(slotToggleOffFitToWindow()));

    // -- Canvas connections ------------------------------------------------

    connect(m_canvas, SIGNAL(signalShowNextImage()),
            this, SLOT(slotForward()));

    connect(m_canvas, SIGNAL(signalShowPrevImage()),
            this, SLOT(slotBackward()));

    connect(m_canvas, SIGNAL(signalRightButtonClicked()),
            this, SLOT(slotContextMenu()));

    connect(m_stackView, SIGNAL(signalZoomChanged(bool, bool, double)),
            this, SLOT(slotZoomChanged(bool, bool, double)));

    connect(m_canvas, SIGNAL(signalChanged()),
            this, SLOT(slotChanged()));

    connect(m_canvas, SIGNAL(signalUndoStateChanged(bool, bool, bool)),
            this, SLOT(slotUndoStateChanged(bool, bool, bool)));

    connect(m_canvas, SIGNAL(signalSelected(bool)),
            this, SLOT(slotSelected(bool)));

    connect(m_canvas, SIGNAL(signalPrepareToLoad()),
            this, SLOT(slotPrepareToLoad()));

    connect(m_canvas, SIGNAL(signalLoadingStarted(const QString&)),
            this, SLOT(slotLoadingStarted(const QString&)));

    connect(m_canvas, SIGNAL(signalLoadingFinished(const QString&, bool)),
            this, SLOT(slotLoadingFinished(const QString&, bool)));

    connect(m_canvas, SIGNAL(signalLoadingProgress(const QString&, float)),
            this, SLOT(slotLoadingProgress(const QString&, float)));

    connect(m_canvas, SIGNAL(signalSavingStarted(const QString&)),
            this, SLOT(slotSavingStarted(const QString&)));

    connect(m_canvas, SIGNAL(signalSavingFinished(const QString&, bool)),
            this, SLOT(slotSavingFinished(const QString&, bool)));

    connect(m_canvas, SIGNAL(signalSavingProgress(const QString&, float)),
            this, SLOT(slotSavingProgress(const QString&, float)));

    connect(m_canvas, SIGNAL(signalSelectionChanged(const QRect&)),
            this, SLOT(slotSelectionChanged(const QRect&)));

    // -- if rotating/flipping set the rotatedflipped flag to true -----------

    connect(d->rotateLeftAction, SIGNAL(activated()),
            this, SLOT(slotRotatedOrFlipped()));

    connect(d->rotateRightAction, SIGNAL(activated()),
            this, SLOT(slotRotatedOrFlipped()));

    connect(d->flipHorizAction, SIGNAL(activated()),
            this, SLOT(slotRotatedOrFlipped()));

    connect(d->flipVertAction, SIGNAL(activated()),
            this, SLOT(slotRotatedOrFlipped()));

    // -- status bar connections --------------------------------------

    connect(m_nameLabel, SIGNAL(signalCancelButtonPressed()),
            this, SLOT(slotNameLabelCancelButtonPressed()));

    connect(m_nameLabel, SIGNAL(signalCancelButtonPressed()),
            d->toolIface, SLOT(slotToolAborted()));

    // -- Icc settings connections --------------------------------------

    connect(IccSettings::instance(), SIGNAL(settingsChanged()),
            this, SLOT(slotColorManagementOptionsChanged()));
}

void EditorWindow::setupStandardActions()
{
    // -- Standard 'File' menu actions ---------------------------------------------

    m_backwardAction = KStandardAction::back(this, SLOT(slotBackward()), this);
    actionCollection()->addAction("editorwindow_backward", m_backwardAction);
    m_backwardAction->setShortcut( KShortcut(Qt::Key_PageUp, Qt::Key_Backspace) );

    m_forwardAction = KStandardAction::forward(this, SLOT(slotForward()), this);
    actionCollection()->addAction("editorwindow_forward", m_forwardAction);
    m_forwardAction->setShortcut( KShortcut(Qt::Key_PageDown, Qt::Key_Space) );

    m_firstAction = new KAction(KIcon("go-first"), i18n("&First"), this);
    m_firstAction->setShortcut(KStandardShortcut::begin());
    connect(m_firstAction, SIGNAL(triggered()), this, SLOT(slotFirst()));
    actionCollection()->addAction("editorwindow_first", m_firstAction);

    m_lastAction = new KAction(KIcon("go-last"), i18n("&Last"), this);
    m_lastAction->setShortcut(KStandardShortcut::end());
    connect(m_lastAction, SIGNAL(triggered()), this, SLOT(slotLast()));
    actionCollection()->addAction("editorwindow_last", m_lastAction);

    m_saveAction = KStandardAction::save(this, SLOT(slotSave()), this);
    actionCollection()->addAction("editorwindow_save", m_saveAction);

    m_saveAsAction = KStandardAction::saveAs(this, SLOT(slotSaveAs()), this);
    actionCollection()->addAction("editorwindow_saveas", m_saveAsAction);

    m_revertAction = KStandardAction::revert(this, SLOT(slotRevert()), this);
    actionCollection()->addAction("editorwindow_revert", m_revertAction);

    m_saveAction->setEnabled(false);
    m_saveAsAction->setEnabled(false);
    m_revertAction->setEnabled(false);

    d->filePrintAction = new KAction(KIcon("document-print-frame"), i18n("Print Image..."), this);
    d->filePrintAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_P));
    connect(d->filePrintAction, SIGNAL(triggered()), this, SLOT(slotFilePrint()));
    actionCollection()->addAction("editorwindow_print", d->filePrintAction);

    m_fileDeleteAction = new KAction(KIcon("user-trash"), i18nc("Non-pluralized", "Move to Trash"), this);
    m_fileDeleteAction->setShortcut(KShortcut(Qt::Key_Delete));
    connect(m_fileDeleteAction, SIGNAL(triggered()), this, SLOT(slotDeleteCurrentItem()));
    actionCollection()->addAction("editorwindow_delete", m_fileDeleteAction);

    KAction* closeAction = KStandardAction::close(this, SLOT(close()), this);
    actionCollection()->addAction("editorwindow_close", closeAction);

    // -- Standard 'Edit' menu actions ---------------------------------------------

    d->copyAction = KStandardAction::copy(m_canvas, SLOT(slotCopy()), this);
    actionCollection()->addAction("editorwindow_copy", d->copyAction);
    d->copyAction->setEnabled(false);

    m_undoAction = new KToolBarPopupAction(KIcon("edit-undo"), i18n("Undo"), this);
    m_undoAction->setShortcut(KStandardShortcut::undo());
    m_undoAction->setEnabled(false);
    actionCollection()->addAction("editorwindow_undo", m_undoAction);

    connect(m_undoAction->menu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowUndoMenu()));

    // we are using a signal mapper to identify which of a bunch of actions was triggered
    d->undoSignalMapper = new QSignalMapper(this);

    // connect mapper to view
    connect(d->undoSignalMapper, SIGNAL(mapped(int)),
            m_canvas, SLOT(slotUndo(int)));

    // connect simple undo action
    connect(m_undoAction, SIGNAL(triggered()), d->undoSignalMapper, SLOT(map()));
    d->undoSignalMapper->setMapping(m_undoAction, 1);

    m_redoAction = new KToolBarPopupAction(KIcon("edit-redo"), i18n("Redo"), this);
    m_redoAction->setShortcut(KStandardShortcut::redo());
    m_redoAction->setEnabled(false);
    actionCollection()->addAction("editorwindow_redo", m_redoAction);

    connect(m_redoAction->menu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowRedoMenu()));

    d->redoSignalMapper = new QSignalMapper(this);

    connect(d->redoSignalMapper, SIGNAL(mapped(int)),
            m_canvas, SLOT(slotRedo(int)));

    connect(m_redoAction, SIGNAL(triggered()), d->redoSignalMapper, SLOT(map()));
    d->redoSignalMapper->setMapping(m_redoAction, 1);

    d->selectAllAction = new KAction(i18n("Select All"), this);
    d->selectAllAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_A));
    connect(d->selectAllAction, SIGNAL(triggered()), m_canvas, SLOT(slotSelectAll()));
    actionCollection()->addAction("editorwindow_selectAll", d->selectAllAction);

    d->selectNoneAction = new KAction(i18n("Select None"), this);
    d->selectNoneAction->setShortcut(KShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_A));
    connect(d->selectNoneAction, SIGNAL(triggered()), m_canvas, SLOT(slotSelectNone()));
    actionCollection()->addAction("editorwindow_selectNone", d->selectNoneAction);

    // -- Standard 'View' menu actions ---------------------------------------------

    d->zoomPlusAction  = KStandardAction::zoomIn(this, SLOT(slotIncreaseZoom()), this);
    KShortcut keysPlus = d->zoomPlusAction->shortcut();
    keysPlus.setAlternate(Qt::Key_Plus);
    d->zoomPlusAction->setShortcut(keysPlus);
    actionCollection()->addAction("editorwindow_zoomplus", d->zoomPlusAction);

    d->zoomMinusAction  = KStandardAction::zoomOut(this, SLOT(slotDecreaseZoom()), this);
    KShortcut keysMinus = d->zoomMinusAction->shortcut();
    keysMinus.setAlternate(Qt::Key_Minus);
    d->zoomMinusAction->setShortcut(keysMinus);
    actionCollection()->addAction("editorwindow_zoomminus", d->zoomMinusAction);

    d->zoomTo100percents = new KAction(KIcon("zoom-original"), i18n("Zoom to 100%"), this);
    d->zoomTo100percents->setShortcut(KShortcut(Qt::ALT + Qt::CTRL + Qt::Key_0));       // NOTE: Photoshop 7 use ALT+CTRL+0
    connect(d->zoomTo100percents, SIGNAL(triggered()), this, SLOT(slotZoomTo100Percents()));
    actionCollection()->addAction("editorwindow_zoomto100percents", d->zoomTo100percents);

    d->zoomFitToWindowAction = new KToggleAction(KIcon("zoom-fit-best"), i18n("Fit to &Window"), this);
    d->zoomFitToWindowAction->setShortcut(KShortcut(Qt::ALT + Qt::CTRL + Qt::Key_E));
    connect(d->zoomFitToWindowAction, SIGNAL(triggered()), this, SLOT(slotToggleFitToWindow()));
    actionCollection()->addAction("editorwindow_zoomfit2window", d->zoomFitToWindowAction);

    d->zoomFitToSelectAction = new KAction(KIcon("zoom-select-fit"), i18n("Fit to &Selection"), this);
    d->zoomFitToSelectAction->setShortcut(KShortcut(Qt::ALT + Qt::CTRL + Qt::Key_S));   // NOTE: Photoshop 7 use ALT+CTRL+0
    connect(d->zoomFitToSelectAction, SIGNAL(triggered()), this, SLOT(slotFitToSelect()));
    actionCollection()->addAction("editorwindow_zoomfit2select", d->zoomFitToSelectAction);
    d->zoomFitToSelectAction->setEnabled(false);
    d->zoomFitToSelectAction->setWhatsThis(i18n("This option can be used to zoom the image to the "
                                                "current selection area."));

    // --------------------------------------------------------

    m_fullScreenAction = KStandardAction::fullScreen(this, SLOT(slotToggleFullScreen()), this, this);
    actionCollection()->addAction("editorwindow_fullscreen", m_fullScreenAction);

    d->slideShowAction = new KAction(KIcon("view-presentation"), i18n("Slideshow"), this);
    d->slideShowAction->setShortcut(KShortcut(Qt::Key_F9));
    connect(d->slideShowAction, SIGNAL(triggered()), this, SLOT(slotToggleSlideShow()));
    actionCollection()->addAction("editorwindow_slideshow", d->slideShowAction);

    d->viewUnderExpoAction = new KToggleAction(KIcon("underexposure"), i18n("Under-Exposure Indicator"), this);
    d->viewUnderExpoAction->setShortcut(KShortcut(Qt::Key_F10));
    d->viewUnderExpoAction->setWhatsThis(i18n("Set this option to display black "
                                              "overlaid on the image. This will help you to avoid "
                                              "under-exposing the image."));
    connect(d->viewUnderExpoAction, SIGNAL(triggered(bool)), this, SLOT(slotSetUnderExposureIndicator(bool)));
    actionCollection()->addAction("editorwindow_underexposure", d->viewUnderExpoAction);

    d->viewOverExpoAction = new KToggleAction(KIcon("overexposure"), i18n("Over-Exposure Indicator"), this);
    d->viewOverExpoAction->setShortcut(KShortcut(Qt::Key_F11));
    d->viewOverExpoAction->setWhatsThis(i18n("Set this option to display white "
                                             "overlaid on the image. This will help you to avoid "
                                             "over-exposing the image." ) );
    connect(d->viewOverExpoAction, SIGNAL(triggered(bool)), this, SLOT(slotSetOverExposureIndicator(bool)));
    actionCollection()->addAction("editorwindow_overexposure", d->viewOverExpoAction);

    d->viewCMViewAction = new KToggleAction(KIcon("video-display"), i18n("Color-Managed View"), this);
    d->viewCMViewAction->setShortcut(KShortcut(Qt::Key_F12));
    connect(d->viewCMViewAction, SIGNAL(triggered()), this, SLOT(slotToggleColorManagedView()));
    actionCollection()->addAction("editorwindow_cmview", d->viewCMViewAction);

    d->softProofOptionsAction = new KAction(KIcon("printer"), i18n("Soft Proofing Options..."), this);
    connect(d->softProofOptionsAction, SIGNAL(triggered()), this, SLOT(slotSoftProofingOptions()));
    actionCollection()->addAction("editorwindow_softproofoptions", d->softProofOptionsAction);

    d->viewSoftProofAction = new KToggleAction(KIcon("document-print-preview"), i18n("Soft Proofing View"), this);
    connect(d->viewSoftProofAction, SIGNAL(triggered()), this, SLOT(slotUpdateSoftProofingState()));
    actionCollection()->addAction("editorwindow_softproofview", d->viewSoftProofAction);

    m_showBarAction = thumbBar()->getToggleAction(this);
    actionCollection()->addAction("editorwindow_showthumbs", m_showBarAction);

    // -- Standard 'Transform' menu actions ---------------------------------------------

    d->cropAction = new KAction(KIcon("transform-crop-and-resize"), i18n("Crop"), this);
    d->cropAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_X));
    connect(d->cropAction, SIGNAL(triggered()), m_canvas, SLOT(slotCrop()));
    actionCollection()->addAction("editorwindow_crop", d->cropAction);
    d->cropAction->setEnabled(false);
    d->cropAction->setWhatsThis(i18n("This option can be used to crop the image. "
                                     "Select a region of the image to enable this action."));

    // -- Standard 'Flip' menu actions ---------------------------------------------

    d->flipHorizAction = new KAction(KIcon("object-flip-horizontal"), i18n("Flip Horizontally"), this);
    d->flipHorizAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_Asterisk));
    connect(d->flipHorizAction, SIGNAL(triggered()), m_canvas, SLOT(slotFlipHoriz()));
    actionCollection()->addAction("editorwindow_flip_horiz", d->flipHorizAction);
    d->flipHorizAction->setEnabled(false);

    d->flipVertAction = new KAction(KIcon("object-flip-vertical"), i18n("Flip Vertically"), this);
    d->flipVertAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_Slash));
    connect(d->flipVertAction, SIGNAL(triggered()), m_canvas, SLOT(slotFlipVert()));
    actionCollection()->addAction("editorwindow_flip_vert", d->flipVertAction);
    d->flipVertAction->setEnabled(false);

    // -- Standard 'Rotate' menu actions ----------------------------------------

    d->rotateLeftAction = new KAction(KIcon("object-rotate-left"), i18n("Rotate Left"), this);
    d->rotateLeftAction->setShortcut(KShortcut(Qt::SHIFT+Qt::CTRL+Qt::Key_Left));
    connect(d->rotateLeftAction, SIGNAL(triggered()), m_canvas, SLOT(slotRotate270()));
    actionCollection()->addAction("editorwindow_rotate_left", d->rotateLeftAction);
    d->rotateLeftAction->setEnabled(false);

    d->rotateRightAction = new KAction(KIcon("object-rotate-right"), i18n("Rotate Right"), this);
    d->rotateRightAction->setShortcut(KShortcut(Qt::SHIFT+Qt::CTRL+Qt::Key_Right));
    connect(d->rotateRightAction, SIGNAL(triggered()), m_canvas, SLOT(slotRotate90()));
    actionCollection()->addAction("editorwindow_rotate_right", d->rotateRightAction);
    d->rotateRightAction->setEnabled(false);

    // -- Standard 'Configure' menu actions ----------------------------------------

    d->showMenuBarAction = KStandardAction::showMenubar(this, SLOT(slotShowMenuBar()), actionCollection());
    d->showMenuBarAction->setChecked(!menuBar()->isHidden());  // NOTE: workaround for B.K.O #171080

    KStandardAction::keyBindings(this,            SLOT(slotEditKeys()),          actionCollection());
    KStandardAction::configureToolbars(this,      SLOT(slotConfToolbars()),      actionCollection());
    KStandardAction::configureNotifications(this, SLOT(slotConfNotifications()), actionCollection());
    KStandardAction::preferences(this,            SLOT(slotSetup()),             actionCollection());

    // ---------------------------------------------------------------------------------

    m_themeMenuAction = new KSelectAction(i18n("&Themes"), this);
    m_themeMenuAction->setItems(ThemeEngine::instance()->themeNames());
    connect(m_themeMenuAction, SIGNAL(triggered(const QString&)),
            this, SLOT(slotChangeTheme(const QString&)));
    actionCollection()->addAction("theme_menu", m_themeMenuAction);

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    // -- Standard 'Help' menu actions ---------------------------------------------

    d->donateMoneyAction = new KAction(i18n("Donate Money..."), this);
    connect(d->donateMoneyAction, SIGNAL(triggered()), this, SLOT(slotDonateMoney()));
    actionCollection()->addAction("editorwindow_donatemoney", d->donateMoneyAction);

    d->contributeAction = new KAction(i18n("Contribute..."), this);
    connect(d->contributeAction, SIGNAL(triggered()), this, SLOT(slotContribute()));
    actionCollection()->addAction("editorwindow_contribute", d->contributeAction);

    d->rawCameraListAction = new KAction(KIcon("kdcraw"), i18n("Supported RAW Cameras"), this);
    connect(d->rawCameraListAction, SIGNAL(triggered()), this, SLOT(slotRawCameraList()));
    actionCollection()->addAction("editorwindow_rawcameralist", d->rawCameraListAction);

    d->libsInfoAction = new KAction(KIcon("help-about"), i18n("Components Information"), this);
    connect(d->libsInfoAction, SIGNAL(triggered()), this, SLOT(slotComponentsInfo()));
    actionCollection()->addAction("editorwindow_librariesinfo", d->libsInfoAction);

    // -- Keyboard-only actions added to <MainWindow> ------------------------------

    KAction *closeToolAction = new KAction(i18n("Close Tool"), this);
    actionCollection()->addAction("editorwindow_closetool", closeToolAction);
    closeToolAction->setShortcut(KShortcut(Qt::Key_Escape) );
    connect(closeToolAction, SIGNAL(triggered()), this, SLOT(slotCloseTool()));

    KAction *altBackwardAction = new KAction(i18n("Previous Image"), this);
    actionCollection()->addAction("editorwindow_backward_shift_space", altBackwardAction);
    altBackwardAction->setShortcut( KShortcut(Qt::SHIFT+Qt::Key_Space) );
    connect(altBackwardAction, SIGNAL(triggered()), this, SLOT(slotBackward()));

    m_animLogo = new DLogoAction(this);
    actionCollection()->addAction("logo_action", m_animLogo);
}

void EditorWindow::setupStatusBar()
{
    m_nameLabel = new StatusProgressBar(statusBar());
    m_nameLabel->setAlignment(Qt::AlignCenter);
    statusBar()->addWidget(m_nameLabel, 100);

    d->infoLabel = new KSqueezedTextLabel(i18n("No selection"), statusBar());
    d->infoLabel->setAlignment(Qt::AlignCenter);
    statusBar()->addWidget(d->infoLabel, 100);

    m_resLabel   = new QLabel(statusBar());
    m_resLabel->setAlignment(Qt::AlignCenter);
    statusBar()->addWidget(m_resLabel, 100);
    m_resLabel->setToolTip( i18n("Information about image size"));

    d->zoomBar   = new DZoomBar(statusBar());
    d->zoomBar->setZoomToFitAction(d->zoomFitToWindowAction);
    d->zoomBar->setZoomTo100Action(d->zoomTo100percents);
    d->zoomBar->setZoomPlusAction(d->zoomPlusAction);
    d->zoomBar->setZoomMinusAction(d->zoomMinusAction);
    d->zoomBar->setBarMode(DZoomBar::PreviewZoomCtrl);
    statusBar()->addPermanentWidget(d->zoomBar);

    connect(d->zoomBar, SIGNAL(signalZoomSliderChanged(int)),
            m_stackView, SLOT(slotZoomSliderChanged(int)));

    connect(d->zoomBar, SIGNAL(signalZoomValueEdited(double)),
            m_stackView, SLOT(setZoomFactor(double)));

    d->previewToolBar = new PreviewToolBar(statusBar());
    d->previewToolBar->setEnabled(false);
    statusBar()->addPermanentWidget(d->previewToolBar);

    connect(d->previewToolBar, SIGNAL(signalPreviewModeChanged(int)),
            this, SIGNAL(signalPreviewModeChanged(int)));

    QWidget* buttonsBox      = new QWidget(statusBar());
    QHBoxLayout *hlay        = new QHBoxLayout(buttonsBox);
    QButtonGroup *buttonsGrp = new QButtonGroup(buttonsBox);
    buttonsGrp->setExclusive(false);

    d->underExposureIndicator = new QToolButton(buttonsBox);
    d->underExposureIndicator->setDefaultAction(d->viewUnderExpoAction);
//    new ButtonIconDisabler(d->underExposureIndicator);

    d->overExposureIndicator  = new QToolButton(buttonsBox);
    d->overExposureIndicator->setDefaultAction(d->viewOverExpoAction);
//    new ButtonIconDisabler(d->overExposureIndicator);

    d->cmViewIndicator        = new QToolButton(buttonsBox);
    d->cmViewIndicator->setDefaultAction(d->viewCMViewAction);
//    new ButtonIconDisabler(d->cmViewIndicator);

    buttonsGrp->addButton(d->underExposureIndicator);
    buttonsGrp->addButton(d->overExposureIndicator);
    buttonsGrp->addButton(d->cmViewIndicator);

    hlay->setSpacing(0);
    hlay->setMargin(0);
    hlay->addWidget(d->underExposureIndicator);
    hlay->addWidget(d->overExposureIndicator);
    hlay->addWidget(d->cmViewIndicator);

    statusBar()->addPermanentWidget(buttonsBox);
}

void EditorWindow::printImage(const KUrl& /*url*/)
{
    uchar* ptr      = m_canvas->interface()->getImage();
    int w           = m_canvas->interface()->origWidth();
    int h           = m_canvas->interface()->origHeight();
    bool hasAlpha   = m_canvas->interface()->hasAlpha();
    bool sixteenBit = m_canvas->interface()->sixteenBit();

    if (!ptr || !w || !h)
        return;

    DImg image(w, h, sixteenBit, hasAlpha, ptr);

    PrintHelper printHelp(this);
    printHelp.print(image);
}

void EditorWindow::slotEditKeys()
{
    KShortcutsDialog dialog(KShortcutsEditor::AllActions,
                            KShortcutsEditor::LetterShortcutsAllowed, this);
    dialog.addCollection(actionCollection(), i18nc("general editor shortcuts", "General"));
    dialog.addCollection(d->imagepluginsActionCollection, i18nc("imageplugins shortcuts", "Image Plugins"));
    dialog.configure();
}

void EditorWindow::slotAboutToShowUndoMenu()
{
    m_undoAction->menu()->clear();
    QStringList titles;
    m_canvas->getUndoHistory(titles);

    for (int i=0; i<titles.size(); ++i)
    {
        QAction *action = m_undoAction->menu()->addAction(titles[i], d->undoSignalMapper, SLOT(map()));
        d->undoSignalMapper->setMapping(action, i + 1);
    }
}

void EditorWindow::slotAboutToShowRedoMenu()
{
    m_redoAction->menu()->clear();
    QStringList titles;
    m_canvas->getRedoHistory(titles);

    for (int i=0; i<titles.size(); ++i)
    {
        QAction *action = m_redoAction->menu()->addAction(titles[i], d->redoSignalMapper, SLOT(map()));
        d->redoSignalMapper->setMapping(action, i + 1);
    }
}

void EditorWindow::slotConfToolbars()
{
    saveMainWindowSettings(KGlobal::config()->group(CONFIG_GROUP_NAME));
    KEditToolBar dlg(factory(), this);

    connect(&dlg, SIGNAL(newToolbarConfig()),
            this, SLOT(slotNewToolbarConfig()));

    dlg.exec();
}

void EditorWindow::slotConfNotifications()
{
    KNotifyConfigWidget::configure(this);
}

void EditorWindow::slotNewToolbarConfig()
{
    applyMainWindowSettings(KGlobal::config()->group(CONFIG_GROUP_NAME));
}

void EditorWindow::slotIncreaseZoom()
{
    m_stackView->increaseZoom();
}

void EditorWindow::slotDecreaseZoom()
{
    m_stackView->decreaseZoom();
}

void EditorWindow::slotToggleFitToWindow()
{
    d->zoomPlusAction->setEnabled(true);
    d->zoomBar->setEnabled(true);
    d->zoomMinusAction->setEnabled(true);
    m_stackView->toggleFitToWindow();
}

void EditorWindow::slotFitToSelect()
{
    d->zoomPlusAction->setEnabled(true);
    d->zoomBar->setEnabled(true);
    d->zoomMinusAction->setEnabled(true);
    m_stackView->fitToSelect();
}

void EditorWindow::slotZoomTo100Percents()
{
    d->zoomPlusAction->setEnabled(true);
    d->zoomBar->setEnabled(true);
    d->zoomMinusAction->setEnabled(true);
    m_stackView->zoomTo100Percent();
}

void EditorWindow::slotZoomChanged(bool isMax, bool isMin, double zoom)
{
    d->zoomPlusAction->setEnabled(!isMax);
    d->zoomMinusAction->setEnabled(!isMin);

    double zmin = m_stackView->zoomMin();
    double zmax = m_stackView->zoomMax();
    d->zoomBar->setZoom(zoom, zmin, zmax);
}

void EditorWindow::slotToggleOffFitToWindow()
{
    d->zoomFitToWindowAction->blockSignals(true);
    d->zoomFitToWindowAction->setChecked(false);
    d->zoomFitToWindowAction->blockSignals(false);
}

void EditorWindow::slotEscapePressed()
{
    if (m_fullScreen)
        m_fullScreenAction->activate(QAction::Trigger);
}

void EditorWindow::loadImagePlugins()
{
    if (d->imagepluginsActionCollection)
    {
        d->imagepluginsActionCollection->clear();
        delete d->imagepluginsActionCollection;
    }
    d->imagepluginsActionCollection = new KActionCollection(this, KGlobal::mainComponent());

    QList<ImagePlugin *> pluginList = m_imagePluginLoader->pluginList();

    foreach (ImagePlugin *plugin, pluginList)
    {
        if (plugin)
        {
            guiFactory()->addClient(plugin);
            plugin->setEnabledSelectionActions(false);

            // add actions to imagepluginsActionCollection
#if KDE_IS_VERSION(4,1,68)
            QString categoryStr = plugin->actionCategory();

            if (categoryStr != QString("__INVALID__") && !categoryStr.isEmpty())
            {
                KActionCategory *category = new KActionCategory(categoryStr, d->imagepluginsActionCollection);
                foreach (QAction* action, plugin->actionCollection()->actions())
                {
                    category->addAction(action->objectName(), action);
                }
            }
            else
            {
#endif
                foreach (QAction* action, plugin->actionCollection()->actions())
                {
                    d->imagepluginsActionCollection->addAction(action->objectName(), action);
                }
#if KDE_IS_VERSION(4,1,68)
            }
#endif
        }
        else
        {
            kDebug() << "Invalid plugin to add!";
        }
    }

    // load imagepluginsActionCollection settings
    d->imagepluginsActionCollection->readSettings();
}

void EditorWindow::unLoadImagePlugins()
{
    if (d->imagepluginsActionCollection)
    {
        d->imagepluginsActionCollection->clear();
        delete d->imagepluginsActionCollection;
    }

    QList<ImagePlugin *> pluginList = m_imagePluginLoader->pluginList();

    foreach (ImagePlugin *plugin, pluginList)
    {
        if (plugin)
        {
            guiFactory()->removeClient(plugin);
            plugin->setEnabledSelectionActions(false);
        }
    }
}

void EditorWindow::readStandardSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(CONFIG_GROUP_NAME);

    // Restore Canvas layout
    if (group.hasKey(d->configVerticalSplitterSizesEntry) && m_vSplitter)
    {
        QByteArray state;
        state = group.readEntry(d->configVerticalSplitterStateEntry, state);
        m_vSplitter->restoreState(QByteArray::fromBase64(state));
    }

    // Restore full screen Mode
    if (group.readEntry(d->configFullScreenEntry, false))
    {
        m_fullScreenAction->activate(QAction::Trigger);
        m_fullScreen = true;
    }

    // Restore Auto zoom action
    bool autoZoom = group.readEntry(d->configAutoZoomEntry, true);
    if (autoZoom)
        d->zoomFitToWindowAction->activate(QAction::Trigger);

    slotSetUnderExposureIndicator(group.readEntry(d->configUnderExposureIndicatorEntry, false));
    slotSetOverExposureIndicator(group.readEntry(d->configOverExposureIndicatorEntry, false));
    d->previewToolBar->readSettings(group);
}

void EditorWindow::applyStandardSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();

    slotColorManagementOptionsChanged();

    // -- JPEG, PNG, TIFF JPEG2000 files format settings --------------------------------------

    KConfigGroup group = config->group(CONFIG_GROUP_NAME);

    m_IOFileSettings->JPEGCompression     = JPEGSettings::convertCompressionForLibJpeg(group.readEntry(d->configJpegCompressionEntry, 75));

    m_IOFileSettings->JPEGSubSampling     = group.readEntry(d->configJpegSubSamplingEntry, 1);  // Medium subsampling

    m_IOFileSettings->PNGCompression      = PNGSettings::convertCompressionForLibPng(group.readEntry(d->configPngCompressionEntry, 1));

    // TIFF compression setting.
    m_IOFileSettings->TIFFCompression     = group.readEntry(d->configTiffCompressionEntry, false);

    // JPEG2000 quality slider settings : 1 - 100
    m_IOFileSettings->JPEG2000Compression = group.readEntry(d->configJpeg2000CompressionEntry, 100);

    // JPEG2000 LossLess setting.
    m_IOFileSettings->JPEG2000LossLess    = group.readEntry(d->configJpeg2000LossLessEntry, true);

    // PGF quality slider settings : 1 - 9
    m_IOFileSettings->PGFCompression      = group.readEntry(d->configPgfCompressionEntry, 3);

    // PGF LossLess setting.
    m_IOFileSettings->PGFLossLess         = group.readEntry(d->configPgfLossLessEntry, true);

    // -- RAW images decoding settings ------------------------------------------------------

    m_IOFileSettings->useRAWImport = group.readEntry(d->configUseRawImportToolEntry, false);
    m_IOFileSettings->rawDecodingSettings.readSettings(group);

    // If digiKam Color Management is enable, no need to correct color of decoded RAW image,
    // else, sRGB color workspace will be used.

    if (d->ICCSettings->enableCM)
    {
        if (d->ICCSettings->defaultUncalibratedBehavior & ICCSettingsContainer::AutomaticColors)
        {
            m_IOFileSettings->rawDecodingSettings.outputColorSpace = DRawDecoding::CUSTOMOUTPUTCS;
            m_IOFileSettings->rawDecodingSettings.outputProfile    = d->ICCSettings->workspaceProfile;
        }
        else
        {
            m_IOFileSettings->rawDecodingSettings.outputColorSpace = DRawDecoding::RAWCOLOR;
        }
    }
    else
    {
        m_IOFileSettings->rawDecodingSettings.outputColorSpace = DRawDecoding::SRGB;
    }

    // -- GUI Settings -------------------------------------------------------

    d->legacyUpdateSplitterState(group);
    m_splitter->restoreState(group);

    d->fullScreenHideToolBar = group.readEntry(d->configFullScreenHideToolBarEntry, false);
    m_fullScreenHideThumbBar = group.readEntry(d->configFullScreenHideThumbBarEntry, true);

    slotThemeChanged();

    // -- Exposure Indicators Settings ---------------------------------------

    d->exposureSettings->underExposureColor = group.readEntry(d->configUnderExposureColorEntry, QColor(Qt::white));
    d->exposureSettings->overExposureColor  = group.readEntry(d->configOverExposureColorEntry, QColor(Qt::black));
}

void EditorWindow::saveStandardSettings()
{

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(CONFIG_GROUP_NAME);

    group.writeEntry(d->configAutoZoomEntry, d->zoomFitToWindowAction->isChecked());
    m_splitter->saveState(group);
    if (m_vSplitter)
        group.writeEntry(d->configVerticalSplitterStateEntry, m_vSplitter->saveState().toBase64());

    group.writeEntry("Show Thumbbar", thumbBar()->shouldBeVisible());
    group.writeEntry(d->configFullScreenEntry, m_fullScreenAction->isChecked());
    group.writeEntry(d->configUnderExposureIndicatorEntry, d->exposureSettings->underExposureIndicator);
    group.writeEntry(d->configOverExposureIndicatorEntry, d->exposureSettings->overExposureIndicator);
    d->previewToolBar->writeSettings(group);

    config->sync();

}

/** Method used by Editor Tools. Only tools based on imageregionwidget support zoomming.
    TODO: Fix this behavior when editor tool preview widgets will be factored.
 */
void EditorWindow::toggleZoomActions(bool val)
{
    d->zoomMinusAction->setEnabled(val);
    d->zoomPlusAction->setEnabled(val);
    d->zoomTo100percents->setEnabled(val);
    d->zoomFitToWindowAction->setEnabled(val);
    d->zoomBar->setEnabled(val);
}

void EditorWindow::toggleStandardActions(bool val)
{
    d->zoomFitToSelectAction->setEnabled(val);
    toggleZoomActions(val);

    d->rotateLeftAction->setEnabled(val);
    d->rotateRightAction->setEnabled(val);
    d->flipHorizAction->setEnabled(val);
    d->flipVertAction->setEnabled(val);
    d->filePrintAction->setEnabled(val);
    m_fileDeleteAction->setEnabled(val);
    m_saveAsAction->setEnabled(val);
    d->selectAllAction->setEnabled(val);
    d->selectNoneAction->setEnabled(val);
    d->slideShowAction->setEnabled(val);

    // these actions are special: They are turned off if val is false,
    // but if val is true, they may be turned on or off.
    if (val)
    {
        // Trigger sending of signalUndoStateChanged
        // Note that for saving and loading, this is not necessary
        // because the signal will be sent later anyway.
        m_canvas->updateUndoState();
    }
    else
    {
        m_saveAction->setEnabled(val);
        m_undoAction->setEnabled(val);
        m_redoAction->setEnabled(val);
    }

    QList<ImagePlugin *> pluginList = m_imagePluginLoader->pluginList();

    foreach (ImagePlugin *plugin, pluginList)
    {
        if (plugin)
        {
            plugin->setEnabledActions(val);
        }
    }
}

void EditorWindow::slotToggleFullScreen()
{
    if (m_fullScreen) // out of fullscreen
    {
        setWindowState( windowState() & ~Qt::WindowFullScreen ); // reset

        m_canvas->setBackgroundColor(m_bgColor);

        menuBar()->show();
        statusBar()->show();
        showToolBars();

        if (d->removeFullScreenButton)
        {
            QList<KToolBar *> toolbars = toolBars();
            foreach(KToolBar *toolbar, toolbars)
            {
                // name is set in ui.rc XML file
                if (toolbar->objectName() == "ToolBar")
                {
                    toolbar->removeAction(m_fullScreenAction);
                    break;
                }
            }
        }

        toggleGUI2FullScreen();
        m_fullScreen = false;
    }
    else  // go to fullscreen
    {
        m_canvas->setBackgroundColor(QColor(Qt::black));

        // hide the menubar and the statusbar
        menuBar()->hide();
        statusBar()->hide();

        if (d->fullScreenHideToolBar)
        {
            hideToolBars();
        }
        else
        {
            showToolBars();

            QList<KToolBar *> toolbars = toolBars();
            KToolBar *mainToolbar = 0;
            foreach(KToolBar *toolbar, toolbars)
            {
                if (toolbar->objectName() == "ToolBar")
                {
                    mainToolbar = toolbar;
                    break;
                }
            }

            // add fullscreen action if necessary
            if ( mainToolbar && !mainToolbar->actions().contains(m_fullScreenAction) )
            {
                mainToolbar->addAction(m_fullScreenAction);
                d->removeFullScreenButton=true;
            }
            else
            {
                // If FullScreen button is enabled in toolbar settings,
                // we shall not remove it when leaving of fullscreen mode.
                d->removeFullScreenButton=false;
            }
        }

        toggleGUI2FullScreen();
        setWindowState( windowState() | Qt::WindowFullScreen ); // set
        m_fullScreen = true;
    }
}

void EditorWindow::slotRotatedOrFlipped()
{
    m_rotatedOrFlipped = true;
}

void EditorWindow::slotLoadingProgress(const QString&, float progress)
{
    m_nameLabel->setProgressValue((int)(progress*100.0));
}

void EditorWindow::slotSavingProgress(const QString&, float progress)
{
    m_nameLabel->setProgressValue((int)(progress*100.0));
}

bool EditorWindow::promptForOverWrite()
{

    KUrl destination = saveDestinationUrl();

    if (destination.isLocalFile())
    {

        QFileInfo fi(m_canvas->currentImageFilePath());
        QString warnMsg(i18n("About to overwrite file \"%1\"\nAre you sure?", fi.fileName()));
        return (KMessageBox::warningContinueCancel(this,
                                                   warnMsg,
                                                   i18n("Warning"),
                                                   KGuiItem(i18n("Overwrite")),
                                                   KStandardGuiItem::cancel(),
                                                   QString("editorWindowSaveOverwrite"))
                ==  KMessageBox::Continue);

    }
    else
    {
        // in this case kio handles the overwrite request
        return true;
    }

}

void EditorWindow::slotUndoStateChanged(bool moreUndo, bool moreRedo, bool canSave)
{
    m_revertAction->setEnabled(canSave);
    m_undoAction->setEnabled(moreUndo);
    m_redoAction->setEnabled(moreRedo);
    m_saveAction->setEnabled(hasChangesToSave());

    if (!moreUndo)
        m_rotatedOrFlipped = false;
}

bool EditorWindow::hasChangesToSave()
{
    // virtual, can be extended by subclasses
    return m_canvas->hasChangesToSave();
}

bool EditorWindow::promptUserSave(const KUrl& url, SaveOrSaveAs saveOrSaveAs, bool allowCancel)
{
    if (d->currentWindowModalDialog)
    {
        d->currentWindowModalDialog->reject();
    }

    if (hasChangesToSave())
    {
        // if window is minimized, show it
        if (isMinimized())
        {
            KWindowSystem::unminimizeWindow(winId());
        }

        int result;
        QString boxMessage = i18n("The image '%1' has been modified.\n"
                                  "Do you want to save it?", url.fileName());
        if (allowCancel)
        {
            result = KMessageBox::warningYesNoCancel(this,
                                  boxMessage,
                                  QString(),
                                  KStandardGuiItem::save(),
                                  KStandardGuiItem::discard());
        }
        else
        {
            result = KMessageBox::warningYesNo(this,
                                  boxMessage,
                                  QString(),
                                  KStandardGuiItem::save(),
                                  KStandardGuiItem::discard());
        }

        if (result == KMessageBox::Yes)
        {
            bool saving = false;

            switch (saveOrSaveAs)
            {
                case AskIfNeeded:
                    if (m_canvas->isReadOnly())
                        saving = saveAs();
                    else if (promptForOverWrite())
                        saving = save();
                    break;
                case OverwriteWithoutAsking:
                    if (m_canvas->isReadOnly())
                        saving = saveAs();
                    else
                        saving = save();
                    break;
                case AlwaysSaveAs:
                    saving = saveAs();
                    break;
            }

            // save and saveAs return false if they were canceled and did not enter saving at all
            // In this case, do not call enterWaitingLoop because quitWaitingloop will not be called.
            if (saving)
            {
                // Waiting for asynchronous image file saving operation running in separate thread.
                m_savingContext->synchronizingState = SavingContextContainer::SynchronousSaving;
                enterWaitingLoop();
                m_savingContext->synchronizingState = SavingContextContainer::NormalSaving;
                return m_savingContext->synchronousSavingResult;
            }
            else
            {
                return false;
            }
        }
        else if (result == KMessageBox::No)
        {
            m_saveAction->setEnabled(false);
            return true;
        }
        else
        {
            return false;
        }
    }

    return true;
}

bool EditorWindow::waitForSavingToComplete()
{
    // avoid reentrancy - return false means we have reentered the loop already.
    if (m_savingContext->synchronizingState == SavingContextContainer::SynchronousSaving)
        return false;

    if (m_savingContext->savingState != SavingContextContainer::SavingStateNone)
    {
        // Waiting for asynchronous image file saving operation running in separate thread.
        m_savingContext->synchronizingState = SavingContextContainer::SynchronousSaving;
        KMessageBox::queuedMessageBox(this,
                                      KMessageBox::Information,
                                      i18n("Please wait while the image is being saved..."));
        enterWaitingLoop();
        m_savingContext->synchronizingState = SavingContextContainer::NormalSaving;
    }
    return true;
}

void EditorWindow::enterWaitingLoop()
{
    d->waitingLoop->exec(QEventLoop::ExcludeUserInputEvents);
}

void EditorWindow::quitWaitingLoop()
{
    d->waitingLoop->quit();
}

void EditorWindow::slotSelected(bool val)
{
    // Update menu actions.
    d->cropAction->setEnabled(val);
    d->zoomFitToSelectAction->setEnabled(val);
    d->copyAction->setEnabled(val);

    QList<ImagePlugin*> pluginList = m_imagePluginLoader->pluginList();
    foreach (ImagePlugin *plugin, pluginList)
    {
        if (plugin)
        {
            plugin->setEnabledSelectionActions(val);
        }
    }

    QRect sel = m_canvas->getSelectedArea();
    // Update histogram into sidebar.
    emit signalSelectionChanged(sel);

    // Update status bar
    if (val)
        setToolInfoMessage(QString("(%1, %2) (%3 x %4)").arg(sel.x()).arg(sel.y()).arg(sel.width()).arg(sel.height()));
    else
        setToolInfoMessage(i18n("No selection"));
}

void EditorWindow::hideToolBars()
{
    QList<KToolBar *> toolbars = toolBars();
    foreach(KToolBar *toolbar, toolbars)
    {
        toolbar->hide();
    }
}

void EditorWindow::showToolBars()
{
    QList<KToolBar *> toolbars = toolBars();
    foreach(KToolBar *toolbar, toolbars)
    {
        toolbar->show();
    }
}

void EditorWindow::slotPrepareToLoad()
{
    // Disable actions as appropriate during loading
    emit signalNoCurrentItem();
    unsetCursor();
    m_animLogo->stop();
    toggleActions(false);
    slotUpdateItemInfo();
}

void EditorWindow::slotLoadingStarted(const QString& /*filename*/)
{
    setCursor(Qt::WaitCursor);
    m_animLogo->start();
    m_nameLabel->progressBarMode(StatusProgressBar::ProgressBarMode, i18n("Loading: "));
}

void EditorWindow::slotLoadingFinished(const QString& /*filename*/, bool success)
{
    m_nameLabel->progressBarMode(StatusProgressBar::TextMode);
    slotUpdateItemInfo();

    // Enable actions as appropriate after loading
    // No need to re-enable image properties sidebar here, it's will be done
    // automatically by a signal from canvas
    toggleActions(success);
    unsetCursor();
    m_animLogo->stop();

    if (success)
        colorManage();
}

void EditorWindow::colorManage()
{
    if (!d->ICCSettings->enableCM)
        return;

    DImg image = m_canvas->currentImage();
    if (image.isNull())
        return;

    if (!IccManager::needsPostLoadingManagement(image))
        return;

    IccManager manager(image, m_canvas->currentImageFilePath());
    if (!manager.hasValidWorkspace())
    {
        QString message = i18n("Cannot open the specified working space profile (\"%1\"). "
                               "No color transformation will be applied. "
                               "Please check the color management "
                               "configuration in digiKam's setup.", d->ICCSettings->workspaceProfile);
        KMessageBox::information(this, message);
    }

    // Show dialog and get transform from user choice
    IccTransform trans = manager.postLoadingManage(this);
    // apply transform in thread.
    // Do _not_ test for willHaveEffect() here - there are more side effects when calling this method
    m_canvas->applyTransform(trans);
    slotUpdateItemInfo();
}

void EditorWindow::slotNameLabelCancelButtonPressed()
{
    // If we saving an image...
    if (m_savingContext->savingState != SavingContextContainer::SavingStateNone)
    {
        m_savingContext->abortingSaving = true;
        m_canvas->abortSaving();
    }

    // If we preparing SlideShow...
    m_cancelSlideShow = true;
}

void EditorWindow::slotSave()
{
    if (m_canvas->isReadOnly())
        saveAs();
    else if (promptForOverWrite())
        save();
}

void EditorWindow::slotSavingStarted(const QString& /*filename*/)
{
    setCursor(Qt::WaitCursor);
    m_animLogo->start();

    // Disable actions as appropriate during saving
    emit signalNoCurrentItem();
    toggleActions(false);

    m_nameLabel->progressBarMode(StatusProgressBar::CancelProgressBarMode, i18n("Saving: "));
}

void EditorWindow::movingSaveFileFinished(bool successful)
{
    if (!successful)
    {
        finishSaving(false);
        return;
    }

    m_canvas->setUndoHistoryOrigin();

    // remove image from cache since it has changed
    LoadingCacheInterface::fileChanged(m_savingContext->destinationURL.toLocalFile());

    // restore state of disabled actions. saveIsComplete can start any other task
    // (loading!) which might itself in turn change states
    finishSaving(true);

    if (m_savingContext->executedOperation == SavingContextContainer::SavingStateSave)
        saveIsComplete();
    else
        saveAsIsComplete();

    // Take all actions necessary to update information and re-enable sidebar
    slotChanged();
}

void EditorWindow::slotSavingFinished(const QString& filename, bool success)
{
    Q_UNUSED(filename);

    // only handle this if we really wanted to save a file...
    if ((m_savingContext->savingState == SavingContextContainer::SavingStateSave) ||
        (m_savingContext->savingState == SavingContextContainer::SavingStateSaveAs))
    {
        // from save()
        m_savingContext->executedOperation = m_savingContext->savingState;
        m_savingContext->savingState = SavingContextContainer::SavingStateNone;

        if (!success)
        {
            if (!m_savingContext->abortingSaving)
            {
                KMessageBox::error(this, i18n("Failed to save file\n\"%1\"\nto\n\"%2\".",
                                              m_savingContext->destinationURL.fileName(),
                                              m_savingContext->destinationURL.toLocalFile()));
            }
            finishSaving(false);
            return;
        }

        moveFile();

    }
    else
    {
        kWarning() << "Why was slotSavingFinished called "
                                  << "if we did not want to save a file?";
    }
}

void EditorWindow::finishSaving(bool success)
{
    m_savingContext->synchronousSavingResult = success;

    if (m_savingContext->saveTempFile)
    {
        delete m_savingContext->saveTempFile;
        m_savingContext->saveTempFile = 0;
    }

    // Exit of internal Qt event loop to unlock promptUserSave() method.
    if (m_savingContext->synchronizingState == SavingContextContainer::SynchronousSaving)
        quitWaitingLoop();

    // Enable actions as appropriate after saving
    toggleActions(true);
    unsetCursor();
    m_animLogo->stop();

    m_nameLabel->progressBarMode(StatusProgressBar::TextMode);

    // On error, continue using current image
    if (!success)
    {
        m_canvas->switchToLastSaved(m_savingContext->srcURL.toLocalFile());
    }
}

void EditorWindow::setupTempSaveFile(const KUrl & url)
{
#ifdef _WIN32
    KUrl parent(url.directory(KUrl::AppendTrailingSlash));
    QString tempDir = parent.toLocalFile();
#else
    QString tempDir = url.directory(KUrl::AppendTrailingSlash);
#endif

    // use magic file extension which tells the digikamalbums ioslave to ignore the file
    m_savingContext->saveTempFile = new KTemporaryFile();
    // if the destination url is on local file system, try to set the temp file
    // location to the destination folder, otherwise use a local default
    if (url.isLocalFile())
    {
        m_savingContext->saveTempFile->setPrefix(tempDir);
    }
    m_savingContext->saveTempFile->setSuffix(".digikamtempfile.tmp");
    m_savingContext->saveTempFile->setAutoRemove(false);
    m_savingContext->saveTempFile->open();

    if (!m_savingContext->saveTempFile->open())
    {
        KMessageBox::error(this, i18n("Could not open a temporary file in the folder \"%1\": %2 (%3)",
                                      tempDir, m_savingContext->saveTempFile->errorString(),
                                      m_savingContext->saveTempFile->error()));
        return;
    }

    m_savingContext->saveTempFileName = m_savingContext->saveTempFile->fileName();
    delete m_savingContext->saveTempFile;
    m_savingContext->saveTempFile = 0;
}

void EditorWindow::startingSave(const KUrl& url)
{
    kDebug() << "startSaving url = " << url;

    // avoid any reentrancy. Should be impossible anyway since actions will be disabled.
    if (m_savingContext->savingState != SavingContextContainer::SavingStateNone)
        return;

    if (!checkPermissions(url))
        return;

    setupTempSaveFile(url);

    m_savingContext->srcURL             = url;
    m_savingContext->destinationURL     = m_savingContext->srcURL;
    m_savingContext->destinationExisted = true;
    m_savingContext->originalFormat     = m_canvas->currentImageFileFormat();
    m_savingContext->format             = m_savingContext->originalFormat;
    m_savingContext->abortingSaving     = false;
    m_savingContext->savingState        = SavingContextContainer::SavingStateSave;
    m_savingContext->executedOperation  = SavingContextContainer::SavingStateNone;

    m_canvas->saveAs(m_savingContext->saveTempFileName, m_IOFileSettings,
                     m_setExifOrientationTag && (m_rotatedOrFlipped || m_canvas->exifRotated()));
}

QStringList EditorWindow::getWritingFilters()
{
    // begin with the filters KImageIO supports
    QString pattern             = KImageIO::pattern(KImageIO::Writing);
    QStringList writablePattern = pattern.split(QChar('\n'));
    kDebug() << "KImageIO offered pattern: " << writablePattern;

    // append custom file types
    writablePattern.append(QString("*.jp2|") + i18n("JPEG 2000 image"));
    writablePattern.append(QString("*.pgf|") + i18n("Progressive Graphics File"));

    return writablePattern;
}

QString EditorWindow::findFilterByExtension(const QStringList& allFilters, const QString& extension)
{
    kDebug() << "Searching for a filter with extension '" << extension
             << "' in: " << allFilters;

    const QString filterExtension = QString("*.%1").arg(extension.toLower());

    foreach(const QString& filter, allFilters)
    {

        if (filter.contains(filterExtension))
        {
            kDebug() << "Found filter '" << filter << "'";
            return filter;
        }

    }

    // fall back to "all image types"
    if (!allFilters.empty() && allFilters.first().contains(filterExtension))
    {
        kDebug() << "using fall back all images filter: " << allFilters.first();
        return allFilters.first();
    }

    return QString();
}

QString EditorWindow::getExtensionFromFilter(const QString& filter)
{
    kDebug () << "Trying to extract format from filter: " << filter;

    // find locations of interesting characters in the filter string
    const int asteriskLocation = filter.indexOf('*');
    if (asteriskLocation < 0)
    {
        kDebug() << "Could not find a * in the filter";
        return QString();
    }

    int endLocation = filter.indexOf(QRegExp("[|\\* ]"), asteriskLocation + 1);
    if (endLocation < 0)
    {
        endLocation = filter.length();
    }

    kDebug() << "astriskLocation = " << asteriskLocation
             << ", endLocation = " << endLocation;

    // extract extension with the locations found above
    QString formatString = filter;
    formatString.remove(0, asteriskLocation + 2);
    formatString = formatString.left(endLocation - asteriskLocation - 2);
    kDebug() << "Extracted format " << formatString;
    return formatString;
}

bool EditorWindow::selectValidSavingFormat(const QString& filter,
                                           const KUrl& targetUrl, const QString &autoFilter)
{
    kDebug() << "Trying to find a saving format with filter = "
             << filter << ", targetUrl = " << targetUrl;

    // build a list of valid types
    QStringList validTypes = KImageIO::types(KImageIO::Writing);
    kDebug() << "KDE Offered types: " << validTypes;

    validTypes << "TIF";
    validTypes << "TIFF";
    validTypes << "JPG";
    validTypes << "JPEG";
    validTypes << "JPE";
    validTypes << "J2K";
    validTypes << "JP2";
    validTypes << "PGF";

    kDebug() << "Writable formats: " << validTypes;

    // if the auto filter is used, use the format provided in the filename
    if (filter == autoFilter)
    {
        QString suffix;
        if (targetUrl.isLocalFile())
        {
            // for local files QFileInfo can be used
            QFileInfo fi(targetUrl.toLocalFile());
            suffix = fi.suffix();
            kDebug() << "Possible format from local file: " << suffix;
        }
        else
        {
            // for remote files string manipulation is needed unfortunately
            QString fileName = targetUrl.fileName();
            const int periodLocation = fileName.lastIndexOf('.');
            if (periodLocation >= 0)
            {
                suffix = fileName.right(fileName.size() - periodLocation - 1);
            }
            kDebug() << "Possible format from remote file: " << suffix;
        }
        if (!suffix.isEmpty() && validTypes.contains(suffix, Qt::CaseInsensitive))
        {
            kDebug() << "Using format from target url " << suffix;
            m_savingContext->format = suffix;
            return true;
        }
    }
    else
    {
        // use extension from the filter

        QString filterExtension = getExtensionFromFilter(filter);
        if (!filterExtension.isEmpty() &&
            validTypes.contains(filterExtension, Qt::CaseInsensitive))
        {
            kDebug() << "Using format from filter extension: " << filterExtension;
            m_savingContext->format = filterExtension;
            return true;
        }
    }

    // another way to determine the format is to use the original file
    {
        QString originalFormat(QImageReader::imageFormat(
                        m_savingContext->srcURL.toLocalFile()));
        if (validTypes.contains(originalFormat, Qt::CaseInsensitive))
        {
            kDebug() << "Using format from original file: " << originalFormat;
            m_savingContext->format = originalFormat;
            return true;
        }
    }

    kDebug() << "No suitable format found";

    return false;
}

bool EditorWindow::startingSaveAs(const KUrl& url)
{
    kDebug() << "startSavingAs called";

    if (m_savingContext->savingState != SavingContextContainer::SavingStateNone)
        return false;

    m_savingContext->srcURL = url;

    // prepare the save dialog
    FileSaveOptionsBox *options      = new FileSaveOptionsBox();
    QPointer<KFileDialog> imageFileSaveDialog
                                     = new KFileDialog(m_savingContext->srcURL.isLocalFile() ?
                                                       m_savingContext->srcURL : KUrl(QDir::homePath()),
                                                       QString(),
                                                       this,
                                                       options);
    options->setDialog(imageFileSaveDialog);

    ImageDialogPreview *preview = new ImageDialogPreview(imageFileSaveDialog);
    imageFileSaveDialog->setPreviewWidget(preview);
    imageFileSaveDialog->setOperationMode(KFileDialog::Saving);
    imageFileSaveDialog->setMode(KFile::File);
    imageFileSaveDialog->setCaption(i18n("New Image File Name"));

    // restore old settings for the dialog
    QFileInfo info(m_savingContext->srcURL.fileName());
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(CONFIG_GROUP_NAME);
    const QString optionLastExtension = "LastSavedImageExtension";
    QString ext               = group.readEntry(optionLastExtension, "png");
    if (ext.isEmpty())
    {
        ext = "png";
    }
    QString fileName          = info.completeBaseName() + QString(".") + ext;

    // Determine the default filter from LastSavedImageTypeMime
    QStringList writablePattern = getWritingFilters();
    imageFileSaveDialog->setFilter(writablePattern.join(QChar('\n')));

    // find the correct spelling of the auto filter
    // XXX bad assumption that the "all images" filter is always the first
    imageFileSaveDialog->filterWidget()->setCurrentFilter(writablePattern.at(0));
    QString autoFilter = imageFileSaveDialog->filterWidget()->currentFilter();
    options->setAutoFilter(autoFilter);

    imageFileSaveDialog->setSelection(fileName);

    // Start dialog and check if canceled.
    int result;
    if (d->currentWindowModalDialog)
    {
        // go application-modal - we will create utter confusion if descending into more than one window-modal dialog
        imageFileSaveDialog->setModal(true);
        result = imageFileSaveDialog->exec();
    }
    else
    {
        imageFileSaveDialog->setWindowModality(Qt::WindowModal);
        d->currentWindowModalDialog = imageFileSaveDialog;
        result = imageFileSaveDialog->exec();
        d->currentWindowModalDialog = 0;
    }
    if (result != KFileDialog::Accepted || !imageFileSaveDialog)
    {
       return false;
    }

    KUrl newURL = imageFileSaveDialog->selectedUrl();
    kDebug() << "Writing file to " << newURL;

#ifdef _WIN32
    //-- Show Settings Dialog ----------------------------------------------

    const QString configShowImageSettingsDialog="ShowImageSettingsDialog";
    bool showDialog = group.readEntry(configShowImageSettingsDialog, true);
    if (showDialog && options->discoverFormat(newURL.fileName(), DImg::NONE)!=DImg::NONE) {
        FileSaveOptionsDlg *fileSaveOptionsDialog   = new FileSaveOptionsDlg(this, options);
        options->slotImageFileFormatChanged(newURL.fileName());

        if (d->currentWindowModalDialog)
        {
            // go application-modal - we will create utter confusion if descending into more than one window-modal dialog
            fileSaveOptionsDialog->setModal(true);
            result = fileSaveOptionsDialog->exec();
        }
        else
        {
            fileSaveOptionsDialog->setWindowModality(Qt::WindowModal);
            d->currentWindowModalDialog = fileSaveOptionsDialog;
            result = fileSaveOptionsDialog->exec();
            d->currentWindowModalDialog = 0;
        }
        if (result != KFileDialog::Accepted || !fileSaveOptionsDialog)
        {
            return false;
        }
    }
#endif

    // Update file save settings in editor instance.
    options->applySettings();
    applyStandardSettings();

    // select the format to save the image with
    bool validFormatSet = selectValidSavingFormat(imageFileSaveDialog->currentFilter(), newURL, autoFilter);

    if (!validFormatSet)
    {
        KMessageBox::error(this, i18n("Unable to determine the format to save the target image with."));
        return false;
    }

    if (!newURL.isValid())
    {
        KMessageBox::error(this, i18n("Failed to save file\n\"%1\"\nto\n\"%2\".",
                                      info.completeBaseName(),
                                      newURL.prettyUrl()));
        kWarning() << "target URL is not valid !";
        return false;
    }

    group.writeEntry(optionLastExtension, m_savingContext->format);
    config->sync();

    // if new and original URL are equal use slotSave() ------------------------------

    KUrl currURL(m_savingContext->srcURL);
    currURL.cleanPath();
    newURL.cleanPath();

    if (currURL.equals(newURL))
    {
        slotSave();
        return false;
    }

    // Check for overwrite ----------------------------------------------------------

    QFileInfo fi(newURL.toLocalFile());
    m_savingContext->destinationExisted = fi.exists();
    if ( m_savingContext->destinationExisted )
    {
        int result =

            KMessageBox::warningYesNo( this, i18n("A file named \"%1\" already "
                                                  "exists. Are you sure you want "
                                                  "to overwrite it?",
                                                  newURL.fileName()),
                                       i18n("Overwrite File?"),
                                       KStandardGuiItem::overwrite(),
                                       KStandardGuiItem::cancel() );

        if (result != KMessageBox::Yes)
            return false;

        // There will be two message boxes if the file is not writable.
        // This may be controversial, and it may be changed, but it was a deliberate decision.
        if (!checkPermissions(newURL))
            return false;
    }

    // Now do the actual saving -----------------------------------------------------

    setupTempSaveFile(newURL);

    m_savingContext->destinationURL = newURL;
    m_savingContext->originalFormat = m_canvas->currentImageFileFormat();
    m_savingContext->savingState    = SavingContextContainer::SavingStateSaveAs;
    m_savingContext->executedOperation = SavingContextContainer::SavingStateNone;
    m_savingContext->abortingSaving = false;

    m_canvas->saveAs(m_savingContext->saveTempFileName, m_IOFileSettings,
                     m_setExifOrientationTag && (m_rotatedOrFlipped || m_canvas->exifRotated()),
                     m_savingContext->format.toLower());

    return true;
}

bool EditorWindow::checkPermissions(const KUrl& url)
{
    //TODO: Check that the permissions can actually be changed
    //      if write permissions are not available.

    QFileInfo fi(url.toLocalFile());

    if (fi.exists() && !fi.isWritable())
    {
       int result =

            KMessageBox::warningYesNo( this, i18n("You do not have write permissions "
                                                  "for the file named \"%1\". "
                                                  "Are you sure you want "
                                                  "to overwrite it?",
                                                  url.fileName()),
                                       i18n("Overwrite File?"),
                                       KStandardGuiItem::overwrite(),
                                       KStandardGuiItem::cancel() );

        if (result != KMessageBox::Yes)
            return false;
    }

    return true;
}

void EditorWindow::moveFile()
{
    // how to move a file depends on if the file is on a local system or not.
    if (m_savingContext->destinationURL.isLocalFile())
    {
        kDebug() << "moving a local file";

        QByteArray dstFileName = QFile::encodeName(m_savingContext->destinationURL.toLocalFile());
#ifndef _WIN32
        // Store old permissions:
        // Just get the current umask.
        mode_t curr_umask = umask(S_IREAD | S_IWRITE);
        // Restore the umask.
        umask(curr_umask);

        // For new files respect the umask setting.
        mode_t filePermissions = (S_IREAD | S_IWRITE | S_IROTH | S_IWOTH | S_IRGRP | S_IWGRP) & ~curr_umask;

        // For existing files, use the mode of the original file.
        if (m_savingContext->destinationExisted)
        {
            struct stat stbuf;
            if (::stat(dstFileName, &stbuf) == 0)
            {
                filePermissions = stbuf.st_mode;
            }
        }
#endif
        // rename tmp file to dest
        int ret;
#if KDE_IS_VERSION(4,2,85)
        // KDE 4.3.0
        // KDE::rename() takes care of QString -> bytestring encoding
        ret = KDE::rename(m_savingContext->saveTempFileName,
                           m_savingContext->destinationURL.toLocalFile());
#else
        // KDE 4.2.x or 4.1.x
        ret = KDE_rename(QFile::encodeName(m_savingContext->saveTempFileName),
                          dstFileName);
#endif
        if (ret != 0)
        {
            KMessageBox::error(this, i18n("Failed to overwrite original file"),
                               i18n("Error Saving File"));
            movingSaveFileFinished(false);
            return;
        }

#ifndef _WIN32
        // restore permissions
        if (::chmod(dstFileName, filePermissions) != 0)
        {
            kWarning() << "Failed to restore file permissions for file " << dstFileName;
        }
#endif
        movingSaveFileFinished(true);
        return;
    }
    else
    {
        // for remote destinations use kio to move the temp file over there

        kDebug() << "moving a remote file via KIO";

        KIO::CopyJob *moveJob = KIO::move(KUrl(
                        m_savingContext->saveTempFileName),
                        m_savingContext->destinationURL);
        connect(moveJob, SIGNAL(result(KJob*)),
                this, SLOT(slotKioMoveFinished(KJob*)));
    }
}

void EditorWindow::slotKioMoveFinished(KJob *job)
{
    if (job->error())
    {
        KMessageBox::error(this, i18n("Failed to save file: %1", job->errorString()),
                           i18n("Error Saving File"));
    }

    movingSaveFileFinished(!job->error());
}

void EditorWindow::slotColorManagementOptionsChanged()
{
    *d->ICCSettings = IccSettings::instance()->settings();

    d->viewCMViewAction->blockSignals(true);

    d->viewCMViewAction->setEnabled(d->ICCSettings->enableCM);
    d->viewCMViewAction->setChecked(d->ICCSettings->useManagedView);
    setColorManagedViewIndicatorToolTip(d->ICCSettings->enableCM, d->ICCSettings->useManagedView);

    d->viewSoftProofAction->setEnabled(d->ICCSettings->enableCM &&
                                       !d->ICCSettings->defaultProofProfile.isEmpty());
    d->softProofOptionsAction->setEnabled(d->ICCSettings->enableCM);

    d->toolIface->updateICCSettings();
    d->viewCMViewAction->blockSignals(false);
}

void EditorWindow::slotToggleColorManagedView()
{
    d->viewCMViewAction->blockSignals(true);
    bool cmv = false;
    if (d->ICCSettings->enableCM)
    {
        cmv = !d->ICCSettings->useManagedView;
        d->ICCSettings->useManagedView = cmv;
        d->toolIface->updateICCSettings();
        IccSettings::instance()->setUseManagedView(cmv);
    }

    d->viewCMViewAction->setChecked(cmv);
    setColorManagedViewIndicatorToolTip(d->ICCSettings->enableCM, cmv);
    d->viewCMViewAction->blockSignals(false);
}

void EditorWindow::setColorManagedViewIndicatorToolTip(bool available, bool cmv)
{
    QString tooltip;
    if (available)
    {
        if (cmv)
            tooltip = i18n("Color-Managed View is enabled.");
        else
            tooltip = i18n("Color-Managed View is disabled.");
    }
    else
    {
        tooltip = i18n("Color Management is not configured, so the Color-Managed View is not available.");
    }
    d->cmViewIndicator->setToolTip(tooltip);
}

void EditorWindow::slotSoftProofingOptions()
{
    // Adjusts global settings
    QPointer<SoftProofDialog> dlg = new SoftProofDialog(this);
    dlg->exec();

    d->viewSoftProofAction->setChecked(dlg->shallEnableSoftProofView());
    slotUpdateSoftProofingState();
    delete dlg;
}

void EditorWindow::slotUpdateSoftProofingState()
{
    bool on = d->viewSoftProofAction->isChecked();
    m_canvas->setSoftProofingEnabled(on);
}

void EditorWindow::slotSetUnderExposureIndicator(bool on)
{
    d->exposureSettings->underExposureIndicator = on;
    d->toolIface->updateExposureSettings();
    d->viewUnderExpoAction->setChecked(on);
    setUnderExposureToolTip(on);
}

void EditorWindow::setUnderExposureToolTip(bool on)
{
    d->underExposureIndicator->setToolTip(
                   on ? i18n("Under-Exposure indicator is enabled")
                      : i18n("Under-Exposure indicator is disabled"));
}

void EditorWindow::slotSetOverExposureIndicator(bool on)
{
    d->exposureSettings->overExposureIndicator = on;
    d->toolIface->updateExposureSettings();
    d->viewOverExpoAction->setChecked(on);
    setOverExposureToolTip(on);
}

void EditorWindow::setOverExposureToolTip(bool on)
{
    d->overExposureIndicator->setToolTip(
                   on ? i18n("Over-Exposure indicator is enabled")
                      : i18n("Over-Exposure indicator is disabled"));
}

void EditorWindow::slotDonateMoney()
{
    KToolInvocation::invokeBrowser("http://www.digikam.org/?q=donation");
}

void EditorWindow::slotContribute()
{
    KToolInvocation::invokeBrowser("http://www.digikam.org/?q=contrib");
}

void EditorWindow::slotToggleSlideShow()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(CONFIG_GROUP_NAME);
    bool startWithCurrent = group.readEntry(d->configSlideShowStartCurrentEntry, false);

    SlideShowSettings settings;
    settings.delay                = group.readEntry(d->configSlideShowDelayEntry, 5) * 1000;
    settings.printName            = group.readEntry(d->configSlideShowPrintNameEntry, true);
    settings.printDate            = group.readEntry(d->configSlideShowPrintDateEntry, false);
    settings.printApertureFocal   = group.readEntry(d->configSlideShowPrintApertureFocalEntry, false);
    settings.printExpoSensitivity = group.readEntry(d->configSlideShowPrintExpoSensitivityEntry, false);
    settings.printMakeModel       = group.readEntry(d->configSlideShowPrintMakeModelEntry, false);
    settings.printComment         = group.readEntry(d->configSlideShowPrintCommentEntry, false);
    settings.printRating          = group.readEntry(d->configSlideShowPrintRatingEntry, false);
    settings.loop                 = group.readEntry(d->configSlideShowLoopEntry, false);
    slideShow(startWithCurrent, settings);
}

void EditorWindow::slotSelectionChanged(const QRect& sel)
{
    setToolInfoMessage(QString("(%1, %2) (%3 x %4)").arg(sel.x()).arg(sel.y()).arg(sel.width()).arg(sel.height()));
}

void EditorWindow::slotRawCameraList()
{
    RawCameraDlg *dlg = new RawCameraDlg(kapp->activeWindow());
    dlg->show();
}

void EditorWindow::slotThemeChanged()
{
    QStringList themes(ThemeEngine::instance()->themeNames());
    int index = themes.indexOf(ThemeEngine::instance()->getCurrentThemeName());
    if (index == -1)
        index = themes.indexOf(i18n("Default"));

    m_themeMenuAction->setCurrentItem(index);

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(CONFIG_GROUP_NAME);

    if (!group.readEntry(d->configUseThemeBackgroundColorEntry, true))
        m_bgColor = group.readEntry(d->configBackgroundColorEntry, QColor(Qt::black));
    else
        m_bgColor = ThemeEngine::instance()->baseColor();

    m_canvas->setBackgroundColor(m_bgColor);
}

void EditorWindow::slotChangeTheme(const QString& theme)
{
    ThemeEngine::instance()->slotChangeTheme(theme);
}

void EditorWindow::toggleGUI2FullScreen()
{
    if (m_fullScreen)
    {
        rightSideBar()->restore(QList<QWidget*>() << thumbBar(), d->fullscreenSizeBackup);

        if (m_fullScreenHideThumbBar)
            thumbBar()->restoreVisibility();
    }
    else
    {
        // See bug #166472, a simple backup()/restore() will hide non-sidebar splitter child widgets
        // in horizontal mode thumbbar wont be member of the splitter, it is just ignored then
        rightSideBar()->backup(QList<QWidget*>() << thumbBar(), &d->fullscreenSizeBackup);

        if (m_fullScreenHideThumbBar)
                thumbBar()->hide();
    }
}

void EditorWindow::slotComponentsInfo()
{
    LibsInfoDlg *dlg = new LibsInfoDlg(this);
    dlg->show();
}

void EditorWindow::setToolStartProgress(const QString& toolName)
{
    m_animLogo->start();
    m_nameLabel->setProgressValue(0);
    m_nameLabel->progressBarMode(StatusProgressBar::CancelProgressBarMode, QString("%1: ").arg(toolName));
}

void EditorWindow::setToolProgress(int progress)
{
    m_nameLabel->setProgressValue(progress);
}

void EditorWindow::setToolStopProgress()
{
    m_animLogo->stop();
    m_nameLabel->setProgressValue(0);
    m_nameLabel->progressBarMode(StatusProgressBar::TextMode);
    slotUpdateItemInfo();
}

void EditorWindow::slotShowMenuBar()
{
    const bool visible = menuBar()->isVisible();
    menuBar()->setVisible(!visible);
}

void EditorWindow::slotCloseTool()
{
    if (d->toolIface)
        d->toolIface->slotCloseTool();
}

void EditorWindow::setPreviewModeMask(int mask)
{
    d->previewToolBar->setPreviewModeMask(mask);
}

PreviewToolBar::PreviewMode EditorWindow::previewMode()
{
    return d->previewToolBar->previewMode();
}

void EditorWindow::setToolInfoMessage(const QString& txt)
{
    d->infoLabel->setText(txt);
}

}  // namespace Digikam
