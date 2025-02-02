//-----------------------------------------------------------------------------
// Created on: 01 December 2016 (*)
//-----------------------------------------------------------------------------
// Copyright (c) 2017, Sergey Slyadnev
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

#ifndef asiUI_DialogFindEdge_h
#define asiUI_DialogFindEdge_h

// A-Situs includes
#include <asiUI.h>

// A-Situs (GUI) includes
#include <asiUI_LineEdit.h>
#include <asiUI_ViewerPart.h>

// A-Situs (visualization) includes
#include <asiVisu_PrsManager.h>

// Qt includes
#pragma warning(push, 0)
#include <QCheckBox>
#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#pragma warning(pop)

//! Dialog for finding edge of interest.
class asiUI_DialogFindEdge : public QDialog
{
  Q_OBJECT

public:

  asiUI_DialogFindEdge(const Handle(asiEngine_Model)&             model,
                       const vtkSmartPointer<asiVisu_PrsManager>& prsMgr,
                       asiUI_ViewerPart*                          pViewer,
                       QWidget*                                   parent = nullptr);

  virtual ~asiUI_DialogFindEdge();

public slots:

  void onUseAddress();
  void onFind();

protected:

  //! Widgets.
  struct t_widgets
  {
  //---------------------------------------------------------------------------
    QPushButton*    pFind;       //!< Find edge of interest.
  //---------------------------------------------------------------------------
    QCheckBox*      pUseAddress; //!< Indicates whether to use address.
    asiUI_LineEdit* pIndex;      //!< Edge index.
    asiUI_LineEdit* pAddress;    //!< Edge address.
  //---------------------------------------------------------------------------

    t_widgets() : pFind       (nullptr),
                  pUseAddress (nullptr),
                  pIndex      (nullptr),
                  pAddress    (nullptr)
    {}

    void Release()
    {
      delete pFind;       pFind       = nullptr;
      delete pUseAddress; pUseAddress = nullptr;
      delete pIndex;      pIndex      = nullptr;
      delete pAddress;    pAddress    = nullptr;
    }
  };

  t_widgets                           m_widgets;     //!< UI controls.
  QVBoxLayout*                        m_pMainLayout; //!< Layout of the widget.
  asiUI_ViewerPart*                   m_pViewer;     //!< External reference to viewer.
  Handle(asiEngine_Model)             m_model;       //!< Data Model instance.
  vtkSmartPointer<asiVisu_PrsManager> m_prsMgr;      //!< Presentation Manager.

};

#endif
