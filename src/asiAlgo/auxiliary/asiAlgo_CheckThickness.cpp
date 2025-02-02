//-----------------------------------------------------------------------------
// Created on: 02 April 2020
//-----------------------------------------------------------------------------
// Copyright (c) 2020-present, Sergey Slyadnev
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
#include <asiAlgo_CheckThickness.h>

// asiAlgo includes
#include <asiAlgo_HitFacet.h>
#include <asiAlgo_MeshField.h>
#include <asiAlgo_MeshMerge.h>

// OpenCascade includes
#include <gp_Lin.hxx>

//-----------------------------------------------------------------------------

asiAlgo_CheckThickness::asiAlgo_CheckThickness(const TopoDS_Shape&  shape,
                                               ActAPI_ProgressEntry progress,
                                               ActAPI_PlotterEntry  plotter)
: ActAPI_IAlgorithm ( progress, plotter ),
  m_bIsCustomDir    ( false ),
  m_customDir       ( gp::DZ() ),
  m_fMinThick       ( 0. ),
  m_fMaxThick       ( 0. )
{
  // Merge facets.
  asiAlgo_MeshMerge meshMerge(shape);
  //
  m_resField.triangulation = meshMerge.GetResultPoly()->GetTriangulation();

  // Build BVH.
  m_bvh = new asiAlgo_BVHFacets(m_resField.triangulation);
}

//-----------------------------------------------------------------------------

asiAlgo_CheckThickness::asiAlgo_CheckThickness(const Handle(Poly_Triangulation)& tris,
                                               ActAPI_ProgressEntry              progress,
                                               ActAPI_PlotterEntry               plotter)
: ActAPI_IAlgorithm ( progress, plotter ),
  m_fMinThick       ( 0. ),
  m_fMaxThick       ( 0. )
{
  m_resField.triangulation = tris;

  // Build BVH.
  m_bvh = new asiAlgo_BVHFacets(m_resField.triangulation);
}

//-----------------------------------------------------------------------------

