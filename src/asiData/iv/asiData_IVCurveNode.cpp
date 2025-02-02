//-----------------------------------------------------------------------------
// Created on: 08 April 2016
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

// Own include
#include <asiData_IVCurveNode.h>

// Active Data includes
#include <ActAux_ArrayUtils.h>
#include <ActData_ParameterFactory.h>

// OCCT includes
#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>

//-----------------------------------------------------------------------------

asiData_IVCurveNode::asiData_IVCurveNode() : ActData_BaseNode()
{
  REGISTER_PARAMETER(Name,      PID_Name);
  REGISTER_PARAMETER(Shape,     PID_Geometry);
  REGISTER_PARAMETER(RealArray, PID_Handles);
  REGISTER_PARAMETER(Int,       PID_ActiveHandle);
  REGISTER_PARAMETER(RealArray, PID_ReperPoints);
  REGISTER_PARAMETER(Int,       PID_ActiveReper);
  REGISTER_PARAMETER(Bool,      PID_DrawOriTip);
}

//-----------------------------------------------------------------------------

Handle(ActAPI_INode) asiData_IVCurveNode::Instance()
{
  return new asiData_IVCurveNode();
}

//-----------------------------------------------------------------------------

void asiData_IVCurveNode::Init()
{
  this->SetCurve              ( nullptr, 0.0, 0.0 );
  this->SetHandles            ( nullptr );
  this->SetActiveHandle       ( -1 );
  this->SetActiveReper        ( -1 );
  this->SetDrawOrientationTip ( true );

  ActParamTool::AsRealArray( this->Parameter(PID_ReperPoints) )->SetArray(nullptr);

  // Initialize properties.
  this->InitParameter (PID_Name,       "Name",             "", ParameterFlag_IsVisible, true);
  this->InitParameter (PID_DrawOriTip, "Orientation tip",  "", ParameterFlag_IsVisible, true);
}

//-----------------------------------------------------------------------------

TCollection_ExtendedString asiData_IVCurveNode::GetName()
{
  return ActParamTool::AsName( this->Parameter(PID_Name) )->GetValue();
}

//-----------------------------------------------------------------------------

void asiData_IVCurveNode::SetName(const TCollection_ExtendedString& name)
{
  ActParamTool::AsName( this->Parameter(PID_Name) )->SetValue(name);
}

//-----------------------------------------------------------------------------

Handle(Geom_Curve) asiData_IVCurveNode::GetCurve() const
{
  double f, l;
  return this->GetCurve(f, l);
}

//-----------------------------------------------------------------------------

Handle(Geom_Curve) asiData_IVCurveNode::GetCurve(double& f, double& l) const
{
  TopoDS_Shape sh = ActParamTool::AsShape( this->Parameter(PID_Geometry) )->GetShape();
  //
  if ( sh.IsNull() || sh.ShapeType() != TopAbs_EDGE )
    return nullptr;

  // Extract edge and its host geometry
  const TopoDS_Edge& E = TopoDS::Edge(sh);
  //
  return BRep_Tool::Curve(E, f, l);
}

//-----------------------------------------------------------------------------

void asiData_IVCurveNode::SetCurve(const Handle(Geom_Curve)& curve,
                                   const double              f,
                                   const double              l)
{
  // Create a fictive edge to take advantage of topology Parameter of Active Data
  TopoDS_Edge E;
  if ( !curve.IsNull() )
    E = BRepBuilderAPI_MakeEdge(curve, f, l);

  // Store
  ActParamTool::AsShape( this->Parameter(PID_Geometry) )->SetShape(E);
}

//-----------------------------------------------------------------------------

void asiData_IVCurveNode::SetCurve(const Handle(Geom_Curve)& curve)
{
  this->SetCurve( curve, curve->FirstParameter(), curve->LastParameter() );
}

//-----------------------------------------------------------------------------

void asiData_IVCurveNode::AddHandle(const double u)
{
  Handle(HRealArray) arr = this->GetHandles();
  //
  if ( arr.IsNull() )
  {
    arr = new HRealArray(0, 0);
    arr->SetValue(0, u);
  }
  else
    arr = ActAux_ArrayUtils::Append<HRealArray, Handle(HRealArray), double>(arr, u);

  // Store array.
  this->SetHandles(arr);
}

