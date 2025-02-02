//-----------------------------------------------------------------------------
// Created on: 30 January 2018
//-----------------------------------------------------------------------------
// Copyright (c) 2018, Sergey Slyadnev
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

#ifndef asiVisu_CurvatureCombsDataProvider_h
#define asiVisu_CurvatureCombsDataProvider_h

// asiVisu includes
#include <asiVisu_CurveDataProvider.h>

// asiData includes
#include <asiData_CurvatureCombsNode.h>
#include <asiData_PartNode.h>

// OCCT includes
#include <BRep_Tool.hxx>
#include <TopoDS.hxx>

//! Data provider for curvature combs.
class asiVisu_CurvatureCombsDataProvider : public asiVisu_CurveDataProvider
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiVisu_CurvatureCombsDataProvider, asiVisu_CurveDataProvider)

public:

  //! Ctor accepting curvature combs Node.
  //! \param[in] partNode  Part Node.
  //! \param[in] combsNode Data Node to set as a source of data.
  asiVisu_CurvatureCombsDataProvider(const Handle(asiData_PartNode)&           partNode,
                                     const Handle(asiData_CurvatureCombsNode)& combsNode)
  : asiVisu_CurveDataProvider()
  {
    m_partNode  = partNode;
    m_combsNode = combsNode;
  }

public:

  //! \return curve type.
  virtual Handle(Standard_Type) GetCurveType() const
  {
    return STANDARD_TYPE(Geom_Curve);
  }

  //! \return 2D curve.
  virtual Handle(Geom2d_Curve) GetCurve2d(double& asiVisu_NotUsed(f), double& asiVisu_NotUsed(l)) const
  {
    return nullptr;
  }

  //! \return 3D curve.
  virtual Handle(Geom_Curve) GetCurve(double& f, double& l) const
  {
    Handle(asiData_CurveNode) curve_n = m_combsNode->GetRefCurve();
    //
    if ( curve_n.IsNull() || !curve_n->IsWellFormed() )
      return nullptr;

    // Get ID of the active edge
    const int globalId = curve_n->GetSelectedEdge();
    //
    const TopTools_IndexedMapOfShape& subShapes = m_partNode->GetAAG()->RequestMapOfSubShapes();
    //
    if ( globalId <= 0 || globalId > subShapes.Extent() )
      return nullptr;

    // Get edge
    const TopoDS_Shape& shape = subShapes(globalId);
    //
    if ( shape.ShapeType() != TopAbs_EDGE )
      return nullptr;

    // Return curve
    return BRep_Tool::Curve( TopoDS::Edge(shape), f, l );
  }

public:

  //! \return scale factor.
  double GetScaleFactor() const
  {
    return m_combsNode->GetScaleFactor();
  }

  //! Returns data vector of discretization points.
  //! \param[out] points vector of discretization points.
  void GetPoints(std::vector<gp_Pnt>& points)
  {
    m_combsNode->GetPoints(points);
  }

  //! Returns data vector containing the statuses of discretization points.
  //! \param[out] points vector of statuses.
  void GetPointsStatuses(std::vector<bool>& pointsOk)
  {
    m_combsNode->GetPointsStatuses(pointsOk);
  }

  //! Returns data vector of discretization parameters.
  //! \param[out] params vector of discretization parameters.
  void GetParameters(std::vector<double>& params)
  {
    m_combsNode->GetParameters(params);
  }

  //! Returns data vector of curvature values.
  //! \param[out] curvatures vector of curvature values.
  void GetCurvatures(std::vector<double>& curvatures)
  {
    m_combsNode->GetCurvatures(curvatures);
  }

  //! Returns data vector of curvature combs.
  //! \param[out] combs vector of curvature combs.
  void GetCombs(std::vector<gp_Vec>& combs)
  {
    m_combsNode->GetCombs(combs);
  }

public:

  //! Returns associated Node ID.
  //! \return Node ID.
  virtual ActAPI_DataObjectId GetNodeID() const
  {
    return m_combsNode->GetId();
  }

protected:

  //! Enumerates all Active Data Parameters playing as sources for DOMAIN -> VTK
  //! translation process. If any Parameter listed by this method is changed
  //! (more precisely, if its MTime record is updated), the translation must
  //! be repeated.
  //! \return list of source Parameters.
  virtual Handle(ActAPI_HParameterList) translationSources() const
  {
    ActParamStream out;

    out << m_combsNode->Parameter(asiData_CurvatureCombsNode::PID_Combs)
        << m_combsNode->Parameter(asiData_CurvatureCombsNode::PID_Points)
        << m_combsNode->Parameter(asiData_CurvatureCombsNode::PID_PointsStatuses)
        << m_combsNode->Parameter(asiData_CurvatureCombsNode::PID_Parameters)
        << m_combsNode->Parameter(asiData_CurvatureCombsNode::PID_Curvatures)
        << m_combsNode->Parameter(asiData_CurvatureCombsNode::PID_Combs)
        << m_combsNode->Parameter(asiData_CurvatureCombsNode::PID_RefCurve)
        << m_combsNode->Parameter(asiData_CurvatureCombsNode::PID_ScaleFactor);

    return out.List;
  }

protected:

  Handle(asiData_PartNode)           m_partNode;  //!< Part Node.
  Handle(asiData_CurvatureCombsNode) m_combsNode; //!< Data Node storing the curvature combs.

};

#endif
