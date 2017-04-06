//-----------------------------------------------------------------------------
// Created on: 21 March 2016
// Created by: Quaoar
//-----------------------------------------------------------------------------
// Web: http://dev.opencascade.org/, http://quaoar.su/blog
//-----------------------------------------------------------------------------

// Own include
#include <asiEngine_Part.h>

// asiVisu includes
#include <asiVisu_GeomPrs.h>

// asiAlgo includes
#include <asiAlgo_Utils.h>

// VTK includes
#include <vtkProperty.h>

// OCCT includes
#include <TColStd_MapIteratorOfPackedMapOfInteger.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>

//-----------------------------------------------------------------------------

//! Converts color value to an integer representation.
//! \param color [in] color.
//! \return converted value
static int ColorToInt(const QColor& color)
{
  unsigned char red   = (unsigned char) color.red();
  unsigned char green = (unsigned char) color.green();
  unsigned char blue  = (unsigned char) color.blue();
  return red << 16 | green << 8 | blue;
}

//-----------------------------------------------------------------------------

//! Convert integer value to a color.
//! \param color [in] integer value.
//! \return converted value
static QColor IntToColor(const int color)
{
  unsigned char red   = ( color >> 16 ) & 0xFF;
  unsigned char green = ( color >>  8 ) & 0xFF;
  unsigned char blue  =   color         & 0xFF;
  return QColor(red, green, blue);
}

//-----------------------------------------------------------------------------

//! \return newly created Part Node.
Handle(asiData_PartNode) asiEngine_Part::Create()
{
  // Add Part Node to Partition
  Handle(asiData_PartNode) geom_n = Handle(asiData_PartNode)::DownCast( asiData_PartNode::Instance() );
  m_model->GetPartPartition()->AddNode(geom_n);

  // Initialize geometry
  geom_n->Init();
  geom_n->SetName("Part");

  // Create underlying face representation Node
  {
    Handle(ActAPI_INode) geom_face_base = asiData_FaceNode::Instance();
    m_model->GetGeomFacePartition()->AddNode(geom_face_base);

    // Initialize
    Handle(asiData_FaceNode) geom_face_n = Handle(asiData_FaceNode)::DownCast(geom_face_base);
    geom_face_n->Init();
    geom_face_n->SetName("Face domain");

    // Set as child
    geom_n->AddChildNode(geom_face_n);
  }

  // Create underlying face norms representation Node
  {
    Handle(ActAPI_INode) geom_face_norms_base = asiData_FaceNormsNode::Instance();
    m_model->GetGeomFaceNormsPartition()->AddNode(geom_face_norms_base);

    // Initialize
    Handle(asiData_FaceNormsNode) geom_face_norms_n = Handle(asiData_FaceNormsNode)::DownCast(geom_face_norms_base);
    geom_face_norms_n->Init();
    geom_face_norms_n->SetName("Face norms");

    // Set as child
    geom_n->AddChildNode(geom_face_norms_n);
  }

  // Create underlying surface representation Node
  {
    Handle(ActAPI_INode) geom_surf_base = asiData_SurfNode::Instance();
    m_model->GetGeomSurfacePartition()->AddNode(geom_surf_base);

    // Initialize
    Handle(asiData_SurfNode) geom_surf_n = Handle(asiData_SurfNode)::DownCast(geom_surf_base);
    geom_surf_n->Init();
    geom_surf_n->SetName("Host surface");

    // Set as child
    geom_n->AddChildNode(geom_surf_n);
  }

  // Create underlying edge representation Node
  {
    Handle(ActAPI_INode) geom_edge_base = asiData_EdgeNode::Instance();
    m_model->GetGeomEdgePartition()->AddNode(geom_edge_base);

    // Initialize
    Handle(asiData_EdgeNode) geom_edge_n = Handle(asiData_EdgeNode)::DownCast(geom_edge_base);
    geom_edge_n->Init();
    geom_edge_n->SetName("Edge domain");

    // Set as child
    geom_n->AddChildNode(geom_edge_n);
  }

  // Create underlying curve representation Node
  {
    Handle(ActAPI_INode) geom_curve_base = asiData_CurveNode::Instance();
    m_model->GetGeomCurvePartition()->AddNode(geom_curve_base);

    // Initialize
    Handle(asiData_CurveNode) geom_curve_n = Handle(asiData_CurveNode)::DownCast(geom_curve_base);
    geom_curve_n->Init();
    geom_curve_n->SetName("Host curve");

    // Set as child
    geom_n->AddChildNode(geom_curve_n);
  }

  // Create underlying boundary edges representation Node
  {
    Handle(ActAPI_INode) geom_edges_base = asiData_BoundaryEdgesNode::Instance();
    m_model->GetGeomBoundaryEdgesPartition()->AddNode(geom_edges_base);

    // Initialize
    Handle(asiData_BoundaryEdgesNode) geom_edges_n = Handle(asiData_BoundaryEdgesNode)::DownCast(geom_edges_base);
    geom_edges_n->Init();
    geom_edges_n->SetName("Boundary");

    // Set as child
    geom_n->AddChildNode(geom_edges_n);
  }

  // Return the just created Node
  return geom_n;
}

