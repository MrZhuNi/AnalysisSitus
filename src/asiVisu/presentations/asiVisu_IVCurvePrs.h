//-----------------------------------------------------------------------------
// Created on: 11 April 2016
//-----------------------------------------------------------------------------
// Copyright (c) 2016-present, Sergey Slyadnev
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

#ifndef asiVisu_IVCurvePrs_h
#define asiVisu_IVCurvePrs_h

// asiVisu includes
#include <asiVisu_DefaultPrs.h>

// asiData includes
#include <asiData_IVCurveNode.h>

//-----------------------------------------------------------------------------

//! Presentation class for curves in IV.
class asiVisu_IVCurvePrs : public asiVisu_DefaultPrs
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiVisu_IVCurvePrs, asiVisu_DefaultPrs)

  // Allows to register this Presentation class
  DEFINE_PRESENTATION_FACTORY(asiData_IVCurveNode, Instance)

public:

  //! Primary pipelines.
  enum PrimaryPipelineId
  {
    PrimaryPipeline_Main = 1, //!< Curve as it is.
    PrimaryPipeline_Poles,    //!< Poles of a B-curve.
    PrimaryPipeline_Knots,    //!< Knots of a B-curve.
    PrimaryPipeline_Handles,  //!< Points on a curve representing its handles.
    PrimaryPipeline_Repers    //!< Reper points which were used to build this curve.
  };

  //! Selection pipelines.
  enum SelectionPipelineId
  {
    SelectionPipeline_Handles = 1, //!< Pipeline for interacting on handles.
    SelectionPipeline_Repers       //!< Pipeline for interacting on reper points.
  };

public:

  //! Creates a Presentation object for the passed Node.
  //! \param[in] N Data Node to create a Presentation for.
  asiVisu_EXPORT static Handle(asiVisu_Prs)
    Instance(const Handle(ActAPI_INode)& N);

protected:

  //! Highlights curve presentation.
  virtual void
    highlight(vtkRenderer*                        renderer,
              const Handle(asiVisu_PickerResult)& pickRes,
              const asiVisu_SelectionNature       selNature) const;

  //! Unhighlights curve presentation.
  virtual void
    unHighlight(vtkRenderer*                  renderer,
                const asiVisu_SelectionNature selNature) const;

  //! Callback on adding presentation pipelines to renderer.
  virtual void
    renderPipelines(vtkRenderer* renderer) const;

  //! Callback on removing presentation pipelines from renderer.
  virtual void
    deRenderPipelines(vtkRenderer* renderer) const;

private:

  //! Allocation is allowed only via Instance() method.
  asiVisu_IVCurvePrs(const Handle(ActAPI_INode)& N);

};

#endif
