//-----------------------------------------------------------------------------
// Created on: 25 February 2019
//-----------------------------------------------------------------------------
// Copyright (c) 2019-present, Sergey Slyadnev
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

#ifndef asiUI_ParameterEditorListener_h
#define asiUI_ParameterEditorPartListener_h

// asiUI includes
#include <asiUI_CommonFacilities.h>
#include <asiUI_ParameterEditor.h>

// Qt includes
#include <QObject>

#pragma warning(disable : 4251)

//-----------------------------------------------------------------------------

//! Default slots for parameter editor.
class asiUI_EXPORT asiUI_ParameterEditorListener : public QObject
{
  Q_OBJECT

public:

  asiUI_ParameterEditorListener(asiUI_ParameterEditor*                wEditor,
                                const Handle(asiUI_CommonFacilities)& cf);

  virtual
    ~asiUI_ParameterEditorListener();

public:

  virtual void
    Connect();

protected:

  asiUI_ParameterEditor*         m_wEditor; //!< Parameter editor.
  Handle(asiUI_CommonFacilities) m_cf;      //!< Common facilities.

};

#pragma warning(default : 4251)

#endif