//-----------------------------------------------------------------------------

void asiData_IVCurveNode::SetHandles(const Handle(HRealArray)& handles)
{
  ActParamTool::AsRealArray( this->Parameter(PID_Handles) )->SetArray(handles);
}

//-----------------------------------------------------------------------------

Handle(HRealArray) asiData_IVCurveNode::GetHandles() const
{
  Handle(HRealArray)
    handles = ActParamTool::AsRealArray( this->Parameter(PID_Handles) )->GetArray();
  //
  return handles;
}

//-----------------------------------------------------------------------------

void asiData_IVCurveNode::SetActiveHandle(const int handleId)
{
  ActParamTool::AsInt( this->Parameter(PID_ActiveHandle) )->SetValue(handleId);
}

//-----------------------------------------------------------------------------

int asiData_IVCurveNode::GetActiveHandle() const
{
  return ActParamTool::AsInt( this->Parameter(PID_ActiveHandle) )->GetValue();
}

//-----------------------------------------------------------------------------

double asiData_IVCurveNode::GetActiveHandleParam() const
{
  const int activeHandle = this->GetActiveHandle();
  //
  if ( activeHandle == -1 )
    return DBL_MAX;

  Handle(HRealArray) handles = this->GetHandles();
  //
  return handles->Value(activeHandle);
}

//-----------------------------------------------------------------------------

void asiData_IVCurveNode::AddReperPoint(const gp_XYZ& pt)
{
  Handle(HRealArray)
    arr = ActParamTool::AsRealArray( this->Parameter(PID_ReperPoints) )->GetArray();

  // Append coordinates.
  Handle(HRealArray)
    newArr = ActAux_ArrayUtils::Append3d<HRealArray, Handle(HRealArray), gp_XYZ>(arr, pt);

  ActParamTool::AsRealArray( this->Parameter(PID_ReperPoints) )->SetArray(newArr);
}

//-----------------------------------------------------------------------------

void asiData_IVCurveNode::SetReperPoints(const std::vector<gp_XYZ>& pts)
{
  Handle(HRealArray) arr;
  ActAux_ArrayUtils::ToCoords3d<gp_XYZ>(pts, arr);

  ActParamTool::AsRealArray( this->Parameter(PID_ReperPoints) )->SetArray(arr);
}

//-----------------------------------------------------------------------------

void asiData_IVCurveNode::GetReperPoints(std::vector<gp_XYZ>& pts) const
{
  Handle(HRealArray)
    arr = ActParamTool::AsRealArray( this->Parameter(PID_ReperPoints) )->GetArray();

  ActAux_ArrayUtils::FromCoords3d<gp_XYZ>(arr, pts);
}

//-----------------------------------------------------------------------------

void asiData_IVCurveNode::SetActiveReper(const int reperId)
{
  ActParamTool::AsInt( this->Parameter(PID_ActiveReper) )->SetValue(reperId);
}

//-----------------------------------------------------------------------------

int asiData_IVCurveNode::GetActiveReper() const
{
  return ActParamTool::AsInt( this->Parameter(PID_ActiveReper) )->GetValue();
}

//-----------------------------------------------------------------------------

gp_XYZ asiData_IVCurveNode::GetActiveReperPoint() const
{
  const int activeReper = this->GetActiveReper();
  //
  if ( activeReper == -1 )
    return gp_XYZ();

  std::vector<gp_XYZ> pts;
  this->GetReperPoints(pts);

  return pts[activeReper];
}

//-----------------------------------------------------------------------------

void asiData_IVCurveNode::SetDrawOrientationTip(const bool on)
{
  ActParamTool::AsBool( this->Parameter(PID_DrawOriTip) )->SetValue(on);
}

//-----------------------------------------------------------------------------

bool asiData_IVCurveNode::GetDrawOrientationTip() const
{
  return ActParamTool::AsBool( this->Parameter(PID_DrawOriTip) )->GetValue();
}
