//-----------------------------------------------------------------------------
// Created on: 26 December 2018
//-----------------------------------------------------------------------------
// Copyright (c) 2018-present, Sergey Slyadnev
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

#ifndef asiUI_PickCurveCallback_h
#define asiUI_PickCurveCallback_h

// asiUI includes
#include <asiUI_BaseCurveCallback.h>

//! Callback for interactive creation of a curve by picking points on
//! triangulation.
class asiUI_PickCurveCallback : public asiUI_BaseCurveCallback
{
  Q_OBJECT

public:

  //! Instantiation routine.
  //! \return instance of the callback class.
  asiUI_EXPORT static asiUI_PickCurveCallback* New();
  //
  vtkTypeMacro(asiUI_PickCurveCallback, asiUI_ViewerCallback)

public:

  //! Listens to a dedicated event. Performs all useful operations.
  //! \param[in] pCaller   caller instance.
  //! \param[in] eventId   ID of the event triggered this listener.
  //! \param[in] pCallData data passed from the caller.
  virtual void Execute(vtkObject*    pCaller,
                       unsigned long eventId,
                       void*         pCallData);

public:

  //! Sets curve name.
  //! \param[in] name name of the curve to create.
  void SetCurveName(const TCollection_AsciiString& name)
  {
    m_name = name;
  }

private:

  //! Constructor accepting owning viewer as a parameter.
  //! \param[in] pViewer owning viewer.
  asiUI_PickCurveCallback(asiUI_Viewer* pViewer);

  //! Dtor.
  ~asiUI_PickCurveCallback();

protected:

  TCollection_AsciiString m_name; //!< Curve name.

};

#endif
