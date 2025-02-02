//-----------------------------------------------------------------------------
// Created on: 11 July 2017
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
#include <asiVisu_TriangulationLinksPipeline.h>

// asiVisu includes
#include <asiVisu_MeshUtils.h>
#include <asiVisu_TriangulationDataProvider.h>
#include <asiVisu_TriangulationNodeInfo.h>
#include <asiVisu_Utils.h>

// VTK includes
#include <vtkActor.h>
#include <vtkInformation.h>
#include <vtkProperty.h>

//-----------------------------------------------------------------------------

asiVisu_TriangulationLinksPipeline::asiVisu_TriangulationLinksPipeline(const vtkSmartPointer<asiVisu_TriangulationSource>& source)
//
: asiVisu_TriangulationPipelineBase (source),
  m_fPartRed                        (0.),
  m_fPartGreen                      (0.),
  m_fPartBlue                       (0.),
  m_bScalarsOn                      (false)
{
  m_dmFilter->SetDisplayMode(MeshDisplayMode_Wireframe);
}

//-----------------------------------------------------------------------------

//! Sets input data for the pipeline.
//! \param[in] dataProvider Data Provider.
void asiVisu_TriangulationLinksPipeline::SetInput(const Handle(asiVisu_DataProvider)& dataProvider)
{
  Handle(asiVisu_TriangulationDataProvider)
    DP = Handle(asiVisu_TriangulationDataProvider)::DownCast(dataProvider);

  /* ===========================
   *  Validate input Parameters.
   * =========================== */

  Handle(Poly_Triangulation) triangulation = DP->GetTriangulation();
  //
  if ( triangulation.IsNull() )
  {
    // Pass empty data set in order to have valid pipeline.
    vtkSmartPointer<vtkPolyData> dummyData = vtkSmartPointer<vtkPolyData>::New();
    this->SetInputData(dummyData);
    this->Modified(); // Update modification timestamp.
    return; // Do nothing.
  }

  /* ============================
   *  Prepare polygonal data set.
   * ============================ */

  // Update part-wise colors.
  DP->GetColor(m_fPartRed, m_fPartGreen, m_fPartBlue);

  // Update use of scalars flag.
  m_bScalarsOn = DP->HasScalars();

  if ( DP->MustExecute( this->GetMTime() ) )
  {
    // Clear cached data which is by design actual for the current state of
    // source only. The source changes, so the cache needs nullification.
    this->clearCache();

    // Bind to a Data Node using information key
    asiVisu_TriangulationNodeInfo::Store( DP->GetNodeID(), this->Actor() );

    // Initialize pipeline.
    this->SetInputConnection( m_source->GetOutputPort() );
  }

  // Update modification timestamp.
  this->Modified();
}
//-----------------------------------------------------------------------------

//! Callback for Update() routine.
void asiVisu_TriangulationLinksPipeline::callback_update()
{
  if ( m_bScalarsOn )
  {
    asiVisu_MeshUtils::InitMapper(m_mapper,
                                  ARRNAME_MESH_ITEM_TYPE,
                                  m_fPartRed,
                                  m_fPartGreen,
                                  m_fPartBlue);
  }
  else
  {
    m_mapper->ScalarVisibilityOff();
    m_mapper->Update();
  }
}
