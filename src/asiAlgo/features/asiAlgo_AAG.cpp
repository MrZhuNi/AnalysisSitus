//-----------------------------------------------------------------------------
// Created on: 26 February 2016
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
//    * Neither the name of Sergey Slyadnev nor the
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
#include <asiAlgo_AAG.h>

// asiAlgo includes
#include <asiAlgo_CheckDihedralAngle.h>
#include <asiAlgo_FeatureAttrAngle.h>
#include <asiAlgo_FeatureAttrFace.h>
#include <asiAlgo_JSON.h>
#include <asiAlgo_Utils.h>

// OCCT includes
#include <ShapeAnalysis_Edge.hxx>
#include <TColStd_MapIteratorOfPackedMapOfInteger.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>

//-----------------------------------------------------------------------------

asiAlgo_AAG::asiAlgo_AAG(const TopoDS_Shape&               masterCAD,
                         const TopTools_IndexedMapOfShape& selectedFaces,
                         const bool                        allowSmooth,
                         const double                      smoothAngularTol)
{
  this->init(masterCAD, selectedFaces, allowSmooth, smoothAngularTol);
}

//-----------------------------------------------------------------------------

asiAlgo_AAG::asiAlgo_AAG(const TopoDS_Shape& masterCAD,
                         const bool          allowSmooth,
                         const double        smoothAngularTol)
{
  this->init(masterCAD, TopTools_IndexedMapOfShape(), allowSmooth, smoothAngularTol);
}

//-----------------------------------------------------------------------------

asiAlgo_AAG::~asiAlgo_AAG()
{}

//-----------------------------------------------------------------------------

Handle(asiAlgo_AAG) asiAlgo_AAG::Copy() const
{
  Handle(asiAlgo_AAG) copy = new asiAlgo_AAG;
  //
  copy->m_master            = this->m_master;
  copy->m_selected          = this->m_selected;
  copy->m_subShapes         = this->m_subShapes;
  copy->m_faces             = this->m_faces;
  copy->m_edges             = this->m_edges;
  copy->m_vertices          = this->m_vertices;
  copy->m_neighbors         = this->m_neighbors;
  copy->m_arc_attributes    = this->m_arc_attributes;
  copy->m_node_attributes   = this->m_node_attributes;
  copy->m_bAllowSmooth      = this->m_bAllowSmooth;
  copy->m_fSmoothAngularTol = this->m_fSmoothAngularTol;
  //
  return copy;
}

//-----------------------------------------------------------------------------

const TopoDS_Shape& asiAlgo_AAG::GetMasterCAD() const
{
  return m_master;
}

//-----------------------------------------------------------------------------

void asiAlgo_AAG::GetMapOf(const TopAbs_ShapeEnum      ssType,
                           TopTools_IndexedMapOfShape& map) const
{
  switch ( ssType )
  {
    case TopAbs_VERTEX:
      map = this->GetMapOfVertices();
      break;
    case TopAbs_EDGE:
      map = this->GetMapOfEdges();
      break;
    case TopAbs_FACE:
      map = this->GetMapOfFaces();
      break;
    default: break;
  }
}

//-----------------------------------------------------------------------------

const TopTools_IndexedMapOfShape& asiAlgo_AAG::GetMapOfFaces() const
{
  return m_faces;
}

//-----------------------------------------------------------------------------

const TopTools_IndexedMapOfShape& asiAlgo_AAG::GetMapOfEdges() const
{
  return m_edges;
}

//-----------------------------------------------------------------------------

const TopTools_IndexedMapOfShape& asiAlgo_AAG::GetMapOfVertices() const
{
  return m_vertices;
}

//-----------------------------------------------------------------------------

const TopTools_IndexedMapOfShape& asiAlgo_AAG::GetMapOfSubShapes() const
{
  return m_subShapes;
}

//-----------------------------------------------------------------------------

bool asiAlgo_AAG::HasFace(const int face_idx) const
{
  return face_idx > 0 && face_idx <= m_faces.Extent();
}

//-----------------------------------------------------------------------------

const TopoDS_Face& asiAlgo_AAG::GetFace(const int face_idx) const
{
  return TopoDS::Face( m_faces.FindKey(face_idx) );
}

//-----------------------------------------------------------------------------

