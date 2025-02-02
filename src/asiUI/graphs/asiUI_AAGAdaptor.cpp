//-----------------------------------------------------------------------------
// Created on: 21 March 2016
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
#include <asiUI_AAGAdaptor.h>

// asiUI includes
#include <asiUI_PartGraphItem.h>

// asiAlgo includes
#include <asiAlgo_FeatureAttrAngle.h>
#include <asiAlgo_Utils.h>

// OCCT includes
#include <TColStd_MapIteratorOfPackedMapOfInteger.hxx>
#include <TopoDS.hxx>

// VTK includes
#pragma warning(push, 0)
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkStringArray.h>
#pragma warning(pop)

//! Converts AAG to VTK presentable form.
//! \param aag [in] AAG to convert.
//! \return VTK graph.
vtkSmartPointer<vtkMutableUndirectedGraph>
  asiUI_AAGAdaptor::Convert(const Handle(asiAlgo_AAG)& aag)
{
  vtkSmartPointer<vtkMutableUndirectedGraph>
    result = vtkSmartPointer<vtkMutableUndirectedGraph>::New();

  // Array for vertex groups
  vtkNew<vtkStringArray> groupArr;
  groupArr->SetNumberOfComponents(1);
  groupArr->SetName(ARRNAME_GROUP);

  // Array for vertex labels
  vtkNew<vtkStringArray> labelArr;
  labelArr->SetNumberOfComponents(1);
  labelArr->SetName(ARRNAME_LABELS);

  // Array for shape addresses
  vtkNew<vtkStringArray> addressArr;
  addressArr->SetNumberOfComponents(1);
  addressArr->SetName(ARRNAME_ADDRESS);

  // Array for angle attributes associated with arcs
  vtkNew<vtkIntArray> angleArr;
  angleArr->SetNumberOfComponents(1);
  angleArr->SetName(ARRNAME_ANGLES);

  // Get faces from AAG
  const TopTools_IndexedMapOfShape& Faces        = aag->GetMapOfFaces();
  const TColStd_PackedMapOfInteger& SelFaces     = aag->GetSelectedFaces();
  const asiAlgo_AdjacencyMx&        Neighborhood = aag->GetNeighborhood();

  // Add vertices for faces
  NCollection_DataMap<int, vtkIdType> FaceVertexMap;
  for ( asiAlgo_AdjacencyMx::t_mx::Iterator it(Neighborhood.mx); it.More(); it.Next() )
  {
    const int          f_idx     = it.Key();
    const TopoDS_Face& face      = TopoDS::Face( Faces(f_idx) );
    const vtkIdType    vertex_id = result->AddVertex();
    //
    FaceVertexMap.Bind(f_idx, vertex_id);

    // Add address
    addressArr->InsertNextValue( asiAlgo_Utils::ShapeAddr(face) );

    // Fill property arrays
    std::string faceName = asiAlgo_Utils::FaceGeometryName(face);
    faceName += ": ";
    faceName += asiAlgo_Utils::Str::ToString<int>(f_idx).c_str();

    // Classify node depending on whether any attributes are available
    // for it or not
    if ( SelFaces.Contains(f_idx) )
    {
      groupArr->InsertNextValue(ARRNAME_GROUP_HIGHLIGHTED);
    }
    else if ( aag->HasNodeAttributes(f_idx) )
    {
      groupArr->InsertNextValue(ARRNAME_GROUP_ATTRIBUTED);

      // Loop over the available attributes
      for ( asiAlgo_AAG::t_attr_set::Iterator it2( aag->GetNodeAttributes(f_idx) );
            it2.More(); it2.Next() )
      {
        const Handle(asiAlgo_FeatureAttr)& attr = it2.GetAttr();
        faceName += " / ";
        faceName += attr->DynamicType()->Name();

        TCollection_AsciiString inlineDump = attr->DumpInline();
        if ( !inlineDump.IsEmpty() )
        {
          faceName += " / ";
          faceName += inlineDump.ToCString();
        }
      }
    }
    else
    {
      groupArr->InsertNextValue(ARRNAME_GROUP_ORDINARY);
    }

    // Add label
    labelArr->InsertNextValue(faceName);
  }

  // Set property arrays
  result->GetVertexData()->AddArray( labelArr  .GetPointer() );
  result->GetVertexData()->AddArray( addressArr.GetPointer() );
  result->GetVertexData()->AddArray( groupArr  .GetPointer() );
  result->GetEdgeData()  ->AddArray( angleArr  .GetPointer() );

  // Add links for adjacency relations
  for ( asiAlgo_AdjacencyMx::t_mx::Iterator it(Neighborhood.mx); it.More(); it.Next() )
  {
    const int       f_idx = it.Key();
    const vtkIdType v_idx = FaceVertexMap(f_idx);

    // Get face neighbors
    const TColStd_PackedMapOfInteger& neighbors = it.Value();

    // Add links for each neighbor
    for ( TColStd_MapIteratorOfPackedMapOfInteger mit(neighbors); mit.More(); mit.Next() )
    {
      const int       neighbor_f_idx = mit.Key();
      const vtkIdType neighbor_v_idx = FaceVertexMap(neighbor_f_idx);

      // Add link
      result->AddEdge(v_idx, neighbor_v_idx);

      // Check angle
      asiAlgo_AAG::t_arc arc(f_idx, neighbor_f_idx);
      Handle(asiAlgo_FeatureAttrAngle)
        attr = Handle(asiAlgo_FeatureAttrAngle)::DownCast( aag->GetArcAttribute(arc) );
      //
      if ( !attr.IsNull() )
        angleArr->InsertNextValue( attr->GetAngleType() );
    }
  }

  return result;
}
