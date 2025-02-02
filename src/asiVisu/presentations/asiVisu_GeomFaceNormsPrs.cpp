//-----------------------------------------------------------------------------
// Created on: 02 March 2017
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
#include <asiVisu_GeomFaceNormsPrs.h>

// asiVisu includes
#include <asiVisu_FaceNormalsDataProvider.h>
#include <asiVisu_Utils.h>
#include <asiVisu_VectorsPipeline.h>

// VTK includes
#include <vtkMapper.h>
#include <vtkProperty.h>

//-----------------------------------------------------------------------------

//! Creates a Presentation object for the passed Node.
//! \param N [in] Node to create a Presentation for.
asiVisu_GeomFaceNormsPrs::asiVisu_GeomFaceNormsPrs(const Handle(ActAPI_INode)& N)
: asiVisu_Prs(N)
{
  Handle(asiData_FaceNormsNode) face_n = Handle(asiData_FaceNormsNode)::DownCast(N);

  // Create Data Provider
  Handle(asiVisu_FaceNormalsDataProvider) DP = new asiVisu_FaceNormalsDataProvider(face_n);

  // Pipeline for normals
  this->addPipeline        ( Pipeline_Main, new asiVisu_VectorsPipeline );
  this->assignDataProvider ( Pipeline_Main, DP );

  // Disable scalar visibility as colors will correspond to orientation
  Handle(asiVisu_VectorsPipeline)
    PL = Handle(asiVisu_VectorsPipeline)::DownCast( this->GetPipeline(Pipeline_Main) );
  //
  PL->Mapper()->ScalarVisibilityOff();
}

//! Factory method for Presentation.
//! \param N [in] Node to create a Presentation for.
//! \return new Presentation instance.
Handle(asiVisu_Prs) asiVisu_GeomFaceNormsPrs::Instance(const Handle(ActAPI_INode)& N)
{
  return new asiVisu_GeomFaceNormsPrs(N);
}

//! Returns true if the Presentation is visible, false -- otherwise.
//! \return true/false.
bool asiVisu_GeomFaceNormsPrs::IsVisible() const
{
  return m_node->HasUserFlags(NodeFlag_IsPresentationVisible);
}

//-----------------------------------------------------------------------------

//! Callback for initialization of Presentation pipelines.
void asiVisu_GeomFaceNormsPrs::beforeInitPipelines()
{
  // Do nothing...
}

//! Callback for initialization of Presentation pipelines.
void asiVisu_GeomFaceNormsPrs::afterInitPipelines()
{
  // Do nothing...
}

//! Callback for updating of Presentation pipelines invoked before the
//! kernel update routine starts.
void asiVisu_GeomFaceNormsPrs::beforeUpdatePipelines() const
{
  // Do nothing...
}

//! Callback for updating of Presentation pipelines invoked after the
//! kernel update routine completes.
void asiVisu_GeomFaceNormsPrs::afterUpdatePipelines() const
{
  Handle(asiVisu_FaceNormalsDataProvider)
    DP = Handle(asiVisu_FaceNormalsDataProvider)::DownCast( this->dataProvider(Pipeline_Main) );
  //
  Handle(asiVisu_VectorsPipeline)
    PL = Handle(asiVisu_VectorsPipeline)::DownCast( this->GetPipeline(Pipeline_Main) );

  TopoDS_Face face = DP->GetFace();

  // Set color according to orientation
  if ( face.Orientation() == TopAbs_FORWARD )
  {
    PL->Actor()->GetProperty()->SetColor(1.0, 0.0, 0.0);
  }
  else if ( face.Orientation() == TopAbs_REVERSED )
  {
    PL->Actor()->GetProperty()->SetColor(0.0, 0.0, 1.0);
  }
  else
  {
    PL->Actor()->GetProperty()->SetColor(0.5, 0.5, 0.5);
  }
}

//! Callback for highlighting.
void asiVisu_GeomFaceNormsPrs::highlight(vtkRenderer*,
                                         const Handle(asiVisu_PickerResult)&,
                                         const asiVisu_SelectionNature) const
{}

//! Callback for highlighting reset.
void asiVisu_GeomFaceNormsPrs::unHighlight(vtkRenderer*,
                                           const asiVisu_SelectionNature) const
{}

//! Callback for rendering.
void asiVisu_GeomFaceNormsPrs::renderPipelines(vtkRenderer*) const
{}

//! Callback for de-rendering.
void asiVisu_GeomFaceNormsPrs::deRenderPipelines(vtkRenderer*) const
{}