int asiAlgo_AAG::GetFaceId(const TopoDS_Shape& face) const
{
  return m_faces.FindIndex(face);
}

//-----------------------------------------------------------------------------

bool asiAlgo_AAG::HasNeighbors(const int face_idx) const
{
  return m_neighbors.IsBound(face_idx);
}

//-----------------------------------------------------------------------------

const TColStd_PackedMapOfInteger& asiAlgo_AAG::GetNeighbors(const int face_idx) const
{
  return m_neighbors.Find(face_idx);
}

//-----------------------------------------------------------------------------

TColStd_PackedMapOfInteger
  asiAlgo_AAG::GetNeighborsThru(const int face_idx, const TopoDS_Edge& edge) const
{
  TColStd_PackedMapOfInteger result;

  // Get all neighbors of the face of interest
  const TColStd_PackedMapOfInteger& neighbors = this->GetNeighbors(face_idx);

  // Traverse all neighborhood arcs to see if there are any containing
  // the edge of interest in the list of common edges
  for ( TColStd_MapIteratorOfPackedMapOfInteger nit(neighbors); nit.More(); nit.Next() )
  {
    const int neighbor_idx = nit.Key();

    // Get neighborhood attribute
    Handle(asiAlgo_FeatureAttrAdjacency)
      adjAttr = Handle(asiAlgo_FeatureAttrAdjacency)::DownCast( this->GetArcAttribute( t_arc(face_idx, neighbor_idx) ) );
    //
    if ( adjAttr.IsNull() )
      continue;

    // Check the collection of common edges
    const TopTools_IndexedMapOfShape& commonEdges = adjAttr->GetEdges();
    //
    if ( commonEdges.Contains(edge) )
      result.Add(neighbor_idx);
  }

  return result;
}

//-----------------------------------------------------------------------------

TColStd_PackedMapOfInteger
  asiAlgo_AAG::GetNeighbors(const int                         face_idx,
                            const TColStd_PackedMapOfInteger& edge_ids) const
{
  TColStd_PackedMapOfInteger result;

  // Get all edges
  TopTools_IndexedMapOfShape allEdges;
  TopExp::MapShapes(m_master, TopAbs_EDGE, allEdges);

  // Get neighbor faces
  const TColStd_PackedMapOfInteger& neighbor_ids = this->GetNeighbors(face_idx);
  //
  for ( TColStd_MapIteratorOfPackedMapOfInteger nit(neighbor_ids); nit.More(); nit.Next() )
  {
    const int neighbor_idx = nit.Key();

    // Check arc attribute
    Handle(asiAlgo_FeatureAttr) attr = this->GetArcAttribute( t_arc(face_idx, neighbor_idx) );
    if ( !attr->IsKind( STANDARD_TYPE(asiAlgo_FeatureAttrAdjacency) ) )
      continue;

    // Convert to adjacency attribute
    Handle(asiAlgo_FeatureAttrAdjacency)
      adjAttr = Handle(asiAlgo_FeatureAttrAdjacency)::DownCast(attr);
    //
    const TopTools_IndexedMapOfShape& edges = adjAttr->GetEdges();

    // Take the index of each edge and check if this edge is of interest
    for ( int eidx = 1; eidx <= edges.Extent(); ++eidx )
    {
      const int globalIdx = allEdges.FindIndex( edges(eidx) );
      //
      if ( edge_ids.Contains(globalIdx) )
      {
        result.Add(neighbor_idx);
        break;
      }
    }
  }

  return result;
}

//-----------------------------------------------------------------------------

const asiAlgo_AAG::t_adjacency& asiAlgo_AAG::GetNeighborhood() const
{
  return m_neighbors;
}

//-----------------------------------------------------------------------------

const TColStd_PackedMapOfInteger& asiAlgo_AAG::GetSelectedFaces() const
{
  return m_selected;
}

//-----------------------------------------------------------------------------

const asiAlgo_AAG::t_arc_attributes& asiAlgo_AAG::GetArcAttributes() const
{
  return m_arc_attributes;
}

//-----------------------------------------------------------------------------

const Handle(asiAlgo_FeatureAttr)& asiAlgo_AAG::GetArcAttribute(const t_arc& arc) const
{
  return m_arc_attributes.Find(arc);
}

//-----------------------------------------------------------------------------