//-----------------------------------------------------------------------------

//! Updates part's geometry in a smart way, so all dependent attributes
//! are also actualized.
//! \param model [in] CAD part to set.
void asiEngine_Part::Update(const TopoDS_Shape& model)
{
  // Get Part Node
  Handle(asiData_PartNode) part_n = m_model->GetPartNode();
  //
  if ( part_n.IsNull() || !part_n->IsWellFormed() )
    return;

  // Reset data
  Clean();

  // Set working structures
  part_n->SetShape(model);
  part_n->SetLinearDeflection( asiAlgo_Utils::AutoSelectLinearDeflection(model) );
  part_n->SetAngularDeflection( asiAlgo_Utils::AutoSelectAngularDeflection(model) );
  part_n->SetAAG( new asiAlgo_AAG(model) );
}

//-----------------------------------------------------------------------------

//! Cleans up Data Model structure related to the Part Node.
void asiEngine_Part::Clean()
{
  // Get Part Node
  Handle(asiData_PartNode) part_n = m_model->GetPartNode();
  //
  if ( part_n.IsNull() || !part_n->IsWellFormed() )
    return;

  // Reset data
  part_n                                   ->Init();
  part_n->GetFaceRepresentation()          ->Init();
  part_n->GetSurfaceRepresentation()       ->Init();
  part_n->GetEdgeRepresentation()          ->Init();
  part_n->GetCurveRepresentation()         ->Init();
  part_n->GetBoundaryEdgesRepresentation() ->Init();
}

//-----------------------------------------------------------------------------

//! Extracts sub-shape indices for the given collection of face indices.
//! \param faceIndices [in]  indices of faces.
//! \param indices     [out] their corresponding indices among all sub-shapes.
void asiEngine_Part::GetSubShapeIndicesByFaceIndices(const TColStd_PackedMapOfInteger& faceIndices,
                                                     TColStd_PackedMapOfInteger&       indices)
{
  const TopTools_IndexedMapOfShape&
    AllFaces = m_model->GetPartNode()->GetAAG()->GetMapOfFaces();
  //
  TopTools_IndexedMapOfShape SelectedFaces;

  // Get selected faces in topological form
  for ( TColStd_MapIteratorOfPackedMapOfInteger fit(faceIndices); fit.More(); fit.Next() )
  {
    const int input_face_idx = fit.Key();
    SelectedFaces.Add( AllFaces.FindKey(input_face_idx) );
  }

  // Get indices of the faces among all sub-shapes
  GetSubShapeIndices(SelectedFaces, indices);
}

//-----------------------------------------------------------------------------

