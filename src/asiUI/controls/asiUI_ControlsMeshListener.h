//-----------------------------------------------------------------------------
// Created on: 12 February 2019
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

#ifndef asiUI_ControlsMeshListener_h
#define asiUI_ControlsMeshListener_h

// asiUI includes
#include <asiUI_CommonFacilities.h>
#include <asiUI_ControlsMesh.h>

// Qt includes
#pragma warning(push, 0)
#include <QObject>
#pragma warning(pop)

#pragma warning(disable : 4251)

//-----------------------------------------------------------------------------

//! Default slots for controls operating with meshes.
class asiUI_EXPORT asiUI_ControlsMeshListener : public QObject
{
  Q_OBJECT

public:

  asiUI_ControlsMeshListener(asiUI_ControlsMesh*                   wControls,
                             const Handle(asiUI_CommonFacilities)& cf);

  virtual
    ~asiUI_ControlsMeshListener();

public:

  virtual void
    Connect();

protected:

  asiUI_ControlsMesh*            m_wControls; //!< Controls.
  Handle(asiUI_CommonFacilities) m_cf;        //!< Common facilities.

};

#pragma warning(default : 4251)

#endif
