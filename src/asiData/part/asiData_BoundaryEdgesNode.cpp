//-----------------------------------------------------------------------------
// Created on: 22 March 2016
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
#include <asiData_BoundaryEdgesNode.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

//-----------------------------------------------------------------------------

//! Default constructor. Registers all involved Parameters.
asiData_BoundaryEdgesNode::asiData_BoundaryEdgesNode() : ActData_BaseNode()
{
  REGISTER_PARAMETER(Name,  PID_Name);
  REGISTER_PARAMETER(Shape, PID_Green);
  REGISTER_PARAMETER(Shape, PID_Red);
  REGISTER_PARAMETER(Shape, PID_Ordinary);
}

//! Returns new DETACHED instance of the Node ensuring its correct
//! allocation in a heap.
//! \return new instance of the Node.
Handle(ActAPI_INode) asiData_BoundaryEdgesNode::Instance()
{
  return new asiData_BoundaryEdgesNode();
}

//! Performs initial actions required to make this Node WELL-FORMED.
void asiData_BoundaryEdgesNode::Init()
{
  // Initialize name Parameter
  this->InitParameter(PID_Name, "Name");

  // Set empty initial shapes
  ActParamTool::AsShape( this->Parameter(PID_Green)    )->SetShape( TopoDS_Shape(), MT_Silent );
  ActParamTool::AsShape( this->Parameter(PID_Red)      )->SetShape( TopoDS_Shape(), MT_Silent );
  ActParamTool::AsShape( this->Parameter(PID_Ordinary) )->SetShape( TopoDS_Shape(), MT_Silent );
}

//-----------------------------------------------------------------------------
// Generic naming
//-----------------------------------------------------------------------------

//! Accessor for the Node's name.
//! \return name of the Node.
TCollection_ExtendedString asiData_BoundaryEdgesNode::GetName()
{
  return ActParamTool::AsName( this->Parameter(PID_Name) )->GetValue();
}

//! Sets name for the Node.
//! \param theName [in] name to set.
void asiData_BoundaryEdgesNode::SetName(const TCollection_ExtendedString& theName)
{
  ActParamTool::AsName( this->Parameter(PID_Name) )->SetValue(theName);
}