bool asiAlgo_AAG::HasNodeAttributes(const int node) const
{
  return m_node_attributes.IsBound(node);
}

//-----------------------------------------------------------------------------

const asiAlgo_AAG::t_attr_set& asiAlgo_AAG::GetNodeAttributes(const int node) const
{
  return m_node_attributes(node);
}

//-----------------------------------------------------------------------------

Handle(asiAlgo_FeatureAttr)
  asiAlgo_AAG::GetNodeAttribute(const int            node,
                                const Standard_GUID& attr_id) const
{
  if ( !m_node_attributes.IsBound(node) )
    return NULL;

  if ( !m_node_attributes(node).GetMap().IsBound(attr_id) )
    return NULL;

  return m_node_attributes(node)(attr_id);
}

//-----------------------------------------------------------------------------

bool asiAlgo_AAG::SetNodeAttribute(const int                          node,
                                   const Handle(asiAlgo_FeatureAttr)& attr)
{
  if ( attr.IsNull() )
    return false;

  Handle(asiAlgo_FeatureAttr) existing = this->GetNodeAttribute( node, attr->GetGUID() );
  //
  if ( !existing.IsNull() )
    return false; // Already there

  // Set owner AAG
  attr->setAAG(this);

  // Set face ID to the attribute representing a feature face
  if ( attr->IsKind( STANDARD_TYPE(asiAlgo_FeatureAttrFace) ) )
    Handle(asiAlgo_FeatureAttrFace)::DownCast(attr)->setFaceId(node);

  if ( !m_node_attributes.IsBound(node) )
    m_node_attributes.Bind( node, t_attr_set(attr) );
  else
  {
    t_attr_set& attr_set = m_node_attributes.ChangeFind(node);
    attr_set.Add(attr);
  }

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_AAG::FindConvexOnly(TopTools_IndexedMapOfShape& resultFaces) const
{
  TColStd_PackedMapOfInteger traversed;
  for ( t_adjacency::Iterator it(m_neighbors); it.More(); it.Next() )
  {
    const int                         current_face_idx       = it.Key();
    const TColStd_PackedMapOfInteger& current_face_neighbors = it.Value();

    // Mark face as traversed
    if ( !traversed.Contains(current_face_idx) )
      traversed.Add(current_face_idx);
    else
      continue;

    // Loop over the neighbors
    bool isAllConvex = true;
    for ( TColStd_MapIteratorOfPackedMapOfInteger nit(current_face_neighbors); nit.More(); nit.Next() )
    {
      const int neighbor_face_idx = nit.Key();

      // Get angle attribute
      Handle(asiAlgo_FeatureAttrAngle)
        attr = Handle(asiAlgo_FeatureAttrAngle)::DownCast( this->GetArcAttribute( t_arc(current_face_idx,
                                                                                        neighbor_face_idx) ) );

      if ( attr->GetAngle() != FeatureAngleType_Convex )
      {
        isAllConvex = false;

        // Mark face as traversed as we don't want to check concave neighbors
        traversed.Add(neighbor_face_idx);
      }
    }

    if ( isAllConvex )
      resultFaces.Add( this->GetFace(current_face_idx) );
  }

  return resultFaces.Extent() > 0;
}

//-----------------------------------------------------------------------------

bool asiAlgo_AAG::FindConcaveOnly(TopTools_IndexedMapOfShape& resultFaces) const
{
  TColStd_PackedMapOfInteger traversed;
  for ( t_adjacency::Iterator it(m_neighbors); it.More(); it.Next() )
  {
    const int                         current_face_idx       = it.Key();
    const TColStd_PackedMapOfInteger& current_face_neighbors = it.Value();

    // Mark face as traversed
    if ( !traversed.Contains(current_face_idx) )
      traversed.Add(current_face_idx);
    else
      continue;

    // Loop over the neighbors
    bool isAllConcave = true;
    for ( TColStd_MapIteratorOfPackedMapOfInteger nit(current_face_neighbors); nit.More(); nit.Next() )
    {
      const int neighbor_face_idx = nit.Key();

      // Get angle attribute
      Handle(asiAlgo_FeatureAttrAngle)
        attr = Handle(asiAlgo_FeatureAttrAngle)::DownCast( this->GetArcAttribute( t_arc(current_face_idx,
                                                                                        neighbor_face_idx) ) );

      if ( attr->GetAngle() != FeatureAngleType_Concave )
      {
        isAllConcave = false;

        // Mark face as traversed as we don't want to check concave neighbors
        traversed.Add(neighbor_face_idx);
      }
    }

    if ( isAllConcave )
      resultFaces.Add( this->GetFace(current_face_idx) );
  }

  return resultFaces.Extent() > 0;
}


//-----------------------------------------------------------------------------

void asiAlgo_AAG::Remove(const TopTools_IndexedMapOfShape& faces)
{
  // NOTICE: indexed map of shapes is not affected as we want to keep
  //         using the original indices of faces

  // Find IDs of the faces to remove
  TColStd_PackedMapOfInteger toRemove;
  for ( int f = 1; f <= faces.Extent(); ++f )
  {
    const int face_idx = this->GetFaceId( faces.FindKey(f) );
    toRemove.Add(face_idx);
  }

  // Loop over the target faces
  for ( TColStd_MapIteratorOfPackedMapOfInteger fit(toRemove); fit.More(); fit.Next() )
  {
    const int face_idx = fit.Key();

    // Unbind node attributes
    m_node_attributes.UnBind(face_idx);

    // Find all neighbors
    const TColStd_PackedMapOfInteger& neighbor_indices = m_neighbors.Find(face_idx);
    for ( TColStd_MapIteratorOfPackedMapOfInteger nit(neighbor_indices); nit.More(); nit.Next() )
    {
      const int neighbor_idx = nit.Key();

      // Unbind arc attributes
      m_arc_attributes.UnBind( t_arc(face_idx, neighbor_idx) );

      // Kill the faces being removed from the list of neighbors
      if ( m_neighbors.IsBound(neighbor_idx) )
        m_neighbors.ChangeFind(neighbor_idx).Subtract(toRemove);
    }

    // Unbind node
    m_neighbors.UnBind(face_idx);
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_AAG::Dump(Standard_OStream& out) const
{
  out << "===================================================\n";
  out << "***AAG structure\n";
  out << "---------------------------------------------------\n";
  out << " Adjacency\n";
  out << "---------------------------------------------------\n";

  // Dump neighborhood
  for ( int f = 1; f <= m_faces.Extent(); ++f )
  {
    out << "\t" << f << " -> ";
    const TColStd_PackedMapOfInteger& neighbors = this->GetNeighbors(f);
    //
    for ( TColStd_MapIteratorOfPackedMapOfInteger nit(neighbors); nit.More(); nit.Next() )
    {
      out << nit.Key() << " ";
    }
    out << "\n";
  }

  // Dump arc attributes
  out << "---------------------------------------------------\n";
  out << " Node attributes\n";
  out << "---------------------------------------------------\n";
  for ( int f = 1; f <= m_faces.Extent(); ++f )
  {
    if ( !this->HasNodeAttributes(f) )
      continue;

    const NCollection_DataMap<Standard_GUID, Handle(asiAlgo_FeatureAttr), Standard_GUID>& attrs = this->GetNodeAttributes(f).GetMap();
    //
    if ( attrs.IsEmpty() )
      continue;

    out << "\t" << f << " ~ ";
    //
    for ( NCollection_DataMap<Standard_GUID, Handle(asiAlgo_FeatureAttr), Standard_GUID>::Iterator ait(attrs); ait.More(); ait.Next() )
    {
      out << "[" << ait.Value()->DynamicType()->Name() << "]\n";
      out << ">>>\n";
      ait.Value()->Dump(out);
      out << "\n<<<\n";
    }
    out << "\n";
  }
  out << "===================================================\n";
}

//-----------------------------------------------------------------------------

void asiAlgo_AAG::DumpJSON(Standard_OStream& out) const
{
  out << std::setprecision( std::numeric_limits<double>::max_digits10 );
  out << "{";
  out <<  "\n  \"nodes\": {";
  //
  this->dumpNodesJSON(out);
  //
  out << "\n  },"; // End 'nodes'.
  //
  out <<  "\n  \"arcs\": [";
  //
  this->dumpArcsJSON(out);
  //
  out << "\n  ]"; // End 'arcs'.
  out << "\n}\n";
}

//-----------------------------------------------------------------------------

void asiAlgo_AAG::init(const TopoDS_Shape&               masterCAD,
                       const TopTools_IndexedMapOfShape& selectedFaces,
                       const bool                        allowSmooth,
                       const double                      smoothAngularTol)
{
  m_master            = masterCAD;
  m_bAllowSmooth      = allowSmooth;
  m_fSmoothAngularTol = smoothAngularTol;

  if ( masterCAD.IsNull() )
    return;

  //---------------------------------------------------------------------------

  ShapeAnalysis_Edge sae;

  // Extract all sub-shapes with unique indices from the master CAD
  TopExp::MapShapes(masterCAD, m_subShapes);

  // Extract all faces with unique indices from the master CAD
  TopExp::MapShapes(masterCAD, TopAbs_FACE, m_faces);

  // Extract all edges with unique indices from the master CAD
  TopExp::MapShapes(masterCAD, TopAbs_EDGE, m_edges);

  // Extract all vertices with unique indices from the master CAD
  TopExp::MapShapes(masterCAD, TopAbs_VERTEX, m_vertices);

  // Fill adjacency map with empty buckets and provide all required
  // treatment for each individual face
  for ( int f = 1; f <= m_faces.Extent(); ++f )
  {
    m_neighbors.Bind( f, TColStd_PackedMapOfInteger() );
    //
    const TopoDS_Face& face = TopoDS::Face( m_faces(f) );

    // Special treatment deserve those faces having seam edges. Such faces
    // get their own attributes
    for ( TopExp_Explorer exp(face, TopAbs_EDGE); exp.More(); exp.Next() )
    {
      const TopoDS_Edge& edge = TopoDS::Edge( exp.Current() );
      //
      if ( sae.IsSeam(edge, face) )
      {
        TopTools_IndexedMapOfShape edges;
        asiAlgo_CheckDihedralAngle checkDihAngle(NULL, NULL);

        // Notice that smooth transitions are not allowed here. This is because
        // the following treatment is designed for periodic faces, and we normally
        // have self-transition of quality C1 and better there
        double angRad = 0.0;
        //
        const asiAlgo_FeatureAngleType
          face_angle = checkDihAngle.AngleBetweenFaces(face, face, false, 0.0, edges, angRad);

        // Bind attribute representing the type of dihedral angle. This is an
        // exceptional case as normally such attributes are bound to arcs.
        m_node_attributes.Bind( f, t_attr_set( new asiAlgo_FeatureAttrAngle(face_angle) ) );
      }
    }
  }

  //---------------------------------------------------------------------------

  TopTools_IndexedDataMapOfShapeListOfShape ChildParentMap;
  TopExp::MapShapesAndAncestors(masterCAD, TopAbs_EDGE, TopAbs_FACE, ChildParentMap);

  // Build adjacency graph
  for ( TopExp_Explorer exp(masterCAD, TopAbs_EDGE); exp.More(); exp.Next() )
  {
    const TopoDS_Edge&          commonEdge    = TopoDS::Edge( exp.Current() );
    const TopTools_ListOfShape& adjacentFaces = ChildParentMap.FindFromKey(commonEdge);
    //
    this->addMates(adjacentFaces);
  }

  //---------------------------------------------------------------------------

  // Save selected faces for future filtering
  for ( int s = 1; s <= selectedFaces.Extent(); ++s )
  {
    m_selected.Add( m_faces.FindIndex( selectedFaces.FindKey(s) ) );
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_AAG::addMates(const TopTools_ListOfShape& mateFaces)
{
  // Create solid angle calculation tool
  asiAlgo_CheckDihedralAngle checkDihAngle(NULL, NULL);

  // Now analyze the face pairs
  for ( TopTools_ListIteratorOfListOfShape lit(mateFaces); lit.More(); lit.Next() )
  {
    const int                   face_idx   = m_faces.FindIndex( lit.Value() );
    TColStd_PackedMapOfInteger& face_links = m_neighbors.ChangeFind(face_idx);
    const TopoDS_Face&          face       = TopoDS::Face( m_faces.FindKey(face_idx) );

    // Add all the rest faces as neighbors
    for ( TopTools_ListIteratorOfListOfShape lit2(mateFaces); lit2.More(); lit2.Next() )
    {
      const int linked_face_idx = m_faces.FindIndex( lit2.Value() );

      if ( linked_face_idx == face_idx )
        continue; // Skip the same index to avoid loop arcs in the graph

      if ( face_links.Contains(linked_face_idx) )
        continue;

      face_links.Add(linked_face_idx);

      // The graph is not oriented, so we do not want to compute arc
      // attribute G-F is previously we have already done F-G attribution
      t_arc arc(face_idx, linked_face_idx);
      if ( m_arc_attributes.IsBound(arc) )
        continue;

      //-----------------------------------------------------------------------
      // Associate attributes
      //-----------------------------------------------------------------------

      const TopoDS_Face& linked_face = TopoDS::Face( m_faces.FindKey(linked_face_idx) );
      //
      TopTools_IndexedMapOfShape commonEdges;

      // Here we let client code decide whether to allow smooth transitions
      // or not. Smooth transition normally requires additional processing
      // in order to classify feature angle as concave or convex
      double angRad = 0.0;
      //
      const asiAlgo_FeatureAngleType
        angle = checkDihAngle.AngleBetweenFaces(face,
                                                linked_face,
                                                m_bAllowSmooth,
                                                m_fSmoothAngularTol,
                                                commonEdges,
                                                angRad);

      // Bind attribute
      m_arc_attributes.Bind( arc, new asiAlgo_FeatureAttrAngle(angle, commonEdges) );
    }
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_AAG::dumpNodesJSON(Standard_OStream& out) const
{
  const int numNodes = m_neighbors.Extent();
  int nidx = 0;
  //
  for ( t_adjacency::Iterator nit(m_neighbors); nit.More(); nit.Next(), ++nidx )
  {
    const int nodeId = nit.Key();
    //
    this->dumpNodeJSON(nodeId, out);
    //
    if ( nidx < numNodes - 1 )
      out << ",";
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_AAG::dumpNodeJSON(const int         node,
                               Standard_OStream& out) const
{
  // One attribute which should always be dumped is the surface type.
  std::string
    surfName = asiAlgo_Utils::SurfaceName( BRep_Tool::Surface( this->GetFace(node) ) );

  out << "\n    \"" << node << "\": {";
  out << "\n      \"surface\": \"" << surfName << "\"";
  out << "\n    }";
}

//-----------------------------------------------------------------------------

void asiAlgo_AAG::dumpArcsJSON(Standard_OStream& out) const
{
  // Map to filter out the already visited arcs.
  NCollection_Map<t_arc, t_arc> visited;

  int arcidx = 0;
  //
  for ( t_adjacency::Iterator it(m_neighbors); it.More(); it.Next() )
  {
    const int f_idx = it.Key();

    // Get neighbors.
    const TColStd_PackedMapOfInteger& localNeighbors = it.Value();

    // Dump arc for each neighbor.
    for ( TColStd_MapIteratorOfPackedMapOfInteger mit(localNeighbors); mit.More(); mit.Next(), ++arcidx )
    {
      const int neighbor_f_idx = mit.Key();

      // Check if the arc was not traversed before.
      t_arc arc(f_idx, neighbor_f_idx);
      //
      if ( visited.Contains(arc) )
        continue;
      //
      visited.Add(arc);

      // Dump arc.
      this->dumpArcJSON(arc, arcidx == 0, out);
    }
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_AAG::dumpArcJSON(const t_arc&      arc,
                              const bool        isFirst,
                              Standard_OStream& out) const
{
  Handle(asiAlgo_FeatureAttr) arcAttr = this->GetArcAttribute(arc);
  //
  Handle(asiAlgo_FeatureAttrAngle)
    arcAttrAngle = Handle(asiAlgo_FeatureAttrAngle)::DownCast(arcAttr);

  // Prepare a label for the angle type.
  std::string angleTypeStr;
  //
  if ( arcAttrAngle->GetAngle() == FeatureAngleType_Convex )
    angleTypeStr = "convex";
  else if ( arcAttrAngle->GetAngle() == FeatureAngleType_Concave )
    angleTypeStr = "concave";
  else if ( arcAttrAngle->GetAngle() == FeatureAngleType_Smooth )
    angleTypeStr = "smooth";
  else
    angleTypeStr = "undefined";

  if ( !isFirst )
    out << ",";
  //
  out << "\n    [\"" << arc.F1 << "\", \"" << arc.F2 << "\", \"" << angleTypeStr << "\"]";
}
