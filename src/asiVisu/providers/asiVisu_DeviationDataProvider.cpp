//-----------------------------------------------------------------------------
// Created on: 22 August 2019
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

// Own include
#include <asiVisu_DeviationDataProvider.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

//-----------------------------------------------------------------------------

asiVisu_DeviationDataProvider::asiVisu_DeviationDataProvider(const Handle(asiData_DeviationNode)& N)
: asiVisu_MeshNScalarDataProvider()
{
  m_node = N;
  m_triangulationParam = ActParamTool::AsTriangulation( N->Parameter(asiData_DeviationNode::PID_Mesh) );
}

//-----------------------------------------------------------------------------

Handle(HIntArray) asiVisu_DeviationDataProvider::GetNodeIDs() const
{
  return ActParamTool::AsIntArray( m_node->Parameter(asiData_DeviationNode::PID_DistanceFieldIds) )->GetArray();
}

//-----------------------------------------------------------------------------

Handle(HRealArray) asiVisu_DeviationDataProvider::GetNodeScalars() const
{
  return ActParamTool::AsRealArray( m_node->Parameter(asiData_DeviationNode::PID_DistanceFieldValues) )->GetArray();
}

//-----------------------------------------------------------------------------

double asiVisu_DeviationDataProvider::GetTolerance() const
{
  return ActParamTool::AsReal( m_node->Parameter(asiData_DeviationNode::PID_Tolerance) )->GetValue();
}

//-----------------------------------------------------------------------------

double asiVisu_DeviationDataProvider::GetMinScalar() const
{
  return ActParamTool::AsReal( m_node->Parameter(asiData_DeviationNode::PID_ScalarMin) )->GetValue();
}

//-----------------------------------------------------------------------------

double asiVisu_DeviationDataProvider::GetMaxScalar() const
{
  return ActParamTool::AsReal( m_node->Parameter(asiData_DeviationNode::PID_ScalarMax) )->GetValue();
}

//-----------------------------------------------------------------------------

Handle(ActAPI_HParameterList) asiVisu_DeviationDataProvider::translationSources() const
{
  ActAPI_ParameterStream out;
  out << m_node->Parameter(asiData_DeviationNode::PID_Mesh)
      << m_node->Parameter(asiData_DeviationNode::PID_DistanceFieldIds)
      << m_node->Parameter(asiData_DeviationNode::PID_DistanceFieldValues)
      << m_node->Parameter(asiData_DeviationNode::PID_Tolerance)
      << m_node->Parameter(asiData_DeviationNode::PID_ScalarMin)
      << m_node->Parameter(asiData_DeviationNode::PID_ScalarMax)
      ;

  return out.List;
}
