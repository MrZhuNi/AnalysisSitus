//-----------------------------------------------------------------------------
// Created on: 07 December 2015
//-----------------------------------------------------------------------------
// Copyright (c) 2017 Sergey Slyadnev
// Code covered by the MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//-----------------------------------------------------------------------------

// Own include
#include <exe_MainWindow.h>

// exe includes
#include <exe_CommonFacilities.h>
#include <exe_Version.h>

// asiUI includes
#include <asiUI_StatusBar.h>
#include <asiUI_StatusBarImpl.h>

// asiTcl includes
#include <asiTcl_Plugin.h>

// Qt includes
#pragma warning(push, 0)
#include <QApplication>
#include <QTextStream>
#include <QDesktopWidget>
#include <QDockWidget>
#pragma warning(pop)

#define EXE_LOAD_MODULE(name) \
{ \
  Handle(exe_CommonFacilities) cf = exe_CommonFacilities::Instance();\
  \
  if ( !asiTcl_Plugin::Load(cf->Interp, cf, name) ) \
    cf->ProgressNotifier->SendLogMessage(LogErr(Normal) << "Cannot load %1 commands." << name); \
  else \
    cf->ProgressNotifier->SendLogMessage(LogInfo(Normal) << "Loaded %1 commands." << name); \
}

//-----------------------------------------------------------------------------

//! Constructor.
exe_MainWindow::exe_MainWindow() : QMainWindow()
{
  this->createPartViewer();
  this->createDockWindows();
  this->setCentralWidget(m_widgets.wViewerPart);

  // Prepare application name with the version number
  TCollection_AsciiString appName(ASITUS_APP_NAME);
  appName += " ["; appName += ASITUS_VERSION_STRING; appName += "]";
  //
  this->setWindowTitle( appName.ToCString() );

  //---------------------------------------------------------------------------
  // Apply fantastic dark theme
  //---------------------------------------------------------------------------

  QFile f(":qdarkstyle/style.qss");
  if ( !f.exists() )
  {
    printf("Unable to set stylesheet, file not found\n");
  }
  else
  {
    f.open(QFile::ReadOnly | QFile::Text);
    QTextStream ts(&f);
    qApp->setStyleSheet( ts.readAll() );
  }

  //---------------------------------------------------------------------------
  // Set focus on Tcl console
  //---------------------------------------------------------------------------

  m_widgets.wConsole->setFocus();
}

//-----------------------------------------------------------------------------

//! Destructor.
exe_MainWindow::~exe_MainWindow()
{}

//-----------------------------------------------------------------------------

//! Gets control on window close.
//! \param evt [in] event.
void exe_MainWindow::closeEvent(QCloseEvent* evt)
{
  // It seems that we have to destruct objects properly and manually in
  // order to avoid some side effects from VTK. E.g. if we don't kill the
  // widgets explicitly here, we may sometimes get a warning window of VTK
  // saying that it lacks some resources
  m_widgets.Release();
  //
  evt->accept();
}

//-----------------------------------------------------------------------------

//! Creates main (part) viewer.
void exe_MainWindow::createPartViewer()
{
  // Common facilities instance
  Handle(exe_CommonFacilities) cf = exe_CommonFacilities::Instance();

  // Create viewer
  m_widgets.wViewerPart = new asiUI_ViewerPart(cf->Model, true);

  // Initialize desktop
  cf->ViewerPart = m_widgets.wViewerPart;
  cf->Prs.Part   = m_widgets.wViewerPart->PrsMgr();
}

//-----------------------------------------------------------------------------

