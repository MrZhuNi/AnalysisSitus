//-----------------------------------------------------------------------------
// Created on: 30 January 2019
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

#ifndef asiVisu_SurfDeviationPrs_h
#define asiVisu_SurfDeviationPrs_h

// asiVisu includes
#include <asiVisu_DefaultPrs.h>
#include <asiVisu_PlaneWidget.h>

// asiData includes
#include <asiData_SurfDeviationNode.h>

// VTK includes
#include <vtkScalarBarWidget.h>

//-----------------------------------------------------------------------------

//! Presentation class for deviation map on a surface which is computed w.r.t.
//! a reference mesh.
class asiVisu_SurfDeviationPrs : public asiVisu_DefaultPrs
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiVisu_SurfDeviationPrs, asiVisu_DefaultPrs)

  // Allows to register this Presentation class
  DEFINE_PRESENTATION_FACTORY(asiData_SurfDeviationNode, Instance)

public:

  //! Pipelines.
  enum PipelineId
  {
    Pipeline_Main = 1
  };

public:

  asiVisu_EXPORT static Handle(asiVisu_Prs)
    Instance(const Handle(ActAPI_INode)& N);

protected:

  virtual void
    afterUpdatePipelines() const;

  virtual void
    renderPipelines(vtkRenderer* renderer) const;

  virtual void
    deRenderPipelines(vtkRenderer* renderer) const;

private:

  //! Allocation is allowed only via Instance() method.
  asiVisu_SurfDeviationPrs(const Handle(ActAPI_INode)& N);

protected:

  vtkSmartPointer<vtkScalarBarWidget> m_scalarBarWidget; //!< Scalar bar.

};

#endif
