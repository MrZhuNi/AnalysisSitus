//-----------------------------------------------------------------------------
// Created on: 06 April 2017
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

#ifndef asiVisu_PartEdgesPipeline_h
#define asiVisu_PartEdgesPipeline_h

// asiVisu includes
#include <asiVisu_PartPipelineBase.h>

// VTK includes
#include <vtkTransformPolyDataFilter.h>

//-----------------------------------------------------------------------------

//! Visualization pipeline for wireframe of OCCT shape. This pipeline is
//! secondary: it should be feeded with a source of a main shape pipeline.
class asiVisu_PartEdgesPipeline : public asiVisu_PartPipelineBase
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiVisu_PartEdgesPipeline, asiVisu_PartPipelineBase)

public:

  asiVisu_EXPORT
    asiVisu_PartEdgesPipeline(const vtkSmartPointer<asiVisu_ShapeRobustSource>& source);

public:

  asiVisu_EXPORT virtual void
    SetInput(const Handle(asiVisu_DataProvider)& dataProvider);

private:

  virtual void callback_update();

protected:

  //! Filter for transformation.
  vtkSmartPointer<vtkTransformPolyDataFilter> m_tranformFilter;

  //! Components of part-wise color.
  double m_fPartRed, m_fPartGreen, m_fPartBlue;

};

#endif
