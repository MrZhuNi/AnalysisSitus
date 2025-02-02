//-----------------------------------------------------------------------------
// Created on: 13 July 2016
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
#include <asiUI_PartCallback.h>

// asiUI includes
#include <asiUI_Viewer.h>

// asiVisu includes
#include <asiVisu_PrsManager.h>

//! Instantiation routine.
//! \return instance of the callback class.
asiUI_PartCallback* asiUI_PartCallback::New()
{
  return new asiUI_PartCallback(nullptr);
}

//! Instantiation routine accepting viewer.
//! \param pViewer [in] viewer to bind callback object to.
//! \return instance of the callback class.
asiUI_PartCallback* asiUI_PartCallback::New(asiUI_Viewer* pViewer)
{
  return new asiUI_PartCallback(pViewer);
}

//! Constructor accepting owning viewer as a parameter.
//! \param pViewer [in] owning viewer.
asiUI_PartCallback::asiUI_PartCallback(asiUI_Viewer* pViewer)
: asiUI_ViewerCallback(pViewer)
{}

//! Destructor.
asiUI_PartCallback::~asiUI_PartCallback()
{}

//-----------------------------------------------------------------------------

//! Listens to events. Performs all useful operations.
//! \param pCaller   [in] caller instance.
//! \param eventId   [in] ID of the event triggered this listener.
//! \param pCallData [in] invocation context.
void asiUI_PartCallback::Execute(vtkObject*    asiVisu_NotUsed(pCaller),
                                 unsigned long eventId,
                                 void*         asiVisu_NotUsed(pCallData))
{
  if ( eventId == EVENT_FIND_FACE )
    emit findFace();

  if ( eventId == EVENT_FIND_EDGE )
    emit findEdge();

  if ( eventId == EVENT_REFINE_TESSELLATION )
    emit refineTessellation();
}
