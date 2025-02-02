//-----------------------------------------------------------------------------
// Created on: 16 April 2016
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
#include <asiVisu_IVPointSet2dDataProvider.h>

// asiAlgo includes
#include <asiAlgo_PointCloudUtils.h>

//-----------------------------------------------------------------------------

//! Constructor.
//! \param pointSet [in] point set to source data from.
asiVisu_IVPointSet2dDataProvider::asiVisu_IVPointSet2dDataProvider(const Handle(ActAPI_INode)& pointSet)
: asiVisu_PointsDataProvider(pointSet)
{}

//-----------------------------------------------------------------------------

//! \return point cloud to visualize.
Handle(asiAlgo_BaseCloud<double>) asiVisu_IVPointSet2dDataProvider::GetPoints() const
{
  Handle(asiData_IVPointSet2dNode)
    points_n = Handle(asiData_IVPointSet2dNode)::DownCast(m_node);
  //
  if ( points_n.IsNull() || !points_n->IsWellFormed() )
    return nullptr;

  Handle(TColStd_HArray1OfReal) coords2d = points_n->GetPoints();
  if ( coords2d.IsNull() )
    return nullptr;

  // Re-pack to 3d
  const int nPts = coords2d->Length() / 2;
  Handle(TColStd_HArray1OfReal) coords3d = new TColStd_HArray1OfReal(0, nPts*3 - 1);
  //
  for ( int coordidx3d = 0, coordidx2d = 0; coordidx3d < coords3d->Length(); coordidx3d += 3, coordidx2d += 2 )
  {
    coords3d->ChangeValue(coordidx3d + 0) = coords2d->Value(coordidx2d);
    coords3d->ChangeValue(coordidx3d + 1) = coords2d->Value(coordidx2d + 1);
    coords3d->ChangeValue(coordidx3d + 2) = 0.0;
  }

  return asiAlgo_PointCloudUtils::AsCloudd(coords3d);
}

//-----------------------------------------------------------------------------

//! \return nullptr filter.
Handle(TColStd_HPackedMapOfInteger) asiVisu_IVPointSet2dDataProvider::GetIndices() const
{
  return nullptr;
}

//-----------------------------------------------------------------------------

//! Enumerates Data Parameters playing as sources for DOMAIN -> VTK
//! translation process.
//! \return source Parameters.
Handle(ActAPI_HParameterList) asiVisu_IVPointSet2dDataProvider::translationSources() const
{
  // Resulting Parameters
  ActParamStream out;

  Handle(asiData_IVPointSet2dNode)
    points_n = Handle(asiData_IVPointSet2dNode)::DownCast(m_node);
  //
  if ( points_n.IsNull() || !points_n->IsWellFormed() )
    return out;

  // Register Parameter as sensitive
  out << points_n->Parameter(asiData_IVPointSet2dNode::PID_Geometry);

  return out;
}
