//-----------------------------------------------------------------------------
// Created on: 30 November 2016
// Created by: Quaoar
//-----------------------------------------------------------------------------
// Web: http://dev.opencascade.org/, http://quaoar.su/blog
//-----------------------------------------------------------------------------

// Own include
#include <asiVisu_ShapeRobustTessellator.h>

// asiVisu includes
#include <asiVisu_CurveSource.h>

// asiAlgo includes
#include <asiAlgo_MeshGen.h>
#include <asiAlgo_MeshMerge.h>
#include <asiAlgo_Timer.h>
#include <asiAlgo_Utils.h>

// OCCT includes
#include <BRep_Tool.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

#undef COUT_DEBUG
#if defined COUT_DEBUG
  #pragma message("===== warning: COUT_DEBUG is enabled")
#endif

//-----------------------------------------------------------------------------

vtkStandardNewMacro(asiVisu_ShapeRobustTessellator);

//-----------------------------------------------------------------------------

//! ctor.
asiVisu_ShapeRobustTessellator::asiVisu_ShapeRobustTessellator()
: vtkObject()
{
  m_data = new asiVisu_ShapeData; // Allocate data for visualization primitives
                                  // (i.e. VTK points, cells, arrays)
}

//-----------------------------------------------------------------------------

//! Initializes tessellation tool.
//! \param aag                   [in] AAG.
//! \param linearDeflection      [in] linear deflection.
//! \param angularDeflection_deg [in] angular deflection in degrees.
//! \param progress              [in] progress notifier.
//! \param plotter               [in] imperative plotter.
void asiVisu_ShapeRobustTessellator::Initialize(const Handle(asiAlgo_AAG)& aag,
                                                const double               linearDeflection,
                                                const double               angularDeflection_deg,
                                                ActAPI_ProgressEntry       progress,
                                                ActAPI_PlotterEntry        plotter)
{
  m_aag      = aag;
  m_progress = progress;
  m_plotter  = plotter;

  // Set linear deflection
  if ( Abs(linearDeflection) < asiAlgo_TooSmallValue )
    m_fLinDeflection = asiAlgo_Utils::AutoSelectLinearDeflection( m_aag->GetMasterCAD() );
  else
    m_fLinDeflection = linearDeflection;
  //
  m_progress.SendLogMessage(LogInfo(Normal) << "Faceter linear deflection is %1" << m_fLinDeflection);

  // Set angular deflection
  if ( Abs(angularDeflection_deg) < asiAlgo_TooSmallValue )
    m_fAngDeflectionDeg = asiAlgo_Utils::AutoSelectAngularDeflection( m_aag->GetMasterCAD() );
  else
    m_fAngDeflectionDeg = angularDeflection_deg;
  //
  m_progress.SendLogMessage(LogInfo(Normal) << "Faceter angular deflection is %1 deg." << m_fAngDeflectionDeg);
}

//-----------------------------------------------------------------------------

//! Builds the polygonal data.
void asiVisu_ShapeRobustTessellator::Build()
{
  this->internalBuild();
}

//-----------------------------------------------------------------------------

