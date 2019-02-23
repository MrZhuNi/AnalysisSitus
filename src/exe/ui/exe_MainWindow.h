//-----------------------------------------------------------------------------
// Created on: 07 December 2015
//-----------------------------------------------------------------------------
// Copyright (c) 2015-present, Sergey Slyadnev
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//    * Neither the name of the copyright holder(s) nor the
//      names of all contributors may be used to endorse or promote products
//      derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#ifndef exe_MainWindow_h
#define exe_MainWindow_h

// asiUI includes
#include <asiUI_Console.h>
#include <asiUI_ControlsFeature.h>
#include <asiUI_ControlsMeshListener.h>
#include <asiUI_ControlsPartListener.h>
#include <asiUI_ObjectBrowser.h>
#include <asiUI_ViewerDomainListener.h>
#include <asiUI_ViewerHostListener.h>
#include <asiUI_ViewerPartListener.h>

// Qt includes
#pragma warning(push, 0)
#include <QCloseEvent>
#include <QMainWindow>
#pragma warning(pop)

//! Main window for Analysis Situs.
class exe_MainWindow : public QMainWindow
{
  Q_OBJECT

public:

  exe_MainWindow();
  virtual ~exe_MainWindow();

public:

  void closeEvent(QCloseEvent* evt);

private:

  void createPartViewer();
  void createDockWindows();
  void createMenus();
  void createToolbar();

public:

  //! Widgets.
  struct t_widgets
  {
    asiUI_ObjectBrowser*   wBrowser;         //!< Object browser.
    asiUI_ViewerDomain*    wViewerDomain;    //!< Parametric domain viewer.
    asiUI_ViewerPart*      wViewerPart;      //!< Part viewer.
    asiUI_ViewerHost*      wViewerSurface;   //!< Surface viewer.
    asiUI_ControlsPart*    wControlsPart;    //!< Part controls.
    asiUI_ControlsFeature* wControlsFeature; //!< Feature controls.
    asiUI_ControlsMesh*    wControlsMesh;    //!< Mesh controls.
    asiUI_StyledTextEdit*  wLogger;          //!< Logger.
    asiUI_Console*         wConsole;         //!< Console for scripting.

    t_widgets() : wBrowser         (NULL),
                  wViewerDomain    (NULL),
                  wViewerPart      (NULL),
                  wViewerSurface   (NULL),
                  wControlsPart    (NULL),
                  wControlsFeature (NULL),
                  wControlsMesh    (NULL),
                  wLogger          (NULL),
                  wConsole         (NULL)
    {}

    void Release()
    {
      delete wBrowser;         wBrowser         = NULL;
      delete wViewerDomain;    wViewerDomain    = NULL;
      delete wViewerPart;      wViewerPart      = NULL;
      delete wViewerSurface;   wViewerSurface   = NULL;
      delete wControlsPart;    wControlsPart    = NULL;
      delete wControlsFeature; wControlsFeature = NULL;
      delete wControlsMesh;    wControlsMesh    = NULL;
      delete wLogger;          wLogger          = NULL;
      delete wConsole;         wConsole         = NULL;
    }
  };

  //! Listeners.
  struct t_listeners
  {
    asiUI_ControlsPartListener* pControlsPart; //!< Listener for part controls.
    asiUI_ControlsMeshListener* pControlsMesh; //!< Listener for mesh controls.
    asiUI_ViewerPartListener*   pViewerPart;   //!< Listener for part viewer.
    asiUI_ViewerDomainListener* pViewerDomain; //!< Listener for domain viewer.
    asiUI_ViewerHostListener*   pViewerHost;   //!< Listener for host viewer.

    t_listeners() : pControlsPart (NULL),
                    pControlsMesh (NULL),
                    pViewerPart   (NULL),
                    pViewerDomain (NULL),
                    pViewerHost   (NULL)
    {}

    void Release()
    {
      delete pControlsPart; pControlsPart = NULL;
      delete pControlsMesh; pControlsMesh = NULL;
      delete pViewerPart;   pViewerPart   = NULL;
      delete pViewerDomain; pViewerDomain = NULL;
      delete pViewerHost;   pViewerHost   = NULL;
    }
  };

  //! Menu items.
  struct t_menus
  {
  };

  //! Toolbar items.
  struct t_toolbar
  {
  };

  t_widgets   Widgets;
  t_listeners Listeners;
  t_menus     Menus;
  t_toolbar   Toolbar;

};

#endif