//! Extracts sub-shape indices for the given collection of edge indices.
//! \param edgeIndices [in]  indices of edges.
//! \param indices     [out] their corresponding indices among all sub-shapes.
void asiEngine_Part::GetSubShapeIndicesByEdgeIndices(const TColStd_PackedMapOfInteger& edgeIndices,
                                                     TColStd_PackedMapOfInteger&       indices)
{
  const TopTools_IndexedMapOfShape&
    AllEdges = m_model->GetPartNode()->GetAAG()->GetMapOfEdges();
  //
  TopTools_IndexedMapOfShape SelectedEdges;

  // Get selected edges in topological form
  for ( TColStd_MapIteratorOfPackedMapOfInteger fit(edgeIndices); fit.More(); fit.Next() )
  {
    const int input_edge_idx = fit.Key();
    SelectedEdges.Add( AllEdges.FindKey(input_edge_idx) );
  }

  // Get indices of the edges among all sub-shapes
  GetSubShapeIndices(SelectedEdges, indices);
}

//-----------------------------------------------------------------------------

//! Extracts sub-shape indices for the given collection of sub-shapes.
//! \param subShapes [in]  sub-shapes of interest.
//! \param indices   [out] their corresponding IDs.
void asiEngine_Part::GetSubShapeIndices(const TopTools_IndexedMapOfShape& subShapes,
                                        TColStd_PackedMapOfInteger&       indices)
{
  const TopTools_IndexedMapOfShape&
    M = m_model->GetPartNode()->GetAAG()->GetMapOfSubShapes();
  //
  for ( int i = 1; i <= subShapes.Extent(); ++i )
    indices.Add( M.FindIndex( subShapes.FindKey(i) ) );
}

//-----------------------------------------------------------------------------

//! Extracts sub-shape indices for the given collection of sub-shapes. The
//! output is distributed by faces, edges and vertices.
//! \param subShapes     [in]  sub-shapes of interest.
//! \param faceIndices   [out] global indices for faces.
//! \param edgeIndices   [out] global indices for edges.
//! \param vertexIndices [out] global indices for vertices.
void asiEngine_Part::GetSubShapeIndices(const TopTools_IndexedMapOfShape& subShapes,
                                        TColStd_PackedMapOfInteger&       faceIndices,
                                        TColStd_PackedMapOfInteger&       edgeIndices,
                                        TColStd_PackedMapOfInteger&       vertexIndices)
{
  const TopTools_IndexedMapOfShape&
    M = m_model->GetPartNode()->GetAAG()->GetMapOfSubShapes();
  //
  for ( int i = 1; i <= subShapes.Extent(); ++i )
  {
    if ( subShapes.FindKey(i).ShapeType() == TopAbs_FACE )
      faceIndices.Add( M.FindIndex( subShapes.FindKey(i) ) );
    //
    else if ( subShapes.FindKey(i).ShapeType() == TopAbs_EDGE )
      edgeIndices.Add( M.FindIndex( subShapes.FindKey(i) ) );
    //
    else if ( subShapes.FindKey(i).ShapeType() == TopAbs_VERTEX )
      vertexIndices.Add( M.FindIndex( subShapes.FindKey(i) ) );
  }
}

//-----------------------------------------------------------------------------

//! Highlights a single face.
//! \param faceIndex [in] face to highlight.
//! \param color     [in] highlighting color.
void asiEngine_Part::HighlightFace(const int     faceIndex,
                                   const QColor& color)
{
  // Prepare a fictive collection
  TColStd_PackedMapOfInteger faceIndices;
  faceIndices.Add(faceIndex);

  // Highlight
  HighlightFaces(faceIndices, color);
}

//-----------------------------------------------------------------------------

//! Highlights faces.
//! \param faceIndices [in] faces to highlight.
//! \param color       [in] highlighting color.
void asiEngine_Part::HighlightFaces(const TColStd_PackedMapOfInteger& faceIndices,
                                    const QColor&                     color)
{
  // Convert face indices to sub-shape indices
  TColStd_PackedMapOfInteger ssIndices;
  GetSubShapeIndicesByFaceIndices(faceIndices, ssIndices);

  // Highlight
  HighlightSubShapes(ssIndices, ::ColorToInt(color), SelectionMode_Face);
}