//! Creates main dockable widgets.
void exe_MainWindow::createDockWindows()
{
  // Common facilities instance
  Handle(exe_CommonFacilities) cf = exe_CommonFacilities::Instance();

  // Object browser
  QDockWidget* pDockBrowser;
  {
    pDockBrowser = new QDockWidget("Data", this);
    pDockBrowser->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    //
    m_widgets.wBrowser = new asiUI_ObjectBrowser(cf->Model, pDockBrowser);
    m_widgets.wBrowser->AddAssociatedViewer(cf->ViewerPart);
    //
    pDockBrowser->setWidget(m_widgets.wBrowser);
    //
    this->addDockWidget(Qt::LeftDockWidgetArea, pDockBrowser);

    // Initialize desktop
    cf->ObjectBrowser = m_widgets.wBrowser;
  }

  // Domain viewer
  {
    QDockWidget* pDock = new QDockWidget("Domain", this);
    pDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    //
    m_widgets.wViewerDomain = new asiUI_ViewerDomain(cf->Model, pDock);
    pDock->setWidget(m_widgets.wViewerDomain);
    //
    this->addDockWidget(Qt::RightDockWidgetArea, pDock);

    // Initialize desktop
    cf->ViewerDomain = m_widgets.wViewerDomain;
    cf->Prs.Domain   = m_widgets.wViewerDomain->PrsMgr();
  }

  // Host viewer
  {
    QDockWidget* pDock = new QDockWidget("Host", this);
    pDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    //
    m_widgets.wViewerSurface = new asiUI_ViewerHost(cf->Model, pDock);
    pDock->setWidget(m_widgets.wViewerSurface);
    //
    this->addDockWidget(Qt::RightDockWidgetArea, pDock);

    // Initialize desktop
    cf->ViewerHost = m_widgets.wViewerSurface;
    cf->Prs.Host   = m_widgets.wViewerSurface->PrsMgr();
  }

  // Now we have everything to initialize an imperative plotter
  cf->Plotter = new asiUI_JournalIV(cf->Model, cf->Prs.Part, cf->Prs.Domain, cf->ObjectBrowser);

  // Feature controls
  QDockWidget* pDockFeature;
  {
    pDockFeature = new QDockWidget("Features", this);
    pDockFeature->setAllowedAreas(Qt::LeftDockWidgetArea);
    //
    m_widgets.wControlsFeature = new asiUI_ControlsFeature(cf->Model,
                                                           cf->ViewerPart,
                                                           cf->ProgressNotifier,
                                                           cf->Plotter,
                                                           pDockFeature);
    //
    pDockFeature->setWidget(m_widgets.wControlsFeature);
    //
    this->addDockWidget(Qt::LeftDockWidgetArea, pDockFeature);
  }

  // Tabify widgets
  this->tabifyDockWidget(pDockBrowser, pDockFeature);

  // Part controls
  QDockWidget* pDockPart;
  {
    pDockPart = new QDockWidget("Part", this);
    pDockPart->setAllowedAreas(Qt::LeftDockWidgetArea);
    //
    m_widgets.wControlsPart = new asiUI_ControlsPart(cf->Model,
                                                     cf->ViewerPart,
                                                     cf->ProgressNotifier,
                                                     cf->Plotter,
                                                     pDockPart);
    //
    pDockPart->setWidget(m_widgets.wControlsPart);
    //
    this->addDockWidget(Qt::LeftDockWidgetArea, pDockPart);
  }

  // Tabify widgets
  this->tabifyDockWidget(pDockFeature, pDockPart);

  // Listener for part controls
  m_listeners.pControlsPart = new asiUI_ControlsPartListener(m_widgets.wControlsPart,
                                                             m_widgets.wViewerPart,
                                                             m_widgets.wViewerDomain,
                                                             m_widgets.wViewerSurface,
                                                             cf->Model,
                                                             cf->ProgressNotifier);

  // Listener for part viewer
  m_listeners.pViewerPart = new asiUI_ViewerPartListener(m_widgets.wViewerPart,
                                                         m_widgets.wViewerDomain,
                                                         m_widgets.wViewerSurface,
                                                         cf->Model,
                                                         cf->ProgressNotifier,
                                                         cf->Plotter);

  // Signals-slots
  m_listeners.pControlsPart->Connect();
  m_listeners.pViewerPart->Connect();

  // Log window
  QDockWidget* pDockLogWindow;
  {
    pDockLogWindow = new QDockWidget("Logger", this);
    pDockLogWindow->setAllowedAreas(Qt::BottomDockWidgetArea);
    //
    m_widgets.wLogger = new asiUI_StyledTextEdit(pDockLogWindow);
    //
    pDockLogWindow->setWidget(m_widgets.wLogger);
    //
    this->addDockWidget(Qt::BottomDockWidgetArea, pDockLogWindow);
  }

  // Create status bar
  Handle(asiUI_StatusBarImpl)
    statusBar = new asiUI_StatusBarImpl(new asiUI_StatusBar);
  //
  this->setStatusBar( statusBar->GetStatusBar() );
  //
  cf->StatusBar = statusBar;
  cf->StatusBar->SetStatusText("Load part from STEP or BREP to start analysis");

  // Initialize and connect progress listener
  cf->Logger           = new asiUI_Logger(m_widgets.wLogger);
  cf->ProgressListener = new asiUI_ProgressListener(statusBar, cf->ProgressNotifier, cf->Logger);
  cf->ProgressListener->Connect();

  /* ==================================
   *  Tcl console with custom commands
   * ================================== */

  // Construct the interpreter
  cf->Interp = new asiTcl_Interp;
  cf->Interp->Init();
  cf->Interp->SetPlotter(cf->Plotter);
  cf->Interp->SetProgress(cf->ProgressNotifier);

  // Load commands
  EXE_LOAD_MODULE("cmdMisc")
  EXE_LOAD_MODULE("cmdInspector")

  // Console window
  QDockWidget* pDockConsoleWindow;
  {
    pDockConsoleWindow = new QDockWidget("Console", this);
    pDockConsoleWindow->setAllowedAreas(Qt::BottomDockWidgetArea);
    //
    m_widgets.wConsole = new asiUI_Console(cf->Interp, pDockLogWindow);
    //
    pDockConsoleWindow->setWidget(m_widgets.wConsole);
    //
    this->addDockWidget(Qt::BottomDockWidgetArea, pDockConsoleWindow);
  }
}

//-----------------------------------------------------------------------------

//! Creates menus.
void exe_MainWindow::createMenus()
{
}

//-----------------------------------------------------------------------------

//! Creates toolbar.
void exe_MainWindow::createToolbar()
{
}
