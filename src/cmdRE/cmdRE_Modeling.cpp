//-----------------------------------------------------------------------------
// Created on: 26 December 2018
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

// cmdRE includes
#include <cmdRE.h>

// asiAlgo includes
#include <asiAlgo_CheckDeviations.h>
#include <asiAlgo_Cloudify.h>
#include <asiAlgo_MeshInterPlane.h>
#include <asiAlgo_MeshMerge.h>
#include <asiAlgo_MeshProjectLine.h>
#include <asiAlgo_PlaneOnPoints.h>
#include <asiAlgo_PlateOnEdges.h>
#include <asiAlgo_ProjectPointOnMesh.h>
#include <asiAlgo_PurifyCloud.h>
#include <asiAlgo_Timer.h>
#include <asiAlgo_Utils.h>

// asiEngine includes
#include <asiEngine_IV.h>
#include <asiEngine_Part.h>
#include <asiEngine_RE.h>
#include <asiEngine_Triangulation.h>

// asiVisu includes
#include <asiVisu_ReEdgePrs.h>

// asiUI includes
#include <asiUI_IV.h>

#if defined USE_MOBIUS
  // Mobius includes
  #include <mobius/cascade.h>
  #include <mobius/geom_ApproxBSurf.h>
  #include <mobius/geom_BuildAveragePlane.h>
  #include <mobius/geom_InterpolateMultiCurve.h>
  #include <mobius/geom_FairBCurve.h>
  #include <mobius/geom_PlaneSurface.h>
  #include <mobius/geom_SkinSurface.h>

  using namespace mobius;
#endif

// Active Data includes
#include <ActData_Mesh_ElementsIterator.h>
#include <ActData_Mesh_Quadrangle.h>

// OCCT includes
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <GCPnts_QuasiUniformAbscissa.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <ShapeAnalysis_Surface.hxx>

// Qt includes
#pragma warning(push, 0)
#include <QDir>
#include <QMainWindow>
#include <QProcess>
#pragma warning(pop)

//-----------------------------------------------------------------------------

//! Unoriented link.
struct t_undirectedLink
{
  int N1; //!< First node.
  int N2; //!< Second node.

  //! Ctor default.
  t_undirectedLink() : N1(0), N2(0) {}

  //! Ctor with parameters.
  t_undirectedLink(const int _N1, const int _N2) : N1(_N1), N2(_N2) {}

  //! \return hash code for the link.
  static int HashCode(const t_undirectedLink& arc, const int upper)
  {
    int key = arc.N1 + arc.N2;
    key += (key << 10);
    key ^= (key >> 6);
    key += (key << 3);
    key ^= (key >> 11);
    return (key & 0x7fffffff) % upper;
  }

  //! \return true if two links are equal.
  static int IsEqual(const t_undirectedLink& arc1,
                     const t_undirectedLink& arc2)
  {
    return ( (arc1.N1 == arc2.N1) && (arc1.N2 == arc2.N2) ) ||
           ( (arc1.N2 == arc2.N1) && (arc1.N1 == arc2.N2) );
  }
};

//-----------------------------------------------------------------------------