//-----------------------------------------------------------------------------

//! Highlights edges.
//! \param edgeIndices [in] edges to highlight.
//! \param color       [in] highlighting color.
void asiEngine_Part::HighlightEdges(const TColStd_PackedMapOfInteger& edgeIndices,
                                    const QColor&                     color)
{
  // Convert edge indices to sub-shape indices
  TColStd_PackedMapOfInteger ssIndices;
  GetSubShapeIndicesByEdgeIndices(edgeIndices, ssIndices);

  // Highlight
  HighlightSubShapes(ssIndices, ::ColorToInt(color), SelectionMode_Edge);
}

//-----------------------------------------------------------------------------

//! Highlights the passed sub-shapes identified by their indices.
//! \param subShapeIndices [in] indices of the sub-shapes to highlight.
//! \param selMode         [in] selection mode.
void asiEngine_Part::HighlightSubShapes(const TColStd_PackedMapOfInteger& subShapeIndices,
                                        const asiVisu_SelectionMode       selMode)
{
  double pick_color[3];
  asiVisu_Utils::DefaultPickingColor(pick_color[0], pick_color[1], pick_color[2]);
  QColor color;
  color.setRedF   (pick_color[0]);
  color.setGreenF (pick_color[1]);
  color.setBlueF  (pick_color[2]);
  //
  HighlightSubShapes( subShapeIndices, ::ColorToInt(color), selMode );
}

//-----------------------------------------------------------------------------

//! Highlights the passed sub-shapes identified by their indices.
//! \param subShapeIndices [in] indices of the sub-shapes to highlight.
//! \param color           [in] highlighting color.
//! \param selMode         [in] selection mode.
void asiEngine_Part::HighlightSubShapes(const TColStd_PackedMapOfInteger& subShapeIndices,
                                        const int                         color,
                                        const asiVisu_SelectionMode       selMode)
{
  // Get Part Node
  Handle(asiData_PartNode) N = m_model->GetPartNode();

  // Get Presentation for the Part Node
  Handle(asiVisu_GeomPrs)
    prs = Handle(asiVisu_GeomPrs)::DownCast( m_prsMgr->GetPresentation(N) );

  // Prepare list to satisfy the API of Presentation Manager
  Handle(ActAPI_HNodeList) dummyList = new ActAPI_HNodeList;
  dummyList->Append(N);

  // Prepare selection object
  asiVisu_ActorElemMap selection;
  selection.Bind( prs->MainActor(), subShapeIndices );

  // Set color
  double prevColor[3];
  QColor qcolor = ::IntToColor(color);
  prs->GetPickPipeline()->Actor()->GetProperty()->GetColor( prevColor[0], prevColor[1], prevColor[2] );
  prs->GetPickPipeline()->Actor()->GetProperty()->SetColor( qcolor.redF(), qcolor.greenF(), qcolor.blueF() );

  // Highlight
  m_prsMgr->Highlight(dummyList, selection, selMode);

  // Restore previous color
  prs->GetPickPipeline()->Actor()->GetProperty()->SetColor( prevColor[0], prevColor[1], prevColor[2] );
}

//-----------------------------------------------------------------------------

//! Highlights the passed sub-shapes in Part Viewer.
//! \param subShapes [in] sub-shapes to highlight.
void asiEngine_Part::HighlightSubShapes(const TopTools_IndexedMapOfShape& subShapes)
{
  double pick_color[3];
  asiVisu_Utils::DefaultPickingColor(pick_color[0], pick_color[1], pick_color[2]);
  QColor color;
  color.setRedF   (pick_color[0]);
  color.setGreenF (pick_color[1]);
  color.setBlueF  (pick_color[2]);
  //
  HighlightSubShapes( subShapes, ::ColorToInt(color) );
}