bool asiAlgo_CheckThickness::Perform_RayMethod()
{
  if ( m_resField.triangulation.IsNull() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Null triangulation.");
    return false;
  }

  asiAlgo_HitFacet HitFacet(m_bvh/*, m_progress, m_plotter*/);

  // Prepare scalar field.
  Handle(asiAlgo_MeshScalarField) field = new asiAlgo_MeshScalarField;
  m_resField.fields.push_back(field);

  // Cast a ray from each facet.
  double minScalar = DBL_MAX, maxScalar = -DBL_MAX;
  //
  for ( int tidx = 1; tidx <= m_resField.triangulation->NbTriangles(); ++tidx )
  {
    const Poly_Triangle& tri = m_resField.triangulation->Triangle(tidx);

    // Get nodes.
    int n1, n2, n3;
    tri.Get(n1, n2, n3);
    //
    gp_Pnt P0 = m_resField.triangulation->Node(n1);
    gp_Pnt P1 = m_resField.triangulation->Node(n2);
    gp_Pnt P2 = m_resField.triangulation->Node(n3);

    // Center point.
    gp_Pnt C = ( P0.XYZ() + P1.XYZ() + P2.XYZ() ) / 3.;

    /* Initialize norm. */

    gp_Vec V1(P0, P1);
    //
    if ( V1.SquareMagnitude() < 1e-8 )
      continue; // Skip invalid facet.
    //
    V1.Normalize();

    gp_Vec V2(P0, P2);
    //
    if ( V2.SquareMagnitude() < 1e-8 )
      continue; // Skip invalid facet.
    //
    V2.Normalize();

    // Compute norm.
    gp_Vec N = V1.Crossed(V2);
    //
    if ( N.SquareMagnitude() < 1e-8 )
      continue; // Skip invalid facet
    //
    N.Normalize();

    // Direction to analyze thickness.
    bool   isDirDefined = true;
    gp_Dir dir;
    gp_Dir localDir = N.Reversed();
    //
    if ( !m_bIsCustomDir )
    {
      dir = localDir;
    }
    else
    {
      if ( Abs( m_customDir.Dot(localDir) ) > 0.001 ) // Check for general position.
        dir = m_customDir;
      else
        isDirDefined = false;
    }

    if ( !isDirDefined )
      continue;

    /* Shoot a ray to find intersection. */

    // Exclude the originating face from the intersection test.
    HitFacet.SetFaceToSkip(tidx);

    // Thickness scalar.
    double thickness = 0.;

    // Do the intersection test. For the custom directions, the
    // test is done twice: in the forward and the reversed directions.
    gp_XYZ hit1, hit2, hit;
    int facetIdx1, facetIdx2, facetIdx = -1;
    //
    bool isHit1 = HitFacet(gp_Lin( C, dir ), facetIdx1, hit1);
    bool isHit2 = false;
    //
    if ( m_bIsCustomDir )
    {
      isHit2 = HitFacet(gp_Lin( C, dir.Reversed() ), facetIdx2, hit2);

      /*if ( tidx == 51121 )
      {
        m_plotter.REDRAW_POINT("C", C, Color_Blue);
        m_plotter.REDRAW_POINT("hit1", hit1, Color_Red);
        m_plotter.REDRAW_POINT("hit2", hit2, Color_Red);
        m_plotter.REDRAW_VECTOR_AT("dir1", C, dir, Color_Blue);
        m_plotter.REDRAW_VECTOR_AT("dir2", C, dir.Reversed(), Color_Blue);

        return false;
      }*/

      if ( !isHit1 && !isHit2 )
      {
        m_progress.SendLogMessage(LogWarn(Normal) << "Cannot find the intersected facet.");
      }
      else if ( isHit1 && !isHit2 )
      {
        hit      = hit1;
        facetIdx = facetIdx1;
      }
      else if ( !isHit1 && isHit2 )
      {
        hit      = hit2;
        facetIdx = facetIdx2;
      }
      else
      {
        // Choose the closest one.
        const double d1 = C.Distance(hit1);
        const double d2 = C.Distance(hit2);
        //
        hit      = ( (d1 < d2) ? hit1      : hit2 );
        facetIdx = ( (d1 < d2) ? facetIdx1 : facetIdx2 );
      }
    }
    else
    {
      if ( !isHit1 )
      {
        m_progress.SendLogMessage(LogWarn(Normal) << "Cannot find the intersected facet.");
      }
      else
      {
        hit      = hit1;
        facetIdx = facetIdx1;
      }
    }

    // Now thickness is simply a distance.
    if ( facetIdx != -1 )
      thickness = C.Distance(hit);

    /*if ( !m_bIsCustomDir && (tidx == 51121) )
    {
      m_plotter.REDRAW_POINT("C", C, Color_Blue);
      m_plotter.REDRAW_POINT("hit", hit, Color_Red);
      m_plotter.REDRAW_VECTOR_AT("N", C, N.Reversed(), Color_Blue);

      return false;
    }*/

    // Store scalars in the field.
    if ( facetIdx != -1 )
    {
      double *pThick = field->data.ChangeSeek(tidx);
      if ( pThick == nullptr )
        field->data.Bind(tidx, thickness);
      else if ( thickness > *pThick)
        *pThick = thickness;

      // Update the extreme values.
      if ( thickness < minScalar )
      {
        minScalar = thickness;
      }
      if ( thickness > maxScalar )
      {
        maxScalar = thickness;
      }
    }
  }

  // Set extreme thickness values.
  m_fMinThick = minScalar;
  m_fMaxThick = maxScalar;

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_CheckThickness::Perform_SphereMethod()
{
  m_progress.SendLogMessage(LogErr(Normal) << "Sphere-based method is not yet implemented.");
  return false;
}
