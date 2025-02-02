//-----------------------------------------------------------------------------
// Created on: 11 April 2016
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
#include <asiVisu_IVSurfaceDataProvider.h>

// Visualization includes
#include <asiData_IVSurfaceNode.h>

//-----------------------------------------------------------------------------

//! Constructor.
//! \param N [in] source Node.
asiVisu_IVSurfaceDataProvider::asiVisu_IVSurfaceDataProvider(const Handle(ActAPI_INode)& N)
: asiVisu_SurfaceDataProvider(),
  m_node(N)
{}

//-----------------------------------------------------------------------------

//! \return ID of the associated Data Node.
ActAPI_DataObjectId asiVisu_IVSurfaceDataProvider::GetNodeID() const
{
  return m_node->GetId();
}

//-----------------------------------------------------------------------------

//! \return surface type.
Handle(Standard_Type) asiVisu_IVSurfaceDataProvider::GetSurfaceType() const
{
  Handle(asiData_IVSurfaceNode)
    surface_n = Handle(asiData_IVSurfaceNode)::DownCast(m_node);
  //
  if ( surface_n.IsNull() || !surface_n->IsWellFormed() )
    return nullptr;

  return surface_n->GetSurface()->DynamicType();
}

//-----------------------------------------------------------------------------

// Accessor for the parametric surface.
//! \param uMin [out] min bound for U curvilinear axis.
//! \param uMax [out] max bound for U curvilinear axis.
//! \param vMin [out] min bound for V curvilinear axis.
//! \param vMax [out] max bound for V curvilinear axis.
//! \return surface.
Handle(Geom_Surface)
  asiVisu_IVSurfaceDataProvider::GetSurface(double& uMin,
                                            double& uMax,
                                            double& vMin,
                                            double& vMax) const
{
  Handle(asiData_IVSurfaceNode)
    surface_n = Handle(asiData_IVSurfaceNode)::DownCast(m_node);
  //
  if ( surface_n.IsNull() || !surface_n->IsWellFormed() )
    return nullptr;

  // Get bounds.
  surface_n->GetLimits(uMin, uMax, vMin, vMax);

  // Get surface.
  return surface_n->GetSurface();
}

//-----------------------------------------------------------------------------

//! Enumerates Data Parameters playing as sources for DOMAIN -> VTK
//! translation process.
//! \return source Parameters.
Handle(ActAPI_HParameterList) asiVisu_IVSurfaceDataProvider::translationSources() const
{
  // Resulting Parameters
  ActParamStream out;

  Handle(asiData_IVSurfaceNode)
    surface_n = Handle(asiData_IVSurfaceNode)::DownCast(m_node);
  //
  if ( surface_n.IsNull() || !surface_n->IsWellFormed() )
    return out;

  // Add sensitive Parameters
  out << surface_n->Parameter(asiData_IVSurfaceNode::PID_Geometry)
      << surface_n->Parameter(asiData_IVSurfaceNode::PID_UMin)
      << surface_n->Parameter(asiData_IVSurfaceNode::PID_UMax)
      << surface_n->Parameter(asiData_IVSurfaceNode::PID_VMin)
      << surface_n->Parameter(asiData_IVSurfaceNode::PID_VMax);

  return out;
}
