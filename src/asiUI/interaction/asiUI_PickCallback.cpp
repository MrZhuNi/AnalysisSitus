//-----------------------------------------------------------------------------
// Created on: 28 November 2015
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

// Own include
#include <asiUI_PickCallback.h>

// asiUI includes
#include <asiUI_Viewer.h>

// asiVisu includes
#include <asiVisu_PrsManager.h>
#include <asiVisu_Utils.h>

// VTK includes
#pragma warning(push, 0)
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#pragma warning(pop)

//! Instantiation routine.
//! \return instance of the callback class.
asiUI_PickCallback* asiUI_PickCallback::New()
{
  return new asiUI_PickCallback(nullptr, nullptr);
}

//! Instantiation routine accepting viewer.
//! \param[in] model   Data Model instance.
//! \param[in] pViewer iewer to bind callback object to.
//! \return instance of the callback class.
asiUI_PickCallback*
  asiUI_PickCallback::New(const Handle(asiEngine_Model)& model,
                          asiUI_Viewer*                  pViewer)
{
  return new asiUI_PickCallback(model, pViewer);
}

//! Constructor accepting owning viewer as a parameter.
//! \param[in] model   Data Model instance.
//! \param[in] pViewer owning viewer.
asiUI_PickCallback::asiUI_PickCallback(const Handle(asiEngine_Model)& model,
                                       asiUI_Viewer*                  pViewer)
: asiUI_ViewerCallback (pViewer),
  m_iPickerTypes       (PickerType_World)
{
  this->SetModel(model);
}

//! Default destructor.
asiUI_PickCallback::~asiUI_PickCallback()
{}

//-----------------------------------------------------------------------------

//! Listens to a dedicated event. Performs all useful operations.
//! \param pCaller   [in] caller instance.
//! \param eventId   [in] ID of the event triggered this listener.
//! \param pCallData [in] invocation context.
void asiUI_PickCallback::Execute(vtkObject*    vtkNotUsed(pCaller),
                                 unsigned long eventId,
                                 void*         pCallData)
{
  const vtkSmartPointer<asiVisu_PrsManager>& mgr = this->GetViewer()->PrsMgr();

  // Check if the calling context is valid
  if ( eventId != EVENT_PICK_DEFAULT && eventId != EVENT_DETECT_DEFAULT )
    return;
  //
  if ( !this->GetViewer() )
    return;

  // Access selection context
  const int selMode = mgr->GetSelectionMode();

  // Skip for disabled selection
  if ( selMode & SelectionMode_None )
    return;

  // Now pick
  asiVisu_PickInput* pickInput = reinterpret_cast<asiVisu_PickInput*>(pCallData);
  //
  const asiVisu_SelectionNature sel_type = (eventId == EVENT_PICK_DEFAULT) ? SelectionNature_Persistent
                                                                           : SelectionNature_Detection;
  mgr->Pick(pickInput, sel_type, m_iPickerTypes);

  // Notify observers
  if ( eventId == EVENT_PICK_DEFAULT )
    emit picked();
}