//! Internal method which builds the polygonal data.
void asiVisu_ShapeRobustTessellator::internalBuild()
{
  const TopoDS_Shape& master = m_aag->GetMasterCAD();
  //
  if ( master.IsNull() )
    return;

  TIMER_NEW
  TIMER_GO

  // Build map of shapes and their parents
  TopTools_IndexedDataMapOfShapeListOfShape verticesOnEdges;
  TopExp::MapShapesAndAncestors(master, TopAbs_VERTEX, TopAbs_EDGE, verticesOnEdges);
  //
  TopTools_IndexedDataMapOfShapeListOfShape edgesOnFaces;
  TopExp::MapShapesAndAncestors(master, TopAbs_EDGE, TopAbs_FACE, edgesOnFaces);

  // Cached maps
  const TopTools_IndexedMapOfShape& allVertices = m_aag->GetMapOfVertices();
  const TopTools_IndexedMapOfShape& allEdges    = m_aag->GetMapOfEdges();
  const TopTools_IndexedMapOfShape& allFaces    = m_aag->GetMapOfFaces();

  TIMER_FINISH
  TIMER_COUT_RESULT_MSG("asiVisu_ShapeRobustTessellator: build topology maps")

  /* =========================================
   *  STAGE 1: fill data source with vertices
   * ========================================= */

  //// Add vertices
  //for ( int vidx = 1; vidx <= allVertices.Extent(); ++vidx )
  //{
  //  const TopoDS_Vertex&            v = TopoDS::Vertex( allVertices(vidx) );
  //  const TopTools_ListOfShape& edges = verticesOnEdges.FindFromKey(v);

  //  asiVisu_ShapeCellType type;
  //  if ( edges.IsEmpty() )
  //    type = ShapeCellType_FreeVertex;
  //  else
  //    type = ShapeCellType_SharedVertex;

  //  // Add vertex to the data source
  //  this->addVertex(v, vidx, type);
  //}

  /* ===============
   *  STAGE 2: mesh
   * =============== */

  TIMER_RESET
  TIMER_GO

  // Discretize B-Rep model to produce visualization facets
  asiAlgo_MeshInfo meshInfo;
  if ( !asiAlgo_MeshGen::DoNative(master,
                                  m_fLinDeflection,
                                  m_fAngDeflectionDeg,
                                  meshInfo) )
  {
    vtkErrorMacro( << "Cannot tessellate B-Rep model with linear deflection " << m_fLinDeflection
                   << " and angular deflection " << m_fAngDeflectionDeg << " [deg]");
    return;
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_MSG("asiVisu_ShapeRobustTessellator: B-Rep mesh")

  /* =========================
   *  STAGE 3: conglomeration
   * ========================= */

  //TIMER_RESET
  //TIMER_GO

  //// Prepare conglomeration of meshes
  //asiAlgo_MeshMerge conglomerate(master);
  ////
  //const Handle(Poly_CoherentTriangulation)& cohTriangulation = conglomerate.GetResultPoly();

  //TIMER_FINISH
  //TIMER_COUT_RESULT_MSG("asiVisu_ShapeRobustTessellator: conglomeration")

  ///* ======================
  // *  STAGE 4: fill points
  // * ====================== */

  //TIMER_RESET
  //TIMER_GO

  //// Loop over the nodes and fill geometric VTK data (points)
  //for ( Poly_CoherentTriangulation::IteratorOfNode nit(cohTriangulation); nit.More(); nit.Next() )
  //{
  //  const Poly_CoherentNode& cohNode = nit.Value();
  //  m_data->InsertCoordinate( cohNode.X(),
  //                            cohNode.Y(),
  //                            cohNode.Z() );
  //}

  //TIMER_FINISH
  //TIMER_COUT_RESULT_MSG("asiVisu_ShapeRobustTessellator: fill VTK points")

  ///* ======================================
  // *  STAGE 5: create VTK topology (cells)
  // * ====================================== */

  //TIMER_RESET
  //TIMER_GO

  //// Here we take advantage of the fact the indices of nodes in coherent
  //// triangulation are exactly the same as PIDs inserted above

  //// Add free links
  //const asiAlgo_MeshMerge::t_link_set& freeLinks = conglomerate.GetFreeLinks();
  ////
  //for ( int i = 1; i <= freeLinks.Extent(); ++i )
  //{
  //  const asiAlgo_MeshMerge::t_unoriented_link& link = freeLinks(i);
  //  //
  //  m_data->InsertLine(0, link.N1, link.N2, ShapePrimitive_FreeEdge);
  //}

  //// Add manifold links
  //const asiAlgo_MeshMerge::t_link_set& manifoldLinks = conglomerate.GetManifoldLinks();
  ////
  //for ( int i = 1; i <= manifoldLinks.Extent(); ++i )
  //{
  //  const asiAlgo_MeshMerge::t_unoriented_link& link = manifoldLinks(i);
  //  //
  //  m_data->InsertLine(0, link.N1, link.N2, ShapePrimitive_ManifoldEdge);
  //}

  //// Add non-manifold links
  //const asiAlgo_MeshMerge::t_link_set& nonManifoldLinks = conglomerate.GetNonManifoldLinks();
  ////
  //for ( int i = 1; i <= nonManifoldLinks.Extent(); ++i )
  //{
  //  const asiAlgo_MeshMerge::t_unoriented_link& link = nonManifoldLinks(i);
  //  //
  //  m_data->InsertLine(0, link.N1, link.N2, ShapePrimitive_NonManifoldEdge);
  //}

  //// Add facets for shading
  //for ( Poly_CoherentTriangulation::IteratorOfTriangle fit(cohTriangulation); fit.More(); fit.Next() )
  //{
  //  const Poly_CoherentTriangle& cohTriangle = fit.Value();
  //  m_data->InsertTriangle(0,
  //                         cohTriangle.Node(0),
  //                         cohTriangle.Node(1),
  //                         cohTriangle.Node(2),
  //                         ShapePrimitive_Facet);
  //}

  //TIMER_FINISH
  //TIMER_COUT_RESULT_MSG("asiVisu_ShapeRobustTessellator: fill VTK cells")

  /* ======================================
   *  STAGE 3: fill data source with edges
   * ====================================== */

  // Add edges
  for ( int eidx = 1; eidx <= allEdges.Extent(); ++eidx )
  {
    const TopoDS_Edge&              e = TopoDS::Edge( allEdges(eidx) );
    const TopTools_ListOfShape& faces = edgesOnFaces.FindFromKey(e);

    asiVisu_ShapePrimitive type;
    if ( faces.Extent() == 1 )
      type = ShapePrimitive_FreeEdge;
    else if ( faces.Extent() > 2 )
      type = ShapePrimitive_NonManifoldEdge;
    else
      type = ShapePrimitive_ManifoldEdge;

    // Add edge to the data source
    this->addEdge(e, eidx, type);
  }

  /* ======================================
   *  STAGE 4: fill data source with faces
   * ====================================== */

  // Loop over the faces
  for ( TopExp_Explorer exp(master, TopAbs_FACE); exp.More(); exp.Next() )
  {
    const TopoDS_Face& face = TopoDS::Face( exp.Current() );
    //
    if ( face.IsNull() )
      continue;

    // Get facets associated with the face
    TopLoc_Location loc;
    const Handle(Poly_Triangulation)& triangulation = BRep_Tool::Triangulation(face, loc);
    //
    if ( triangulation.IsNull() )
      continue;

    // Populate node
    NCollection_DataMap<int, vtkIdType> pointIds;
    const TColgp_Array1OfPnt& nodes = triangulation->Nodes();
    //
    for ( int pidx = nodes.Lower(); pidx <= nodes.Upper(); ++pidx )
    {
      gp_Pnt P = nodes(pidx);
      //
      if ( !loc.IsIdentity() )
        P.Transform(loc);

      // Insert VTK point
      vtkIdType pid = m_data->InsertCoordinate( P.X(), P.Y(), P.Z() );
      //
      pointIds.Bind(pidx, pid);
    }

    // Create triangle cells
    const Poly_Array1OfTriangle& triangles = triangulation->Triangles();
    //
    for ( int t = triangles.Lower(); t <= triangles.Upper(); ++t )
    {
      const Poly_Triangle& triangle = triangles(t);

      int n1, n2, n3;
      triangle.Get(n1, n2, n3);

      // Insert VTK cell
      m_data->InsertTriangle(0,
                             pointIds(n1),
                             pointIds(n2),
                             pointIds(n3),
                             ShapePrimitive_Facet);
    }
  }
}

//-----------------------------------------------------------------------------

void asiVisu_ShapeRobustTessellator::addVertex(const TopoDS_Vertex&         vertex,
                                               const vtkIdType              shapeId,
                                               const asiVisu_ShapePrimitive scType)
{
  if ( vertex.IsNull() )
    return;

  // Access host Cartesian point
  gp_Pnt pnt = BRep_Tool::Pnt(vertex);

  // Fill data structure
  vtkIdType pid = m_data->InsertCoordinate( pnt.X(), pnt.Y(), pnt.Z() );
  m_data->InsertVertex(shapeId, pid, scType);
}

//-----------------------------------------------------------------------------

void asiVisu_ShapeRobustTessellator::addEdge(const TopoDS_Edge&           edge,
                                             const vtkIdType              shapeId,
                                             const asiVisu_ShapePrimitive scType)
{
  if ( edge.IsNull() || BRep_Tool::Degenerated(edge) )
    return;

  Handle(Poly_PolygonOnTriangulation) polyOn;
  Handle(Poly_Triangulation) poly;
  TopLoc_Location loc;
  //
  BRep_Tool::PolygonOnTriangulation(edge, polyOn, poly, loc);

  if ( polyOn.IsNull() || poly.IsNull() )
    return;

  // Add node indices to the collection of boundary nodes
  const TColStd_Array1OfInteger& polygonOnTriNodes = polyOn->Nodes();
  vtkSmartPointer<vtkIdList> pids = vtkSmartPointer<vtkIdList>::New();
  //
  for ( int k = polygonOnTriNodes.Lower(); k <= polygonOnTriNodes.Upper(); ++k )
  {
    const int pidx = polygonOnTriNodes(k);
    gp_Pnt P = poly->Nodes()(pidx);
    //
    if ( !loc.IsIdentity() )
      P.Transform(loc);

    // Insert VTK point
    vtkIdType pid = m_data->InsertCoordinate( P.X(), P.Y(), P.Z() );
    //
    pids->InsertNextId(pid);
  }
  //
  m_data->InsertPolyline(shapeId, pids, scType);

//  // Use curve data source to provide the required tessellation
//  vtkSmartPointer<asiVisu_CurveSource>
//    curveSrc = vtkSmartPointer<asiVisu_CurveSource>::New();
//  //
//  if ( !curveSrc->SetInputEdge(edge) )
//  {
//#if defined COUT_DEBUG
//    std::cout << "Error: cannot discretize edge" << std::endl;
//#endif
//    return;
//  }
//
//  // Get tessellation
//  Handle(HRealArray) xCoords, yCoords, zCoords;
//  asiVisu_Orientation ori;
//  //
//  curveSrc->GetInputArrays(xCoords, yCoords, zCoords, ori);
//
//  if ( xCoords->Length() < 2 )
//  {
//#if defined COUT_DEBUG
//    std::cout << "Error: poor edge discretization" << std::endl;
//#endif
//    return;
//  }
//
//  // Prepare a discrete representation for edge
//  vtkSmartPointer<vtkIdList> pids = vtkSmartPointer<vtkIdList>::New();
//  //
//  for ( int j = xCoords->Lower(); j <= xCoords->Upper(); ++j )
//  {
//    IVtk_PointId pid = m_data->InsertCoordinate( xCoords->Value(j),
//                                                 yCoords->Value(j),
//                                                 zCoords->Value(j) );
//    pids->InsertNextId(pid);
//  }
//  m_data->InsertPolyline(shapeId, pids, scType);
}
