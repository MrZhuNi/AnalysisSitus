//-----------------------------------------------------------------------------
// Created on: 17 February 2016
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

#ifndef asiUI_DialogSewing_h
#define asiUI_DialogSewing_h

// asiUI includes
#include <asiUI_LineEdit.h>

// asiEngine includes
#include <asiEngine_Model.h>

// Qt includes
#pragma warning(push, 0)
#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#pragma warning(pop)

//! Dialog for sewing.
class asiUI_DialogSewing : public QDialog
{
  Q_OBJECT

public:

  asiUI_DialogSewing(const Handle(asiEngine_Model)& model,
                     ActAPI_ProgressEntry           progress,
                     QWidget*                       parent = nullptr);

  virtual ~asiUI_DialogSewing();

public slots:

  void onPerform();

protected:

  //! Widgets.
  struct t_widgets
  {
    //! Editor IDs.
    enum editor
    {
      edit_Tolerance = 0,
      //
      LastEditor
    };

  //---------------------------------------------------------------------------
    QPushButton*    pPerform;   //!< Perform sewing.
  //---------------------------------------------------------------------------
    asiUI_LineEdit* pTolerance; //!< Sewing tolerance.
  //---------------------------------------------------------------------------

    t_widgets() : pPerform   (nullptr),
                  pTolerance (nullptr)
    {}

    void Release()
    {
      delete pPerform;   pPerform    = nullptr;
      delete pTolerance; pTolerance  = nullptr;
    }
  };

  t_widgets                m_widgets;     //!< UI controls.
  QVBoxLayout*             m_pMainLayout; //!< Layout of the widget.
  Handle(asiEngine_Model)  m_model;       //!< Data Model instance.
  ActAPI_ProgressEntry     m_progress;    //!< Progress entry.

};

#endif