int RE_SmoothenRegularEdges(const Handle(asiTcl_Interp)& interp,
                            int                          argc,
                            const char**                 argv)
{
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  asiEngine_RE reApi( cmdRE::model,
                      interp->GetProgress(),
                      interp->GetPlotter() );

  Handle(asiData_ReEdgesNode) edges = reApi.Get_Edges();

  /* Iterate over all edges. */

  cmdRE::model->OpenCommand();
  {
    for ( Handle(ActAPI_IChildIterator) eit = edges->GetChildIterator(); eit->More(); eit->Next() )
    {
      Handle(asiData_ReEdgeNode)
        edge = Handle(asiData_ReEdgeNode)::DownCast( eit->Value() );

      if ( reApi.IsRegular(edge) )
      {
        edge->SetSmoothTransition(true);
        //
        reApi.ReconnectSmoothenCornersFunc(edge);
        reApi.ReconnectSmoothenPatchesFunc(edge);
      }
      else
        interp->GetProgress().SendLogMessage( LogWarn(Normal) << "The edge '%1' is irregular, skipped..."
                                                              << edge->GetName() );
    }

    // Execute dependencies.
    cmdRE::model->FuncExecuteAll();
  }
  cmdRE::model->CommitCommand();

  // Actualize Patch Nodes.
  cmdRE::cf->ViewerPart->PrsMgr()->Actualize(reApi.Get_Patches(), true, false, false, false);
  cmdRE::cf->ViewerPart->PrsMgr()->Actualize(reApi.Get_Edges(),   true, false, false, false);
  cmdRE::cf->ViewerPart->Repaint();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int RE_BuildPatches(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  cmdRE_NotUsed(argc);
  cmdRE_NotUsed(argv);

  // Get surface fairing coefficient.
  double fairCoeff = 0.;
  TCollection_AsciiString fairCoeffStr;
  //
  if ( interp->GetKeyValue(argc, argv, "fair", fairCoeffStr) )
    fairCoeff = fairCoeffStr.RealValue();

  // Get a flag indicating if approximation is required.
  const bool isApprox = interp->HasKeyword(argc, argv, "approx");

  asiEngine_RE reApi( cmdRE::model,
                      interp->GetProgress(),
                      interp->GetPlotter() );

  // Find Patches Node.
  Handle(asiData_RePatchesNode) patchesNode = reApi.Get_Patches();
  //
  if ( patchesNode.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No patches are available.");
    return TCL_ERROR;
  }

  /* =============================
   *  Collect patches of interest
   * ============================= */

  Handle(ActAPI_HNodeList) patchNodes = new ActAPI_HNodeList;

  /* Collect patches by names */

  for ( int k = 1; k < argc; ++k )
  {
    // Skip keywords.
    TCollection_AsciiString arg(argv[k]);
    if ( arg.IsRealValue() || arg == "-fair" || arg == "-approx" )
      continue;

    Handle(asiData_RePatchNode)
      patchNode = Handle(asiData_RePatchNode)::DownCast( cmdRE::model->FindNodeByName(arg) );
    //
    if ( patchNode.IsNull() || !patchNode->IsWellFormed() )
    {
      interp->GetProgress().SendLogMessage(LogWarn(Normal) << "Object with name '%1' is not a patch."
                                                            << arg);
      continue;
    }
    //
    patchNodes->Append(patchNode);
  }

  if ( !patchNodes->Length() )
  {
    /* Collect all patches */

    for ( Handle(ActAPI_IChildIterator) eit = patchesNode->GetChildIterator(); eit->More(); eit->Next() )
    {
      Handle(asiData_RePatchNode)
        patchNode = Handle(asiData_RePatchNode)::DownCast( eit->Value() );
      //
      patchNodes->Append(patchNode);
    }
  }

  /* ================
   *  Build surfaces
   * ================ */

  cmdRE::model->OpenCommand();
  {
    // Reconnect basic Tree Functions.
    for ( ActAPI_HNodeList::Iterator nit(*patchNodes); nit.More(); nit.Next() )
    {
      const Handle(asiData_RePatchNode)&
        patchNode = Handle(asiData_RePatchNode)::DownCast( nit.Value() );

      patchNode->SetApproxNodes(isApprox);

      reApi.ReconnectBuildPatchFunc(patchNode);
    }

    cmdRE::model->FuncExecuteAll();
  }
  cmdRE::model->CommitCommand();

  // Actualize Patch Nodes.
  for ( ActAPI_HNodeList::Iterator nit(*patchNodes); nit.More(); nit.Next() )
    cmdRE::cf->ViewerPart->PrsMgr()->Actualize(nit.Value(), false, false, false, false);
  //
  cmdRE::cf->ViewerPart->Repaint();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int RE_BuildContourLines(const Handle(asiTcl_Interp)& interp,
                         int                          argc,
                         const char**                 argv)
{
  Handle(asiEngine_Model)
    M = Handle(asiEngine_Model)::DownCast( interp->GetModel() );

  asiEngine_RE reApi( cmdRE::model,
                      interp->GetProgress(),
                      interp->GetPlotter() );

  // Find Edges Node.
  Handle(asiData_ReEdgesNode) edgesNode = reApi.Get_Edges();
  //
  if ( edgesNode.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No edges are available.");
    return TCL_ERROR;
  }

  /* ===========================
   *  Collect edges of interest
   * =========================== */

  Handle(ActAPI_HNodeList) edgeNodes = new ActAPI_HNodeList;

  bool                    hasToler = false;
  double                  toler    = 5.0;
  TCollection_AsciiString tolerStr;
  //
  if ( interp->GetKeyValue(argc, argv, "toler", tolerStr) )
  {
    hasToler = true;
    toler    = tolerStr.RealValue();
  }

  /* Collect edges by names */
  for ( int k = 1; k < argc; ++k )
  {
    Handle(asiData_ReEdgeNode)
      edgeNode = Handle(asiData_ReEdgeNode)::DownCast( cmdRE::model->FindNodeByName(argv[k]) );
    //
    if ( edgeNode.IsNull() || !edgeNode->IsWellFormed() )
      continue;

    edgeNodes->Append(edgeNode);
  }

  if ( edgeNodes->IsEmpty() )
  {
    /* Collect all edges */
    for ( Handle(ActAPI_IChildIterator) eit = edgesNode->GetChildIterator(); eit->More(); eit->Next() )
      edgeNodes->Append( eit->Value() );
  }

  /* =================================
   *  Perform data model modification
   * ================================= */

  // Initialize progress indicator.
  interp->GetProgress().Init( edgeNodes->Length() );
  interp->GetProgress().SetMessageKey("Approximate curves");

  // Perform transactional changes.
  M->OpenCommand();
  {
    // Connect Tree Functions.
    for ( Handle(ActAPI_IChildIterator) eit = edgesNode->GetChildIterator(); eit->More(); eit->Next() )
    {
      Handle(asiData_ReEdgeNode)
        edgeNode = Handle(asiData_ReEdgeNode)::DownCast( eit->Value() );
      //
      edgeNode->SetApproxToler(toler);

      reApi.ReconnectBuildEdgeFunc(edgeNode);
    }

    // Approximate every edge individually.
    for ( ActAPI_HNodeList::Iterator eit(*edgeNodes); eit.More(); eit.Next() )
    {
      // Progress indication.
      interp->GetProgress().StepProgress(1);
      //
      if ( interp->GetProgress().IsCancelling() )
      {
        interp->GetProgress().SetProgressStatus(ActAPI_ProgressStatus::Progress_Canceled);
        //
        M->AbortCommand();
        return TCL_OK;
      }

      const Handle(asiData_ReEdgeNode)&
        edgeNode = Handle(asiData_ReEdgeNode)::DownCast( eit.Value() );

      // Approximate with parametric curve.
      std::vector<Handle(Geom_BSplineCurve)> curves;
      std::vector<gp_XYZ> pts;
      edgeNode->GetPolyline(pts);
      //
      Handle(Geom_BSplineCurve) curve;
      if ( !asiAlgo_Utils::ApproximatePoints(pts, 3, 3, toler, curve) )
      {
        interp->GetProgress().SendLogMessage( LogErr(Normal) << "Cannot approximate edge '%1'."
                                                              << edgeNode->GetName() );
        //
        continue;
      }

      // Update Data Model.
      edgeNode->SetCurve(curve);

      // Add curve to the collection for filling.
      curves.push_back(curve);
    }
  } M->CommitCommand();

  // Progress indication.
  interp->GetProgress().Init();
  interp->GetProgress().SetMessageKey("Actualize 3D scene");

  // Actualize.
  if ( cmdRE::cf->ViewerPart )
  {
    cmdRE::cf->ViewerPart->PrsMgr()->ActualizeCol(edgeNodes, false, false, false, false);

    // Set visibility of actors.
    for ( ActAPI_HNodeList::Iterator eit(*edgeNodes); eit.More(); eit.Next() )
    {
      const Handle(ActAPI_INode)& edgeNode = eit.Value();

      // Adjust visibility of actors.
      Handle(asiVisu_ReEdgePrs)
        edgePrs = Handle(asiVisu_ReEdgePrs)::DownCast( cmdRE::cf->ViewerPart->PrsMgr()->GetPresentation(edgeNode) );
      //
      edgePrs->SetCurvesOnlyMode(true);
    }

    // Repaint.
    cmdRE::cf->ViewerPart->Repaint();
  }

  interp->GetProgress().SetProgressStatus(ActAPI_ProgressStatus::Progress_Succeeded);
  return TCL_OK;
}

//-----------------------------------------------------------------------------

int RE_FairContourLines(const Handle(asiTcl_Interp)& interp,
                        int                          argc,
                        const char**                 argv)
{
#if defined USE_MOBIUS
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get fairing coefficient.
  const double lambda = atof(argv[1]);

  Handle(asiEngine_Model)
    M = Handle(asiEngine_Model)::DownCast( interp->GetModel() );

  asiEngine_RE reApi( cmdRE::model,
                      interp->GetProgress(),
                      interp->GetPlotter() );

  // Find Edges Node.
  Handle(asiData_ReEdgesNode) edgesNode = reApi.Get_Edges();
  //
  if ( edgesNode.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No edges are available.");
    return TCL_ERROR;
  }

  // Modify parametric model.
  M->OpenCommand();
  {
    for ( Handle(ActAPI_IChildIterator) eit = edgesNode->GetChildIterator(); eit->More(); eit->Next() )
    {
      Handle(asiData_ReEdgeNode)
        edgeNode = Handle(asiData_ReEdgeNode)::DownCast( eit->Value() );

      edgeNode->SetFairCurve(true);
      edgeNode->SetFairingCoeff(lambda);
    }

    // Execute deps.
    M->FuncExecuteAll();
  }
  M->CommitCommand();

  // Actualize.
  cmdRE::cf->ViewerPart->PrsMgr()->Actualize(reApi.Get_Patches(), true, false, false, false);
  cmdRE::cf->ViewerPart->PrsMgr()->Actualize(reApi.Get_Edges(),   true, false, false, false);
  cmdRE::cf->ViewerPart->Repaint();

  return TCL_OK;
#else
  cmdRE_NotUsed(argc);
  cmdRE_NotUsed(argv);

  interp->GetProgress().SendLogMessage(LogErr(Normal) << "This feature is not available.");
  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int RE_CutWithPlane(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  if ( argc != 3 && argc != 4 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  const bool doSort = !interp->HasKeyword(argc, argv, "nosort");

  // Get Triangulaion.
  Handle(asiData_TriangulationNode) tris_n = cmdRE::model->GetTriangulationNode();
  //
  if ( tris_n.IsNull() || !tris_n->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mesh is not ready for cutting.");
    return TCL_ERROR;
  }
  //
  Handle(Poly_Triangulation) triangulation = tris_n->GetTriangulation();

  // Get cutting t_plane.
  Handle(ActAPI_INode) node = cmdRE::model->FindNodeByName(argv[2]);
  //
  if ( node.IsNull() || !node->IsKind( STANDARD_TYPE(asiData_IVSurfaceNode) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Node '%1' is not a surface."
                                                        << argv[1]);
    return TCL_ERROR;
  }
  //
  Handle(asiData_IVSurfaceNode)
    surfaceNode = Handle(asiData_IVSurfaceNode)::DownCast(node);
  //
  Handle(Geom_Plane)
    occtPlane = Handle(Geom_Plane)::DownCast( surfaceNode->GetSurface() );
  //
  if ( occtPlane.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The surface in question is not a t_plane.");
    return TCL_ERROR;
  }

  // Intersect with t_plane.
  asiAlgo_MeshInterPlane algo( triangulation,
                               interp->GetProgress(),
                               interp->GetPlotter() );
  //
  if ( !algo.Perform(occtPlane, true) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Failed to cut mesh with t_plane.");
    return TCL_ERROR;
  }

  // Get the result.
  interp->GetPlotter().REDRAW_POINTS(argv[1], algo.GetResult()->GetCoordsArray(),
                                     doSort ? Color_Green : Color_Red);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int RE_ApproxPoints(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  if ( argc < 4 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get point cloud Node.
  Handle(asiData_IVPointSetNode)
    ptsNode = Handle(asiData_IVPointSetNode)::DownCast( cmdRE::model->FindNodeByName(argv[2]) );
  //
  if ( ptsNode.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Data object '%1' is not a point cloud."
                                                        << argv[2]);
    return TCL_ERROR;
  }

  // Get points with reper (selected) points. The reper points (if any)
  // will be used as breakpoints to approximate separately.
  Handle(asiAlgo_BaseCloud<double>)   ptsCloud  = ptsNode->GetPoints();
  Handle(TColStd_HPackedMapOfInteger) ptsFilter = ptsNode->GetFilter();

  // Get precision.
  const double prec = atof(argv[3]);

  // Check is the curve is expected to be closed.
  const bool isClosed = interp->HasKeyword(argc, argv, "closed");
  //
  if ( isClosed && !ptsFilter.IsNull() && ptsFilter->Map().Extent() )
    interp->GetProgress().SendLogMessage(LogWarn(Normal) << "There is a filter on points. A closed curve cannot be constructed.");

  // Collect indices of breakpoints into an ordered collection. We do not
  // rely here on the order of packed map since this map is not ordered
  // in essence.
  std::vector<int> breakIndices;
  //
  if ( !ptsFilter.IsNull() )
  {
    const TColStd_PackedMapOfInteger& filter = ptsFilter->Map();
    //
    for ( TColStd_MapIteratorOfPackedMapOfInteger fit(filter); fit.More(); fit.Next() )
      breakIndices.push_back( fit.Key() );
    //
    std::sort( breakIndices.begin(), breakIndices.end() );
  }

  // Prepare a collection of point regions.
  std::vector< std::vector<gp_XYZ> > ptsRegions;

  // If no filter is set, there is only one region to approximate.
  if ( !breakIndices.size() )
  {
    std::vector<gp_XYZ> pts;
    for ( int k = 0; k < ptsCloud->GetNumberOfElements(); ++k )
      pts.push_back( ptsCloud->GetElement(k) );
    //
    ptsRegions.push_back(pts);
  }
  else
  {
    // Loop over the indices of breakpoints.
    for ( size_t b = 0; b < breakIndices.size(); ++b )
    {
      const int  b_first    = breakIndices[b];
      const int  b_next     = ( (b == breakIndices.size() - 1) ? breakIndices[0] : breakIndices[b + 1] );
      //const bool isReturing = (b_next == b_first);

      // Populate next region.
      std::vector<gp_XYZ> pts;
      int  k    = b_first;
      bool stop = false;
      //
      do
      {
        if ( pts.size() && (k == b_next) ) // Reached next breakpoint.
          stop = true;

        if ( k == ptsCloud->GetNumberOfElements() - 1 ) // Reached the end, cycling...
        {
          k = 0;
          //
          if ( pts.size() && (k == b_next) ) // Reached next breakpoint.
            stop = true;
        }

        pts.push_back( ptsCloud->GetElement(k++) );
      }
      while ( !stop );

      // Add region.
      ptsRegions.push_back(pts);
    }
  }

  // For unfiltered clouds, the curve can be closed.
  if ( (ptsRegions.size() == 1) && isClosed )
    ptsRegions[0].push_back( ptsRegions[0][0] );

  // Approximate.
  for ( size_t k = 0; k < ptsRegions.size(); ++k )
  {
    // Run approximation algorithm.
    Handle(Geom_BSplineCurve) resCurve;
    if ( !asiAlgo_Utils::ApproximatePoints(ptsRegions[k], 3, 3, prec, resCurve) )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Approximation failed.");
      return TCL_ERROR;
    }

    // Set result.
    if ( ptsRegions.size() == 1 )
      interp->GetPlotter().REDRAW_CURVE(TCollection_AsciiString(argv[1]), resCurve, Color_Red);
    else
      interp->GetPlotter().DRAW_CURVE(resCurve, Color_Red, TCollection_AsciiString(argv[1]));
  }

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int RE_SkinSurface(const Handle(asiTcl_Interp)& interp,
                   int                          argc,
                   const char**                 argv)
{
#if defined USE_MOBIUS
  if ( argc < 5 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Check if fairing is requested.
  bool   isFairing    = false;
  double fairingCoeff = 0.0;
  //
  for ( int k = 2; k < argc-1; ++k )
  {
    if ( interp->IsKeyword(argv[k], "fair-isos") )
    {
      isFairing    = true;
      fairingCoeff = Atof(argv[k + 1]);
      break;
    }
  }

  // Get degree in V curvilinear direction.
  const int vDegree = atoi(argv[2]);

  // Convert OCCT B-curves to Mobius B-curves.
  std::vector< t_ptr<t_bcurve> > bCurves;
  //
  for ( int k = (isFairing ? 5 : 3); k < argc; ++k )
  {
    // Get Node.
    Handle(ActAPI_INode) node = cmdRE::model->FindNodeByName(argv[k]);
    //
    if ( node.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find a data object with name '%1'."
                                                          << argv[k]);
      return TCL_ERROR;
    }

    // Get parametric curve.
    Handle(Geom_Curve) curveBase;
    //
    if ( node->IsInstance( STANDARD_TYPE(asiData_IVCurveNode) ) )
    {
      double f, l;
      curveBase = Handle(asiData_IVCurveNode)::DownCast(node)->GetCurve(f, l);
    }
    else
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Unexpected type of Node with ID %1."
                                                          << argv[k]);
      return TCL_ERROR;
    }

    // Convert curve using a dedicated Mobius utility.
    Handle(Geom_BSplineCurve)
      occtBCurve = Handle(Geom_BSplineCurve)::DownCast(curveBase);
    //
    t_ptr<t_bcurve> mobiusBCurve = cascade::GetMobiusBCurve(occtBCurve);
    //
    bCurves.push_back(mobiusBCurve);

#if defined COUT_DEBUG
    // Print knot vector for information.
    const std::vector<double>& U = mobiusBCurve->Knots();
    //
    TCollection_AsciiString Ustr;
    //
    Ustr += "[";
    for ( size_t s = 0; s < U.size(); ++s )
    {
      Ustr += U[s];
      if ( s < U.size() - 1 )
        Ustr += " ";
    }
    Ustr += "]";
    //
    interp->GetProgress().SendLogMessage(LogInfo(Normal) << "... U[%1] = %2."
                                                         << argv[k] << Ustr);
#endif
  }

  // Skin surface.
  geom_SkinSurface skinner(bCurves, vDegree, true);
  //
  if ( !skinner.PrepareSections() )
  {
    interp->GetProgress().SendLogMessage( LogErr(Normal) << "Cannot prepare sections (error code %1)."
                                                         << skinner.GetErrorCode() );
    return TCL_ERROR;
  }
  //
  if ( !skinner.BuildIsosU() )
  {
    interp->GetProgress().SendLogMessage( LogErr(Normal) << "Cannot build U isos (error code %1)."
                                                         << skinner.GetErrorCode() );
    return TCL_ERROR;
  }

  // Fair skinning curves.
  for ( size_t k = 0; k < skinner.IsoU_Curves.size(); ++k )
  {
    // Convert Mobius curve to OpenCascade curve.
    {
      Handle(Geom_BSplineCurve)
        occtIsoU = cascade::GetOpenCascadeBCurve(skinner.IsoU_Curves[k]);

#if defined DRAW_DEBUG
      interp->GetPlotter().DRAW_CURVE(occtIsoU, Color_White, "Iso_U");
#endif
    }

    // Perform fairing if requested.
    if ( isFairing )
    {
      geom_FairBCurve fairing(skinner.IsoU_Curves[k], fairingCoeff, nullptr, nullptr);
      //asiAlgo_FairBCurve fairing( occtIsoU,
      //                            fairingCoeff, // Fairing coefficient.
      //                            interp->GetProgress(),
      //                            interp->GetPlotter() );
      //
      if ( !fairing.Perform() )
      {
        interp->GetProgress().SendLogMessage( LogErr(Normal) << "Fairing failed for iso-U curve %1."
                                                             << int(k) );
        return TCL_ERROR;
      }

      // Override U iso in the skinner.
      skinner.IsoU_Curves[k] = fairing.GetResult();

      // Convert Mobius curve to OpenCascade curve.
      {
        Handle(Geom_BSplineCurve)
          occtIsoUFaired = cascade::GetOpenCascadeBCurve(skinner.IsoU_Curves[k]);

#if defined DRAW_DEBUG
        interp->GetPlotter().DRAW_CURVE(occtIsoUFaired, Color_White, "Iso_U_faired");
#endif
      }
    }
  }

  // Build final surface.
  if ( !skinner.BuildSurface() )
  {
    interp->GetProgress().SendLogMessage( LogErr(Normal) << "Cannot build interpolant surface (error code %1)."
                                                         << skinner.GetErrorCode() );
    return TCL_ERROR;
  }

  // Get skinned surface.
  const t_ptr<t_bsurf>& mobiusRes = skinner.GetResult();

  /* ==========================================
   *  Convert interpolant to OpenCascade shape
   * ========================================== */

  Handle(Geom_BSplineSurface)
    occtRes = cascade::GetOpenCascadeBSurface(mobiusRes);

  interp->GetPlotter().REDRAW_SURFACE(argv[1], occtRes, Color_Default);

  return TCL_OK;
#else
  cmdRE_NotUsed(argc);
  cmdRE_NotUsed(argv);

  interp->GetProgress().SendLogMessage(LogErr(Normal) << "This feature is not available.");
  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int RE_InterpMulticurve(const Handle(asiTcl_Interp)& interp,
                        int                          argc,
                        const char**                 argv)
{
#if defined USE_MOBIUS
  if ( argc < 5 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  /* =====================
   *  Stage 1: get inputs
   * ===================== */

  // Get number of discretization points on each curve.
  const int numPts = atoi(argv[1]);

  // Get degree of interpolation.
  const int degree = atoi(argv[2]);

  // Get B-curves to interpolate.
  std::vector<Handle(Geom_BSplineCurve)> bCurves;
  std::vector<TCollection_AsciiString>   bCurveNames;
  //
  for ( int k = 3; k < argc; ++k )
  {
    // Get Node.
    Handle(ActAPI_INode) node = cmdRE::model->FindNodeByName(argv[k]);
    //
    if ( node.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find a data object with name '%1'."
                                                          << argv[k]);
      return TCL_ERROR;
    }

    // Get parametric curve.
    Handle(Geom_Curve) curveBase;
    //
    if ( node->IsInstance( STANDARD_TYPE(asiData_IVCurveNode) ) )
    {
      double f, l;
      curveBase = Handle(asiData_IVCurveNode)::DownCast(node)->GetCurve(f, l);
    }
    else
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Unexpected type of Node with ID %1."
                                                          << argv[k]);
      return TCL_ERROR;
    }

    // Add curve to the collection of curves for synchronous interpolation.
    Handle(Geom_BSplineCurve)
      occtBCurve = Handle(Geom_BSplineCurve)::DownCast(curveBase);
    //
    if ( !occtBCurve.IsNull() )
    {
      bCurves.push_back(occtBCurve);
      bCurveNames.push_back(argv[k]);
    }
  }

  /* ==============================================================
   *  Stage 2: discretize curves to have the same number of points
   * ============================================================== */

  TIMER_NEW
  TIMER_GO

  // Prepare interpolation tool.
  geom_InterpolateMultiCurve interpTool(degree,
                                        ParamsSelection_Centripetal,
                                        KnotsSelection_Average);

  for ( size_t k = 0; k < bCurves.size(); ++k )
  {
    // Discretize with a uniform curvilinear step.
    GeomAdaptor_Curve gac(bCurves[k]);
    GCPnts_QuasiUniformAbscissa Defl(gac, numPts);
    //
    if ( !Defl.IsDone() )
      return false;

    // Fill row of points.
    std::vector<t_xyz> ptsRow;
    //
    for ( int i = 1; i <= numPts; ++i )
    {
      const double param = Defl.Parameter(i);
      t_xyz P = cascade::GetMobiusPnt( bCurves[k]->Value(param) );
      //
      ptsRow.push_back(P);
    }

    // Add points to the interpolation tool.
    interpTool.AddRow(ptsRow);
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Discretize curves")

  /* =================================
   *  Stage 3: interpolate multicurve
   * ================================= */

  TIMER_RESET
  TIMER_GO

  if ( !interpTool.Perform() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Interpolation failed.");
    return TCL_ERROR;
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Interpolate curves")

  for ( int k = 0; k < interpTool.GetNumRows(); ++k )
  {
    Handle(Geom_BSplineCurve)
      resCurve = cascade::GetOpenCascadeBCurve( interpTool.GetResult(k) );

    interp->GetPlotter().REDRAW_CURVE(bCurveNames[k], resCurve, Color_Default);
  }

  return TCL_OK;
#else
  cmdRE_NotUsed(argc);
  cmdRE_NotUsed(argv);

  interp->GetProgress().SendLogMessage(LogErr(Normal) << "This feature is not available.");
  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int RE_JoinCurves(const Handle(asiTcl_Interp)& interp,
                  int                          argc,
                  const char**                 argv)
{
  if ( argc < 4 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get B-curves to join.
  std::vector<Handle(Geom_BSplineCurve)> bCurves;
  //
  for ( int k = 2; k < argc; ++k )
  {
    // Get Node.
    Handle(ActAPI_INode) node = cmdRE::model->FindNodeByName(argv[k]);
    //
    if ( node.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find a data object with name '%1'."
                                                          << argv[k]);
      return TCL_ERROR;
    }

    // Get parametric curve.
    Handle(Geom_Curve) curveBase;
    //
    if ( node->IsInstance( STANDARD_TYPE(asiData_IVCurveNode) ) )
    {
      double f, l;
      curveBase = Handle(asiData_IVCurveNode)::DownCast(node)->GetCurve(f, l);
    }
    else
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Unexpected type of Node with ID %1."
                                                          << argv[k]);
      return TCL_ERROR;
    }

    // Add curve to the collection of curves for synchronous interpolation.
    Handle(Geom_BSplineCurve)
      occtBCurve = Handle(Geom_BSplineCurve)::DownCast(curveBase);
    //
    if ( !occtBCurve.IsNull() )
      bCurves.push_back(occtBCurve);
  }

  // Join curves.
  Handle(Geom_BSplineCurve) jointCurve;
  //
  if ( !asiAlgo_Utils::JoinCurves( bCurves,
                                   2, // C2
                                   jointCurve,
                                   interp->GetProgress() ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot join curves.");
    return TCL_ERROR;
  }

  // Set result.
  interp->GetPlotter().REDRAW_CURVE(argv[1], jointCurve, Color_Default);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int RE_CheckSurfDeviation(const Handle(asiTcl_Interp)& interp,
                          int                          argc,
                          const char**                 argv)
{
  if ( argc < 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get surface to analyze.
  Handle(ActAPI_INode) node = cmdRE::model->FindNodeByName(argv[2]);
  //
  if ( node.IsNull() || !node->IsKind( STANDARD_TYPE(asiData_IVSurfaceNode) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Node '%1' is not a surface."
                                                        << argv[2]);
    return TCL_ERROR;
  }
  //
  Handle(asiData_IVSurfaceNode)
    surfaceNode = Handle(asiData_IVSurfaceNode)::DownCast(node);

  // Prepare Deviation Node.
  Handle(asiData_SurfDeviationNode) sdNode;
  //
  cmdRE::model->OpenCommand();
  {
    // If there is no BVH for mesh, build one.
    if ( cmdRE::model->GetTriangulationNode()->GetBVH().IsNull() )
      asiEngine_Triangulation( cmdRE::model, interp->GetProgress(), nullptr ).BuildBVH();

    // Create Surface Deviation Node.
    sdNode = asiEngine_IV(cmdRE::model, interp->GetProgress(), nullptr).Create_SurfaceDeviation(argv[1], surfaceNode);
  }
  cmdRE::model->CommitCommand();

  // Update UI.
  cmdRE::cf->ObjectBrowser->Populate();
  cmdRE::cf->ViewerPart->PrsMgr()->Actualize(sdNode);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int RE_CheckDeviation(const Handle(asiTcl_Interp)& interp,
                      int                          argc,
                      const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get points to analyze.
  Handle(ActAPI_INode) node = cmdRE::model->FindNodeByName(argv[1]);
  //
  if ( node.IsNull() || !node->IsKind( STANDARD_TYPE(asiData_IVPointSetNode) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Node '%1' is not a point set."
                                                        << argv[1]);
    return TCL_ERROR;
  }
  //
  Handle(asiData_IVPointSetNode)
    pointsNode = Handle(asiData_IVPointSetNode)::DownCast(node);

  // Get working part.
  Handle(asiData_PartNode) partNode = cmdRE::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is null or ill-defined.");
    return TCL_ERROR;
  }

  Handle(asiData_DeviationNode) devNode;

  // Check deviations.
  asiEngine_Part partApi( cmdRE::model,
                          cmdRE::cf->ViewerPart->PrsMgr(),
                          interp->GetProgress(),
                          interp->GetPlotter() );
  //
  cmdRE::model->OpenCommand();
  {
    if ( !partApi.CheckDeviation(pointsNode, devNode) )
    {
      cmdRE::model->AbortCommand();

      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Failed to check deviations.");
      return TCL_ERROR;
    }
  }
  cmdRE::model->CommitCommand();

  // Actualize presentation.
  if ( cmdRE::cf->ViewerPart )
    cmdRE::cf->ViewerPart->PrsMgr()->Actualize(devNode);

  // Update Object Browser.
  if ( cmdRE::cf->ObjectBrowser )
    cmdRE::cf->ObjectBrowser->Populate();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int RE_CheckTriDeviation(const Handle(asiTcl_Interp)& interp,
                         int                          argc,
                         const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get points to analyze.
  Handle(ActAPI_INode) node = cmdRE::model->FindNodeByName(argv[1]);
  //
  if ( node.IsNull() || !node->IsKind( STANDARD_TYPE(asiData_IVPointSetNode) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Node '%1' is not a point set."
                                                        << argv[1]);
    return TCL_ERROR;
  }
  //
  Handle(asiData_IVPointSetNode)
    pointsNode = Handle(asiData_IVPointSetNode)::DownCast(node);

  // Get working triangulation.
  Handle(asiData_TriangulationNode) trisNode = cmdRE::model->GetTriangulationNode();
  //
  if ( trisNode.IsNull() || !trisNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Triangulation Node is null or ill-defined.");
    return TCL_ERROR;
  }

  Handle(asiData_DeviationNode) devNode;

  // Check deviations.
  asiEngine_Triangulation triApi( cmdRE::model,
                                  cmdRE::cf->ViewerPart->PrsMgr(),
                                  interp->GetProgress(),
                                  interp->GetPlotter() );
  //
  cmdRE::model->OpenCommand();
  {
    if ( !triApi.CheckDeviation(pointsNode, devNode) )
    {
      cmdRE::model->AbortCommand();

      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Failed to check deviations.");
      return TCL_ERROR;
    }
  }
  cmdRE::model->CommitCommand();

  // Actualize presentation.
  if ( cmdRE::cf->ViewerPart )
    cmdRE::cf->ViewerPart->PrsMgr()->Actualize(devNode);

  // Update Object Browser.
  if ( cmdRE::cf->ObjectBrowser )
    cmdRE::cf->ObjectBrowser->Populate();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int RE_MakeAveragePlane(const Handle(asiTcl_Interp)& interp,
                        int                          argc,
                        const char**                 argv)
{
  if ( argc != 3 && argc != 4 && argc != 7 && argc != 8 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  const bool isMobius = interp->HasKeyword(argc, argv, "mobius");

  // Get point cloud Node.
  Handle(asiData_IVPointSetNode)
    ptsNode = Handle(asiData_IVPointSetNode)::DownCast( cmdRE::model->FindNodeByName(argv[2]) );
  //
  if ( ptsNode.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Data object '%1' is not a point cloud."
                                                        << argv[2]);
    return TCL_ERROR;
  }

  // Get points.
  Handle(asiAlgo_BaseCloud<double>) ptsCloud = ptsNode->GetPoints();

  // Get limits (if any).
  double uMin = 0., uMax = 0., vMin = 0., vMax = 0.;
  //
  if ( argc == 7 || argc == 8 )
  {
    uMin = atof(argv[3]);
    uMax = atof(argv[4]);
    vMin = atof(argv[5]);
    vMax = atof(argv[6]);
  }

  Handle(Geom_Surface) resPlane;
  //
  if ( isMobius )
  {
#if defined USE_MOBIUS
    t_ptr<t_plane> mobPlane;

    // Prepare point cloud.
    t_ptr<t_pcloud> mobPts = new t_pcloud( ptsCloud->GetCoords() );

    // Build average plane.
    geom_BuildAveragePlane planeAlgo;
    //
    if ( !planeAlgo.Build(mobPts, mobPlane) )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot build average plane.");
      return TCL_ERROR;
    }

    // Project point cloud to plane to determine the parametric bounds.
    mobPlane->TrimByPoints(mobPts);

    // Convert plane to B-surf.
    t_ptr<t_bsurf> mobPlaneBSurf = mobPlane->ToBSurface(3, 3);

    // Convert to OpenCascade structure.
    resPlane = cascade::GetOpenCascadeBSurface(mobPlaneBSurf);
#else
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");
#endif
  }
  else
  {
    gp_Pln pln;

    asiAlgo_PlaneOnPoints planeOnPoints( interp->GetProgress(), interp->GetPlotter() );
    //
    if ( !planeOnPoints.Build(ptsCloud, pln) )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Failed to build average plane.");
      return TCL_ERROR;
    }

    resPlane = new Geom_Plane(pln);
  }

  Handle(asiUI_IV) IV = Handle(asiUI_IV)::DownCast( interp->GetPlotter().Access() );

  // Set the result.
  IV->REDRAW_SURFACE(argv[1], resPlane, Color_Default);

  // Set limits (if passed).
  Handle(asiData_IVSurfaceNode)
    ivSurf = Handle(asiData_IVSurfaceNode)::DownCast( IV->GetLastNode() );
  //
  if ( !ivSurf.IsNull() && uMin && uMax && vMin && vMax )
  {
    cmdRE::model->OpenCommand();
    {
      ivSurf->SetLimits(uMin, uMax, vMin, vMax);
    }
    cmdRE::model->CommitCommand();
  }

  return TCL_OK;
}


//-----------------------------------------------------------------------------

int RE_SamplePart(const Handle(asiTcl_Interp)& interp,
                  int                          argc,
                  const char**                 argv)
{
  if ( argc != 4 && argc != 5 && argc != 6 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Part Node to access the shape.
  Handle(asiData_PartNode)
    partNode = Handle(asiEngine_Model)::DownCast( interp->GetModel() )->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is null or ill-defined.");
    return TCL_ERROR;
  }
  //
  TopoDS_Shape shape = partNode->GetShape();

  // Whether to sample facets (alternatively, you may want to sample faces).
  const bool onFacets = interp->HasKeyword(argc, argv, "facets");

  // Whether to take vertices as points.
  const bool useVertices = interp->HasKeyword(argc, argv, "vertices");

  // Cloudify shape.
  Handle(asiAlgo_BaseCloud<double>) sampledPts;
  //
  if ( !useVertices )
  {
    asiAlgo_Cloudify cloudify( std::min( atof(argv[2]), atof(argv[3]) ) );
    //
    cloudify.SetParametricSteps( atof(argv[2]), atof(argv[3]) );
    //
    if ( (  onFacets && !cloudify.Sample_Facets (shape, sampledPts) ) ||
         ( !onFacets && !cloudify.Sample_Faces  (shape, sampledPts) ) )
    {
      interp->GetProgress().SendLogMessage( LogErr(Normal) << "Cannot sample shape." );
      return TCL_ERROR;
    }
  }
  else
  {
    sampledPts = new asiAlgo_BaseCloud<double>;

    if ( onFacets )
    {
      // Merge facets.
      asiAlgo_MeshMerge meshMerge(shape);
      //
      Handle(Poly_Triangulation)
        tris = meshMerge.GetResultPoly()->GetTriangulation();

      for ( int n = 1; n <= tris->NbNodes(); ++n )
      {
        const gp_Pnt& P = tris->Node(n);
        //
        sampledPts->AddElement( P.XYZ() );
      }
    }
    else
    {
      // Map vertices.
      TopTools_IndexedMapOfShape allVertices;
      TopExp::MapShapes(shape, TopAbs_VERTEX, allVertices);
      //
      for ( int n = 1; n <= allVertices.Extent(); ++n )
      {
        gp_Pnt P = BRep_Tool::Pnt( TopoDS::Vertex( allVertices(n) ) );
        //
        sampledPts->AddElement( P.XYZ() );
      }
    }
  }

  // Set the result.
  interp->GetPlotter().REDRAW_POINTS(argv[1], sampledPts->GetCoordsArray(), Color_Yellow);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int RE_InvertPoints(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
#if defined USE_MOBIUS
  if ( argc != 3 && argc != 4 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Find Surface Node by name.
  Handle(asiData_IVSurfaceNode)
    surfNode = Handle(asiData_IVSurfaceNode)::DownCast( interp->GetModel()->FindNodeByName(argv[1]) );
  //
  if ( surfNode.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Node '%1' is not a surface."
                                                        << argv[1]);
    return TCL_ERROR;
  }

  // Find Points Node by name.
  Handle(asiData_IVPointSetNode)
    pointsNode = Handle(asiData_IVPointSetNode)::DownCast( interp->GetModel()->FindNodeByName(argv[2]) );
  //
  if ( pointsNode.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Node '%1' is not a point cloud."
                                                        << argv[2]);
    return TCL_OK;
  }

  const bool isOCC = interp->HasKeyword(argc, argv, "opencascade");

  // Get B-surface.
  Handle(Geom_BSplineSurface)
    occtBSurface = Handle(Geom_BSplineSurface)::DownCast( surfNode->GetSurface() );
  //
  if ( occtBSurface.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The surface in question is not a B-spline surface.");
    return TCL_ERROR;
  }

  // Convert to Mobius B-surface.
  t_ptr<t_bsurf>
    mobSurf = cascade::GetMobiusBSurface(occtBSurface);

  // Get points.
  Handle(asiAlgo_BaseCloud<double>) pts = pointsNode->GetPoints();

  // Resulting collections.
  Handle(asiAlgo_BaseCloud<double>) ptsInverted = new asiAlgo_BaseCloud<double>;
  Handle(asiAlgo_BaseCloud<double>) ptsFailed   = new asiAlgo_BaseCloud<double>;

  ShapeAnalysis_Surface sas(occtBSurface);

  TIMER_NEW
  TIMER_GO

  // Prepare memory arena.
  t_ptr<t_alloc2d> localAlloc = new t_alloc2d;
  //
  const int memBlock_BSplineSurfEvalD2U      = 0;
  const int memBlock_BSplineSurfEvalD2V      = 1;
  const int memBlock_BSplineSurfEvalInternal = 2;
  //
  localAlloc->Allocate(3, mobSurf->GetDegree_U() + 1, true);
  localAlloc->Allocate(3, mobSurf->GetDegree_V() + 1, true);
  localAlloc->Allocate(2, 2 + 1,                      true);

  // Loop over the points and invert each one to the surface.
  for ( int k = 0; k < pts->GetNumberOfElements(); ++k )
  {
    t_xyz xyz = cascade::GetMobiusPnt( pts->GetElement(k) );

    // Invert point.
    t_uv projUV;

    if ( !isOCC )
    {
      if ( !mobSurf->InvertPoint(xyz, projUV, 1e-6,
                                 localAlloc,
                                 memBlock_BSplineSurfEvalD2U,
                                 memBlock_BSplineSurfEvalD2V,
                                 memBlock_BSplineSurfEvalInternal) )
      {
        std::cout << "Failed point num. " << k << std::endl;
        interp->GetProgress().SendLogMessage(LogErr(Normal) << "Point inversion failed.");
        ptsFailed->AddElement( cascade::GetOpenCascadePnt(xyz).XYZ() );
        continue;
      }
    }
    else // Project using OpenCascade kernel for experimenting.
    {
      gp_Pnt2d occUV = sas.ValueOfUV(cascade::GetOpenCascadePnt(xyz), 1e-6);
      //
      projUV.SetU( occUV.X() );
      projUV.SetV( occUV.Y() );
    }

    // Evaluate surface for the obtained (u,v) coordinates.
    t_xyz S;
    mobSurf->Eval(projUV.U(), projUV.V(), S);
    //
    ptsInverted->AddElement( cascade::GetOpenCascadePnt(S).XYZ() );
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "re-invert-points")

  // Render point clouds.
  interp->GetPlotter().REDRAW_POINTS("ptsFailed",   ptsFailed->GetCoordsArray(),   Color_Red);
  interp->GetPlotter().REDRAW_POINTS("ptsInverted", ptsInverted->GetCoordsArray(), Color_Green);

  return TCL_OK;
#else
  cmdRE_NotUsed(argc);
  cmdRE_NotUsed(argv);

  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");
  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int RE_PurifyCloud(const Handle(asiTcl_Interp)& interp,
                   int                          argc,
                   const char**                 argv)
{
  if ( argc != 4 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Find Points Node by name.
  Handle(asiData_IVPointSetNode)
    pointsNode = Handle(asiData_IVPointSetNode)::DownCast( interp->GetModel()->FindNodeByName(argv[2]) );
  //
  if ( pointsNode.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Node '%1' is not a point cloud."
                                                        << argv[2]);
    return TCL_ERROR;
  }

  // Get point cloud.
  Handle(asiAlgo_BaseCloud<double>) pts = pointsNode->GetPoints(), res;

  // Purify.
  asiAlgo_PurifyCloud purify( interp->GetProgress(), interp->GetPlotter() );
  //
  purify.Perform3d(atof(argv[3]), pts, res);

  // Set the result.
  interp->GetPlotter().REDRAW_POINTS(argv[1], res->GetCoordsArray(), Color_Default);
  return TCL_OK;
}

//-----------------------------------------------------------------------------

int RE_ApproxSurf(const Handle(asiTcl_Interp)& interp,
                  int                          argc,
                  const char**                 argv)
{
#if defined USE_MOBIUS
  if (argc != 7 && argc != 8 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Find Points Node by name.
  Handle(asiData_IVPointSetNode)
    pointsNode = Handle(asiData_IVPointSetNode)::DownCast( interp->GetModel()->FindNodeByName(argv[2]) );
  //
  if ( pointsNode.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Node '%1' is not a point cloud."
                                                        << argv[2]);
    return TCL_ERROR;
  }

  // Get fairing coefficient.
  double lambda = 0.;
  TCollection_AsciiString lambdaStr;
  //
  if ( interp->GetKeyValue(argc, argv, "lambda", lambdaStr) )
    lambda = lambdaStr.RealValue();

  // Has init surf.
  const bool hasInitSurf = interp->HasKeyword(argc, argv, "init");

  // Get point cloud.
  Handle(asiAlgo_BaseCloud<double>) pts = pointsNode->GetPoints();

  // Convert to Mobius point cloud.
  t_ptr<t_pcloud> mobPts = new t_pcloud;
  //
  for ( int k = 0; k < pts->GetNumberOfElements(); ++k )
    mobPts->AddPoint( cascade::GetMobiusPnt( pts->GetElement(k) ) );

  t_ptr<t_bsurf> resSurf;

  if ( !hasInitSurf )
  {
    TIMER_NEW
    TIMER_GO

    // Approximate.
    geom_ApproxBSurf approx( mobPts, atoi(argv[3]), atoi(argv[4]) );
    //
    if ( !approx.Perform(lambda) )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Approximation failed.");
      return TCL_ERROR;
    }

    TIMER_FINISH
    TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Approximate (passed degrees)")

    // Get result.
    resSurf = approx.GetResult();
  }
  else /* Initial surface is specified */
  {
    // Find Surface Node by name.
    Handle(asiData_IVSurfaceNode)
      surfNode = Handle(asiData_IVSurfaceNode)::DownCast( interp->GetModel()->FindNodeByName(argv[4]) );
    //
    if ( surfNode.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Node '%1' is not a surface."
                                                          << argv[4]);
      return TCL_ERROR;
    }

    // Get surface.
    Handle(Geom_BSplineSurface)
      bsurfOcc = Handle(Geom_BSplineSurface)::DownCast( surfNode->GetSurface() );
    //
    if ( bsurfOcc.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "The passed surface is not a B-surface.");
      return TCL_ERROR;
    }

    // Check if the boundary is requested to be constrained.
    const bool isPinned = interp->HasKeyword(argc, argv, "pinned");

    // Convert to Mobius form.
    t_ptr<t_bsurf> initSurf = cascade::GetMobiusBSurface(bsurfOcc);

    TIMER_NEW
    TIMER_GO

    // Prepare approximation tool.
    geom_ApproxBSurf approx(mobPts, initSurf);

    // Constraint the boundary.
    if ( isPinned )
    {
      const int nPolesU = int( initSurf->GetPoles().size() );
      const int nPolesV = int( initSurf->GetPoles()[0].size() );
      //
      for ( int i = 0; i < nPolesU; ++i )
      {
        approx.AddPinnedPole( i, 0 );
        approx.AddPinnedPole( i, nPolesV - 1 );
      }
      //
      for ( int j = 0; j < nPolesV; ++j )
      {
        approx.AddPinnedPole( 0, j );
        approx.AddPinnedPole( nPolesU - 1, j );
      }
    }

    // Approximate.
    if ( !approx.Perform(lambda) )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Approximation failed.");
      return TCL_ERROR;
    }

    TIMER_FINISH
    TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Approximate (passed initial surface)")

    // Get result.
    resSurf = approx.GetResult();
  }

  // Set the result.
  interp->GetPlotter().REDRAW_SURFACE(argv[1], cascade::GetOpenCascadeBSurface(resSurf), Color_Default);
  return TCL_OK;
#else
  cmdRE_NotUsed(argc);
  cmdRE_NotUsed(argv);

  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");
  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int RE_GetTriangulationNodes(const Handle(asiTcl_Interp)& interp,
                             int                          argc,
                             const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get triangulation.
  Handle(Poly_Triangulation)
    tris = cmdRE::model->GetTriangulationNode()->GetTriangulation();
  //
  if ( tris.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Triangulation is null.");
    return TCL_ERROR;
  }

  // Prepare a point cloud.
  Handle(asiAlgo_BaseCloud<double>) cloud = new asiAlgo_BaseCloud<double>;
  //
  for ( int i = tris->Nodes().Lower(); i <= tris->Nodes().Upper(); ++i )
    cloud->AddElement( tris->Nodes()(i).XYZ() );

  // Set result.
  interp->GetPlotter().REDRAW_POINTS(argv[1], cloud->GetCoordsArray(), Color_Default);
  return TCL_OK;
}

//-----------------------------------------------------------------------------

int RE_GetInnerPoints(const Handle(asiTcl_Interp)& interp,
                      int                          argc,
                      const char**                 argv)
{
  if ( argc != 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Patch Node.
  Handle(asiData_RePatchNode)
    patchNode = Handle(asiData_RePatchNode)::DownCast( cmdRE::model->FindNodeByName(argv[2]) );
  //
  if ( patchNode.IsNull() || !patchNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Object with name '%1' is not a patch."
                                                        << argv[2]);
    return TCL_ERROR;
  }

  // Prepare service API.
  asiEngine_RE api( cmdRE::model, interp->GetProgress(), interp->GetPlotter() );

  // Get triangles captured by contour.
  Handle(Poly_Triangulation) regionTris;
  //
  if ( !api.ExtractBoundedRegion(patchNode, regionTris) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot extract the region captured by contour of patch '%1'."
                                                        << argv[2]);
    return TCL_ERROR;
  }

  // Get nodes of the captured region.
  Handle(asiAlgo_BaseCloud<double>) pts = new asiAlgo_BaseCloud<double>;
  //
  for ( int i = 1; i <= regionTris->NbNodes(); ++i )
  {
    const gp_Pnt& P = regionTris->Node(i);
    pts->AddElement( P.X(), P.Y(), P.Z() );
  }

  // Set the result.
  interp->GetPlotter().REDRAW_POINTS(argv[1], pts->GetCoordsArray(), Color_Default);
  return TCL_OK;
}

//-----------------------------------------------------------------------------

int RE_Topologize(const Handle(asiTcl_Interp)& interp,
                  int                          argc,
                  const char**                 argv)
{
  if ( argc != 1 && argc != 2 && argc != 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Number of faces to produce.
  const int numFaces = ( (argc == 1) ? 10 : atoi( argv[1] ) );
  //
  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Number of faces to produce is %1."
                                                       << numFaces);

  // Tolerance for projection.
  const double projTol = ( (argc == 2) ? 0.001 : atof( argv[2] ) );

  // Get triangulation.
  Handle(Poly_Triangulation)
    tris = cmdRE::model->GetTriangulationNode()->GetTriangulation();
  //
  if ( tris.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Triangulation is null.");
    return TCL_ERROR;
  }

  // Find Instant Meshes executable.
  QString qimEnv = qgetenv("INSTANT_MESHES_EXE");
  //
  if ( !QDir().exists(qimEnv) )
  {
    interp->GetProgress().SendLogMessage( LogErr(Normal) << "Cannot find executable '%1'."
                                                         << QStr2AsciiStr(qimEnv) );
    return TCL_ERROR;
  }
  //
  interp->GetProgress().SendLogMessage( LogInfo(Normal) << "Instant Meshes executable: '%1'."
                                                        << QStr2AsciiStr(qimEnv) );

  // Save triangulation to feed Instant Meshes.
  TCollection_AsciiString imInput  = "C:/users/ssv/desktop/imInput.ply";
  TCollection_AsciiString imOutput = "C:/users/ssv/desktop/imOutput.obj";
  //
  if ( !asiAlgo_Utils::WritePly( tris, imInput, interp->GetProgress() ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot save temporary mesh file.");
    return TCL_ERROR;
  }

  // Prepare arguments.
  QStringList imArgs;
  //
  imArgs << "-f" << QString::number(numFaces)
         << "-o" << imOutput.ToCString()
         << "-d"
         << imInput.ToCString();

  // Run conversion process.
  QProcess pProcess(cmdRE::cf->MainWindow);
  pProcess.start(qimEnv, imArgs);
  //
  if ( !pProcess.waitForStarted() )
    return TCL_ERROR;
  //
  if ( !pProcess.waitForFinished() )
    return TCL_ERROR;

  // Load OBJ to the Tessellation Node.
  Handle(ActData_Mesh) mesh;
  //
  if ( !asiAlgo_Utils::ReadObj( imOutput, mesh, interp->GetProgress() ) )
  {
    interp->GetProgress().SendLogMessage( LogErr(Normal) << "Cannot read OBJ file '%1'." << imOutput );
    return TCL_ERROR;
  }
  //
  interp->GetProgress().SendLogMessage( LogInfo(Normal) << "Loaded mesh from '%1'." << imOutput );

  // Set mesh.
  Handle(asiData_TessNode) tess_n = cmdRE::model->GetTessellationNode();
  //
  cmdRE::model->OpenCommand(); // tx start
  {
    tess_n->SetMesh(mesh);
  }
  cmdRE::model->CommitCommand(); // tx commit

  // All edges.
  Handle(ActAPI_HNodeList) edgesList = new ActAPI_HNodeList;

  // Build BVH for CAD-agnostic mesh.
  Handle(asiAlgo_BVHFacets) bvh;
  //
  asiEngine_RE            reApi   ( cmdRE::model, interp->GetProgress(), interp->GetPlotter() );
  asiEngine_Triangulation trisApi ( cmdRE::model, interp->GetProgress(), interp->GetPlotter() );
  //
  cmdRE::model->OpenCommand();
  {
    bvh = trisApi.BuildBVH();

    // Tool to project points on mesh.
    asiAlgo_ProjectPointOnMesh pointToMesh(bvh);

    // Constructed topological primitives.
    NCollection_DataMap<int, Handle(asiData_ReVertexNode)>                              vertices;
    NCollection_DataMap<t_undirectedLink, Handle(asiData_ReEdgeNode), t_undirectedLink> edges;

    // Initialize progress indicator.
    interp->GetProgress().Init( mesh->NbFaces() );
    interp->GetProgress().SetMessageKey("Process quads");

    // Iterate over the quads.
    int patchIdx = 0, edgeIdx = 0, vertexIdx = 0;
    for ( ActData_Mesh_ElementsIterator it(mesh, ActData_Mesh_ET_Face); it.More(); it.Next() )
    {
      interp->GetProgress().StepProgress(1);

      // Get next quad.
      const Handle(ActData_Mesh_Element)& elem = it.GetValue();
      //
      if ( !elem->IsKind( STANDARD_TYPE(ActData_Mesh_Quadrangle) ) )
        continue;

      const Handle(ActData_Mesh_Quadrangle)&
        quad = Handle(ActData_Mesh_Quadrangle)::DownCast(elem);

      ++patchIdx;
      TCollection_ExtendedString patchName("Patch "); patchName += patchIdx;

      // Create patch.
      Handle(asiData_RePatchNode) patch = reApi.Create_Patch(patchName);

      // Get nodes.
      int* nodeIDs = (int*) quad->GetConnections();

      // Create vertices.
      std::vector<Handle(asiData_ReVertexNode)> quadVertices;
      std::vector<int>                          quadVerticesIds;
      //
      for ( int k = 0; k < 4; ++k )
      {
        const int n = nodeIDs[k];

        if ( !vertices.IsBound(n) )
        {
          const gp_Pnt& node         = mesh->FindNode(n)->Pnt();
          gp_Pnt        nodeProj     = pointToMesh.Perform(node);
          int           nodeFacetInd = pointToMesh.GetFacetIds().size() ? pointToMesh.GetFacetIds()[0] : -1;
          //
          if ( nodeFacetInd == -1 )
          {
            interp->GetProgress().SendLogMessage(LogErr(Normal) << "Failed to project network node.");
            continue;
          }

          ++vertexIdx;
          TCollection_ExtendedString vertexName("Vertex "); vertexName += vertexIdx;

          // Create vertex.
          Handle(asiData_ReVertexNode)
            V = reApi.Create_Vertex( vertexName,
                                     nodeProj.XYZ(),
                                     bvh->GetFacet(nodeFacetInd).N.XYZ() );
          //
          vertices.Bind(n, V);

          // Store vertices of the current quad.
          quadVertices.push_back(V);
          quadVerticesIds.push_back(n);
        }
        else
        {
          quadVertices.push_back( vertices(n) );
          quadVerticesIds.push_back(n);
        }
      }

      // Create edges/coedges.
      for ( int k = 0; k < 4; ++k )
      {
        const int currIdx = k;
        const int nextIdx = ((k == 3) ? 0 : k + 1);

        const int n1 = quadVerticesIds[currIdx];
        const int n2 = quadVerticesIds[nextIdx];
        t_undirectedLink n1n2(n1, n2);

        // Create or take an existing edge.
        bool sameSense;
        Handle(asiData_ReEdgeNode) edge;
        //
        if ( !edges.IsBound(n1n2) )
        {
          ++edgeIdx;
          TCollection_ExtendedString edgeName("Edge "); edgeName += edgeIdx;

          edge = reApi.Create_Edge(edgeName, quadVertices[currIdx], quadVertices[nextIdx]);
          edges.Bind(n1n2, edge);
          edgesList->Append(edge);
          sameSense = true;

          // Tool for line projection.
          asiAlgo_MeshProjectLine projectLineMesh( bvh,
                                                   interp->GetProgress(),
                                                   interp->GetPlotter() );

          // Project line between vertices n1 and n2 to mesh.
          std::vector<gp_XYZ> projPts;
          std::vector<int>    projInds;
          //
          if ( !projectLineMesh.Perform(quadVertices[currIdx]->GetPoint(),
                                        quadVertices[nextIdx]->GetPoint(),
                                        projPts, projInds, projTol) )
          {
            interp->GetProgress().SendLogMessage(LogWarn(Normal) << "Cannot project line to mesh.");
            continue;
          }

          // Store projection.
          if ( projPts.size() > 2 )
          {
            for ( size_t kk = 0; kk < projPts.size(); ++kk )
              edge->AddPolylinePole(projPts[kk], projInds[kk]);

            edge->AddPolylinePole(quadVertices[nextIdx]->GetPoint(), -1);
          }
        }
        else
        {
          edge      = edges(n1n2);
          sameSense = false;
        }

        // Create coedge.
        reApi.Create_CoEdge(patch, edge, sameSense);
      }
    }
  } cmdRE::model->CommitCommand();

  // Progress indication.
  interp->GetProgress().Init();
  interp->GetProgress().SetMessageKey("Actualize scene");

  // Update UI.
  if ( cmdRE::cf->ViewerPart )
  {
    cmdRE::cf->ViewerPart->PrsMgr()->ActualizeCol ( edgesList, false, false, false, false );
    cmdRE::cf->ViewerPart->PrsMgr()->Actualize    ( tess_n,    false, false, false, false );
    //
    cmdRE::cf->ViewerPart->Repaint();
  }
  //
  if ( cmdRE::cf->ObjectBrowser )
    cmdRE::cf->ObjectBrowser->Populate();

  interp->GetProgress().SetProgressStatus(ActAPI_ProgressStatus::Progress_Succeeded);
  return TCL_OK;
}

//-----------------------------------------------------------------------------

int RE_SmoothenEdges(const Handle(asiTcl_Interp)& interp,
                     int                          argc,
                     const char**                 argv)
{
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  asiEngine_RE reApi(cmdRE::model);

  cmdRE::model->OpenCommand();
  {
    // Loop over the edges and enable smoothing for those
    // that are regular.
    Handle(asiData_ReEdgesNode) edgesNode = reApi.Get_Edges();
    //
    for ( Handle(ActAPI_IChildIterator) cit = edgesNode->GetChildIterator(); cit->More(); cit->Next() )
    {
      Handle(asiData_ReEdgeNode)
        edgeNode = Handle(asiData_ReEdgeNode)::DownCast( cit->Value() );

      Handle(asiData_ReVertexNode) fVertexNode = edgeNode->GetFirstVertex();
      Handle(asiData_ReVertexNode) lVertexNode = edgeNode->GetLastVertex();

      // Get valence of the first vertex.
      int fValence = 0;
      {
        Handle(ActAPI_HParameterList) backrefs = fVertexNode->GetReferrers();
        //
        for ( ActAPI_HParameterList::Iterator rit(*backrefs); rit.More(); rit.Next() )
        {
          Handle(asiData_ReEdgeNode)
            refEdgeNode = Handle(asiData_ReEdgeNode)::DownCast( rit.Value()->GetNode() );
          //
          if ( !refEdgeNode.IsNull() )
            fValence++;
        }
      }

      // Get valence of the second vertex.
      int lValence = 0;
      {
        Handle(ActAPI_HParameterList) backrefs = lVertexNode->GetReferrers();
        //
        for ( ActAPI_HParameterList::Iterator rit(*backrefs); rit.More(); rit.Next() )
        {
          Handle(asiData_ReEdgeNode)
            refEdgeNode = Handle(asiData_ReEdgeNode)::DownCast( rit.Value()->GetNode() );
          //
          if ( !refEdgeNode.IsNull() )
            lValence++;
        }
      }

      if ( (fValence == 4) && (lValence == 4) )
      {
        edgeNode->SetSmoothTransition(true);

        reApi.ReconnectSmoothenCornersFunc(edgeNode);
        reApi.ReconnectSmoothenPatchesFunc(edgeNode);
      }
    }

    // Execute functions.
    cmdRE::model->FuncExecuteAll();
  }
  cmdRE::model->CommitCommand();

  // Actualize.
  cmdRE::cf->ViewerPart->PrsMgr()->Actualize(reApi.Get_Patches(), true, false, false, false);
  cmdRE::cf->ViewerPart->PrsMgr()->Actualize(reApi.Get_Edges(),   true, false, false, false);
  cmdRE::cf->ViewerPart->Repaint();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

void cmdRE::Commands_Modeling(const Handle(asiTcl_Interp)&      interp,
                              const Handle(Standard_Transient)& cmdRE_NotUsed(data))
{
  static const char* group = "cmdRE";

  //-------------------------------------------------------------------------//
  interp->AddCommand("re-smoothen-regular-edges",
    //
    "re-smoothen-regular-edges\n"
    "\t Finds all regular edges in the topology and enables smoothing\n"
    "\t mode for them.",
    //
    __FILE__, group, RE_SmoothenRegularEdges);

  //-------------------------------------------------------------------------//
  interp->AddCommand("re-build-patches",
    //
    "re-build-patches [<patchName1> [<patchName2> ...]] [-fair <coeff>] [-approx]\n"
    "\t Constructs surface patched for the passed data object(s).",
    //
    __FILE__, group, RE_BuildPatches);

  //-------------------------------------------------------------------------//
  interp->AddCommand("re-build-contour-lines",
    //
    "re-build-contour-lines [<edgeName1> [<edgeName2> ...]] [-toler <toler>]\n"
    "\t Constructs contour lines either for all patches or for the passed\n"
    "\t edges only.",
    //
    __FILE__, group, RE_BuildContourLines);

  //-------------------------------------------------------------------------//
  interp->AddCommand("re-fair-contour-lines",
    //
    "re-fair-contour-lines <coeff>\n"
    "\t Fairs (smooths) contour lines for all patches with the given fairing\n"
    "\t coefficient.",
    //
    __FILE__, group, RE_FairContourLines);

  //-------------------------------------------------------------------------//
  interp->AddCommand("re-cut-with-plane",
    //
    "re-cut-with-plane <res> <p> [-nosort]\n"
    "\t Cuts triangulation with plane. If '-nosort' key is passed, the\n"
    "\t resulting points are not post-processed with K-neighbors hull\n"
    "\t algorithm thus remaining disordered.",
    //
    __FILE__, group, RE_CutWithPlane);

  //-------------------------------------------------------------------------//
  interp->AddCommand("re-approx-points",
    //
    "re-approx-points resCurve points prec [-closed]\n"
    "\t Attempts to approximate the given point cloud with a curve.",
    //
    __FILE__, group, RE_ApproxPoints);

  //-------------------------------------------------------------------------//
  interp->AddCommand("re-skin-surface",
    //
    "re-skin-surface <resSurf> <vDegree> [-fair-isos <lambda>] <curveName1> ... <curveNameK>\n"
    "\t Interpolates surface by skinning the passed B-curves.",
    //
    __FILE__, group, RE_SkinSurface);

  //-------------------------------------------------------------------------//
  interp->AddCommand("re-interp-multicurve",
    //
    "re-interp-multicurve <numpts> <degree> <curveName1> [<curveName2> [...]]\n"
    "\t Interpolates a set of curves on the same knot sequence.",
    //
    __FILE__, group, RE_InterpMulticurve);

  //-------------------------------------------------------------------------//
  interp->AddCommand("re-join-curves",
    //
    "re-join-curves <res> <curveName1> <curveName2> [<curveName3> [...]]\n"
    "\t Joins curves into a single curve.",
    //
    __FILE__, group, RE_JoinCurves);

  //-------------------------------------------------------------------------//
  interp->AddCommand("re-check-surf-deviation",
    //
    "re-check-surf-deviation <res> <surfName>\n"
    "\t Checks deviation between the given surface and the reference mesh.",
    //
    __FILE__, group, RE_CheckSurfDeviation);

  //-------------------------------------------------------------------------//
  interp->AddCommand("re-check-deviation",
    //
    "re-check-deviation <pointsName>\n"
    "\t Checks deviation between the given point cloud and the active CAD part.",
    //
    __FILE__, group, RE_CheckDeviation);

  //-------------------------------------------------------------------------//
  interp->AddCommand("re-check-tri-deviation",
    //
    "re-check-tri-deviation <pointsName>\n"
    "\t Checks deviation between the given point cloud and the active triangulation.",
    //
    __FILE__, group, RE_CheckTriDeviation);

  //-------------------------------------------------------------------------//
  interp->AddCommand("re-make-average-plane",
    //
    "re-make-average-plane <res> <pointsName> [{<umin> <umax> <vmin> <vmax> | -mobius}]\n"
    "\t Approximates the given point cloud with a plane.",
    //
    __FILE__, group, RE_MakeAveragePlane);

  //-------------------------------------------------------------------------//
  interp->AddCommand("re-sample-part",
    //
    "re-sample-part <res> <ustep> <vstep> [-facets] [-vertices]\n"
    "\t Makes a point cloud by sampling CAD part.",
    //
    __FILE__, group, RE_SamplePart);

  //-------------------------------------------------------------------------//
  interp->AddCommand("re-invert-points",
    //
    "re-invert-bpoles <surfName> <ptsName> [-opencascade]\n"
    "\t Inverts the passed point cloud to the B-surface with the given name.",
    //
    __FILE__, group, RE_InvertPoints);

  //-------------------------------------------------------------------------//
  interp->AddCommand("re-purify-cloud",
    //
    "re-purify-cloud <resPtsName> <ptsName> <tol3d>\n"
    "\t Purifies point cloud by removing near-coincident points. The precision\n"
    "\t used for coincidence test is passed as <tol3d> argument.",
    //
    __FILE__, group, RE_PurifyCloud);

  //-------------------------------------------------------------------------//
  interp->AddCommand("re-approx-surf",
    //
    "re-approx-surf <resSurf> <ptsName> {<uDegree> <vDegree> | -init <initSurf>} [-lambda <coeff>] [{-pinned | -pinned2}]\n"
    "\t Approximates point cloud with B-surface.",
    //
    __FILE__, group, RE_ApproxSurf);

  //-------------------------------------------------------------------------//
  interp->AddCommand("re-get-triangulation-nodes",
    //
    "re-get-triangulation-nodes <resPtsName>\n"
    "\t Extracts triangulation nodes as a point cloud.",
    //
    __FILE__, group, RE_GetTriangulationNodes);

  //-------------------------------------------------------------------------//
  interp->AddCommand("re-get-inner-points",
    //
    "re-get-inner-points <resName> <patchName>\n"
    "\t Extracts the mesh nodes which are bounded by the contour of the given\n"
    "\t patch specified as <patchName>. The extracted nodes are collected in the\n"
    "\t point cloud specified as <resName>.",
    //
    __FILE__, group, RE_GetInnerPoints);

  //-------------------------------------------------------------------------//
  interp->AddCommand("re-topologize",
    //
    "re-topologize <numFaces> <projToler>\n"
    "\t Attempts to topologize the active triangulation with the quads from\n"
    "\t the active tessellation.",
    //
    __FILE__, group, RE_Topologize);

  //-------------------------------------------------------------------------//
  interp->AddCommand("re-smoothen-edges",
    //
    "re-smoothen-edges\n"
    "\t Enables smoothing for all regular edges.",
    //
    __FILE__, group, RE_SmoothenEdges);
}