//-----------------------------------------------------------------------------

//! Highlights the passed sub-shapes in Part Viewer.
//! \param subShapes [in] sub-shapes to highlight.
//! \param color     [in] highlighting color.
void asiEngine_Part::HighlightSubShapes(const TopTools_IndexedMapOfShape& subShapes,
                                        const int                         color)
{
  // Get global relative indices of the sub-shapes in the CAD model
  TColStd_PackedMapOfInteger selectedFaces, selectedEdges, selectedVertices;
  asiEngine_Part::GetSubShapeIndices(subShapes, selectedFaces, selectedEdges, selectedVertices);

  // Highlight
  if ( !selectedFaces.IsEmpty() )
    HighlightSubShapes(selectedFaces, color, SelectionMode_Face);
  //
  if ( !selectedEdges.IsEmpty() )
    HighlightSubShapes(selectedEdges, color, SelectionMode_Edge);
  //
  if ( !selectedVertices.IsEmpty() )
    HighlightSubShapes(selectedVertices, color, SelectionMode_Vertex);
}

//-----------------------------------------------------------------------------

//! Retrieves highlighted sub-shapes from the viewer.
//! \param subShapes [out] result collection.
void asiEngine_Part::GetHighlightedSubShapes(TopTools_IndexedMapOfShape& subShapes)
{
  // Get the map of ALL shapes to extract topology by selected index which
  // is global (related to full accessory graph)
  const TopTools_IndexedMapOfShape&
    M = m_model->GetPartNode()->GetAAG()->GetMapOfSubShapes();

  // Get actual selection
  const asiVisu_ActualSelection& sel      = m_prsMgr->GetCurrentSelection();
  const asiVisu_PickResult&      pick_res = sel.PickResult(SelectionNature_Pick);
  const asiVisu_ActorElemMap&    elem_map = pick_res.GetPickMap();
  //
  // Prepare cumulative set of all picked element IDs
  for ( asiVisu_ActorElemMap::Iterator it(elem_map); it.More(); it.Next() )
  {
    const TColStd_PackedMapOfInteger& subshape_mask = it.Value();
    //
    for ( TColStd_MapIteratorOfPackedMapOfInteger mit(subshape_mask); mit.More(); mit.Next() )
    {
      const int           subshape_idx = mit.Key();
      const TopoDS_Shape& F            = M.FindKey(subshape_idx);
      subShapes.Add(F);
    }
  }
}

//-----------------------------------------------------------------------------

//! Retrieves indices of the highlighted faces.
//! \param faceIndices [out] indices of the highlighted faces.
void asiEngine_Part::GetHighlightedFaces(TColStd_PackedMapOfInteger& faceIndices)
{
  TopTools_IndexedMapOfShape subShapes;
  GetHighlightedSubShapes(subShapes);

  // Take all faces
  const TopTools_IndexedMapOfShape&
    allFaces = m_model->GetPartNode()->GetAAG()->GetMapOfFaces();

  // Filter out non-selected faces
  for ( int f = 1; f <= allFaces.Extent(); ++f )
  {
    if ( subShapes.Contains( allFaces(f) ) )
      faceIndices.Add(f);
  }
}

//-----------------------------------------------------------------------------

//! Retrieves indices of the highlighted edges.
//! \param edgeIndices [out] indices of the highlighted edges.
void asiEngine_Part::GetHighlightedEdges(TColStd_PackedMapOfInteger& edgeIndices)
{
  TopTools_IndexedMapOfShape subShapes;
  GetHighlightedSubShapes(subShapes);

  // Take all edges
  const TopTools_IndexedMapOfShape&
    allEdges = m_model->GetPartNode()->GetAAG()->GetMapOfEdges();

  // Filter out non-selected edges
  for ( int e = 1; e <= allEdges.Extent(); ++e )
  {
    if ( subShapes.Contains( allEdges(e) ) )
      edgeIndices.Add(e);
  }
}
