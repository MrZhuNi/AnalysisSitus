//-----------------------------------------------------------------------------
// Created on: 21 September 2016
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
#include <asiAlgo_HitFacet.h>

// Geometry includes
#include <asiAlgo_BVHIterator.h>

// OCCT includes
#include <BRep_Builder.hxx>
#include <gp_Lin.hxx>
#include <Precision.hxx>

#undef DRAW_DEBUG
#if defined DRAW_DEBUG
#include <BRepBuilderAPI_MakeEdge.hxx>
#endif

//-----------------------------------------------------------------------------

typedef int BVH_StackItem;

//-----------------------------------------------------------------------------

asiAlgo_HitFacet::asiAlgo_HitFacet(const Handle(asiAlgo_BVHFacets)& facets,
                                   ActAPI_ProgressEntry             progress,
                                   ActAPI_PlotterEntry              plotter)
: asiAlgo_BVHAlgo(facets, progress, plotter), m_iFaceToSkip(0)
{
  this->SetMode(Mode_Nearest);
}

//-----------------------------------------------------------------------------

bool asiAlgo_HitFacet::operator()(const gp_Lin& ray,
                                  int&          facetId,
                                  gp_XYZ&       hit) const
{
  std::vector<int>    facetIds;
  std::vector<gp_XYZ> hits;

  if ( this->operator()(ray, facetIds, hits) )
  {
    facetId = facetIds[0];
    hit     = hits[0];
    return true;
  }

  facetId = -1;
  return false;
}

//-----------------------------------------------------------------------------

bool asiAlgo_HitFacet::operator()(const gp_Lin&        ray,
                                  std::vector<int>&    facetIds,
                                  std::vector<gp_XYZ>& hits) const
{
  const opencascade::handle< BVH_Tree<double, 3> >& bvh = m_facets->BVH();
  if ( bvh.IsNull() )
    return false;

  // Initialize outputs.
  facetIds.clear();
  hits.clear();

  // Initialize facet index for a single-point test.
  int    facetId = -1;
  gp_XYZ hit;

  // Limit of the ray for hit test.
  const double ray_limit = m_facets->GetBoundingDiag()*100;

  // Precision for fast intersection test on AABB.
  const double prec = Precision::Confusion();

  // Intersection parameter for sorting.
  double resultRayParam = 0.;
  if ( m_mode == Mode_Farthest )
    resultRayParam = -RealLast();
  else
    resultRayParam = RealLast();

#if defined DRAW_DEBUG
  this->GetPlotter().REDRAW_POINT("ray_origin", ray.Location(), Color_Red);
  this->GetPlotter().REDRAW_LINK("ray", ray.Location(), ray.Location().XYZ() + ray.Direction().XYZ()*ray_limit, Color_Green);
#endif

  // Traverse BVH.
  for ( asiAlgo_BVHIterator it(bvh); it.More(); it.Next() )
  {
    const BVH_Vec4i& nodeData = it.Current();

    if ( it.IsLeaf() )
    {
      // If we are here, then we are close to solution. It is a right
      // time now to perform a precise check.

      int    facet_candidate = -1;
      double hitParam;
      gp_XYZ hitPoint;
      //
      const bool isHit = this->testLeaf(ray, ray_limit, nodeData, facet_candidate, hitParam, hitPoint);
      //
      if ( isHit )
      {
        if ( ( (m_mode == Mode_Farthest) && (hitParam > resultRayParam) ) ||
             ( (m_mode == Mode_Nearest) && (hitParam < resultRayParam) ) )
        {
          facetId        = facet_candidate;
          resultRayParam = hitParam;
          hit            = hitPoint;
        }
        else if ( m_mode == Mode_All )
        {
          facetIds.push_back(facet_candidate);
          hits.push_back(hitPoint);
        }
      }
    }
    else // sub-volume.
    {
      const BVH_Vec3d& minCorner_Left  = bvh->MinPoint( nodeData.y() );
      const BVH_Vec3d& maxCorner_Left  = bvh->MaxPoint( nodeData.y() );
      const BVH_Vec3d& minCorner_Right = bvh->MinPoint( nodeData.z() );
      const BVH_Vec3d& maxCorner_Right = bvh->MaxPoint( nodeData.z() );

      const bool isLeftOut  = this->isOut(ray, minCorner_Left, maxCorner_Left, prec);
      const bool isRightOut = this->isOut(ray, minCorner_Right, maxCorner_Right, prec);

      if ( isLeftOut )
        it.BlockLeft();
      if ( isRightOut )
        it.BlockRight();
    }
  }

#if defined DRAW_DEBUG
  if ( facet_index != -1 )
  {
    std::cout << "Found intersection with facet "
              << facet_index
              << " on face "
              << m_facets->GetFacet(facet_index).FaceIndex
              << " for ray parameter "
              << resultRayParam
              << std::endl;

    const asiAlgo_BVHFacets::t_facet& facet = m_facets->GetFacet(facet_index);
    //
    this->GetPlotter().DRAW_LINK( gp_Pnt( facet.P0.x(), facet.P0.y(), facet.P0.z() ),
                                  gp_Pnt( facet.P1.x(), facet.P1.y(), facet.P1.z() ), Color_Red );
    this->GetPlotter().DRAW_LINK( gp_Pnt( facet.P0.x(), facet.P0.y(), facet.P0.z() ),
                                  gp_Pnt( facet.P2.x(), facet.P2.y(), facet.P2.z() ), Color_Red );
    this->GetPlotter().DRAW_LINK( gp_Pnt( facet.P1.x(), facet.P1.y(), facet.P1.z() ),
                                  gp_Pnt( facet.P2.x(), facet.P2.y(), facet.P2.z() ), Color_Red );
  }
  else
    std::cout << "Error: cannot find the intersected facet" << std::endl;
#endif

  // In a single-point mode, populate the output collection.
  if ( (m_mode != Mode_All) && (facetId != -1) )
  {
    facetIds.push_back(facetId);
    hits.push_back(hit);
  }

  return (facetIds.size() > 0);
}

//-----------------------------------------------------------------------------

double asiAlgo_HitFacet::operator()(const gp_Pnt& P,
                                    const double  membership_prec,
                                    gp_Pnt&       P_proj,
                                    int&          facet_index) const
{
  const opencascade::handle< BVH_Tree<double, 3> >& bvh = m_facets->BVH();
  if ( bvh.IsNull() )
    return false;

  // Initialize output index
  facet_index = -1;

  // Precision for fast intersection test on AABB
  const double intersectPrec = Precision::Confusion();

  // Distance for sorting
  double minDist = RealLast();

  // Traverse BVH
  for ( asiAlgo_BVHIterator it(bvh); it.More(); it.Next() )
  {
    const BVH_Vec4i& nodeData = it.Current();

    if ( it.IsLeaf() )
    {
      // If we are here, then we are close to solution. It is a right
      // time now to perform a precise check

      int    facet_candidate = -1;
      bool   isInside        = false;
      gp_Pnt pproj;
      //
      const double dist = this->testLeaf(P, nodeData, membership_prec, pproj, facet_candidate, isInside);
      //
      if ( isInside && dist < minDist )
      {
        P_proj      = pproj;
        facet_index = facet_candidate;
        minDist     = dist;
      }
    }
    else // sub-volume
    {
      const BVH_Vec3d& minCorner_Left  = bvh->MinPoint( nodeData.y() );
      const BVH_Vec3d& maxCorner_Left  = bvh->MaxPoint( nodeData.y() );
      const BVH_Vec3d& minCorner_Right = bvh->MinPoint( nodeData.z() );
      const BVH_Vec3d& maxCorner_Right = bvh->MaxPoint( nodeData.z() );

      const bool isLeftOut  = asiAlgo_BVHAlgo::isOut(minCorner_Left, maxCorner_Left, P, intersectPrec);
      const bool isRightOut = asiAlgo_BVHAlgo::isOut(minCorner_Right, maxCorner_Right, P, intersectPrec);

      if ( isLeftOut )
        it.BlockLeft();
      if ( isRightOut )
        it.BlockRight();
    }
  }

#if defined DRAW_DEBUG
  if ( facet_index != -1 )
  {
    std::cout << "Found host facet "
              << facet_index
              << " on face "
              << m_facets->GetFacet(facet_index).FaceIndex
              << " for min distance "
              << minDist
              << std::endl;

    const asiAlgo_BVHFacets::t_facet& facet = m_facets->GetFacet(facet_index);
    //
    this->GetPlotter().DRAW_LINK( gp_Pnt(facet.P0.x(), facet.P0.y(), facet.P0.z()),
                                  gp_Pnt(facet.P1.x(), facet.P1.y(), facet.P1.z()), Color_Red);
    this->GetPlotter().DRAW_LINK( gp_Pnt(facet.P0.x(), facet.P0.y(), facet.P0.z()),
                                  gp_Pnt(facet.P2.x(), facet.P2.y(), facet.P2.z()), Color_Red);
    this->GetPlotter().DRAW_LINK( gp_Pnt(facet.P1.x(), facet.P1.y(), facet.P1.z()),
                                  gp_Pnt(facet.P2.x(), facet.P2.y(), facet.P2.z()), Color_Red);
  }
  else
    std::cout << "Error: cannot find the host facet" << std::endl;
#endif

  return facet_index != -1;
}

//-----------------------------------------------------------------------------

double asiAlgo_HitFacet::testLeaf(const gp_Pnt&    P,
                                  const BVH_Vec4i& leaf,
                                  const double     membership_prec,
                                  gp_Pnt&          P_proj,
                                  int&             resultFacet,
                                  bool&            isInside) const
{
  gp_XYZ p       = P.XYZ();
  double minDist = 1e100;

  // Loop over the tentative facets
  for ( int fidx = leaf.y(); fidx <= leaf.z(); ++fidx )
  {
    // Get facet to test
    const asiAlgo_BVHFacets::t_facet& facet = m_facets->GetFacet(fidx);

    // Get next facet to test
    const gp_XYZ p0( facet.P0.x(), facet.P0.y(), facet.P0.z() );
    const gp_XYZ p1( facet.P1.x(), facet.P1.y(), facet.P1.z() );
    const gp_XYZ p2( facet.P2.x(), facet.P2.y(), facet.P2.z() );

    const gp_XYZ p0p1     = p1 - p0;
    const gp_XYZ p0p2     = p2 - p0;
    const gp_XYZ p0p      = p - p0;
    const double p0p_dist = p0p.Modulus();

    // Compute normal vector
    const gp_XYZ Np = p0p1 ^ p0p2;
    if ( Np.SquareModulus() < RealEpsilon() )
      continue;

    const double cos_alpha = ( p0p * Np ) / ( p0p_dist * Np.Modulus() );
    const double dist      = p0p_dist * cos_alpha;
    const gp_XYZ pproj     = p - Np.Normalized()*dist;

#if defined COUT_DEBUG
    std::cout << "Probe's projection is ("
              << pproj.X() << ", "
              << pproj.Y() << ", "
              << pproj.Z() << ")" << std::endl;
#endif

    if ( this->isInsideBarycentric(pproj, p0, p1, p2, membership_prec) && Abs(dist) <= minDist )
    {
      P_proj      = pproj;
      isInside    = true;
      minDist     = Abs(dist);
      resultFacet = fidx;

#if defined DRAW_DEBUG
      this->GetPlotter().DRAW_LINK(p0, p1, Color_Green);
      this->GetPlotter().DRAW_LINK(p0, p2, Color_Green);
      this->GetPlotter().DRAW_LINK(p1, p2, Color_Green);
#endif
    }
  }

  return minDist;
}

//-----------------------------------------------------------------------------

bool asiAlgo_HitFacet::testLeaf(const gp_Lin&    ray,
                                const double     length,
                                const BVH_Vec4i& leaf,
                                int&             resultFacet,
                                double&          resultRayParamNormalized,
                                gp_XYZ&          hitPoint) const
{
  // Prepare a segment of the ray to pass for intersection test.
  const gp_XYZ l0 = ray.Location().XYZ();
  const gp_XYZ l1 = l0 + ray.Direction().XYZ()*length;

  // Parameter on ray is used for intersection sorting. We are interested
  // in the nearest or the farthest point depending on the mode specified
  // by the user.
  resultFacet              = -1;
  resultRayParamNormalized = RealLast();

  // Loop over the tentative facets
  for ( int fidx = leaf.y(); fidx <= leaf.z(); ++fidx )
  {
    // Get facet to test.
    const asiAlgo_BVHFacets::t_facet& facet = m_facets->GetFacet(fidx);

    if ( facet.FaceIndex == m_iFaceToSkip )
      continue; // Skip facet which is explicitly excluded from the intersection test.

    // Get next facet to test.
    const gp_XYZ p0( facet.P0.x(), facet.P0.y(), facet.P0.z() );
    const gp_XYZ p1( facet.P1.x(), facet.P1.y(), facet.P1.z() );
    const gp_XYZ p2( facet.P2.x(), facet.P2.y(), facet.P2.z() );

    // Hit test.
    double currentParam;
    gp_XYZ currentPoint;
    //
    if ( this->isIntersected(l0, l1, p0, p1, p2, currentParam, currentPoint) )
    {
      if ( currentParam < resultRayParamNormalized )
      {
        if ( !m_plotter.Access().IsNull() )
        {
          TCollection_AsciiString name("facet_");
          name += fidx;
          m_plotter.REDRAW_TRIANGLE(name, p0, p1, p2, Color_Red);
        }

        resultFacet              = fidx;
        resultRayParamNormalized = currentParam;
        hitPoint                 = currentPoint;
      }
    }
  }

  return (resultFacet != -1);
}

//-----------------------------------------------------------------------------

bool asiAlgo_HitFacet::isOut(const gp_Lin&    L,
                             const BVH_Vec3d& boxMin,
                             const BVH_Vec3d& boxMax,
                             const double     prec) const
{
  double xmin = 0, xmax = 0, ymin = 0, ymax = 0, zmin = 0, zmax = 0;
  double parmin, parmax, par1, par2;
  bool   xToSet, yToSet;

  // Protection from degenerated bounding box.
  double myXmin = boxMin.x() - prec;
  double myYmin = boxMin.y() - prec;
  double myZmin = boxMin.z() - prec;
  double myXmax = boxMax.x() + prec;
  double myYmax = boxMax.y() + prec;
  double myZmax = boxMax.z() + prec;

  if ( Abs( L.Direction().XYZ().X() ) > 0.0 )
  {
    par1   = (myXmin - L.Location().XYZ().X()) / L.Direction().XYZ().X();
    par2   = (myXmax - L.Location().XYZ().X()) / L.Direction().XYZ().X();
    parmin = Min(par1, par2);
    parmax = Max(par1, par2);
    xToSet = true;
  }
  else
  {
    if ( L.Location().XYZ().X() < myXmin || myXmax < L.Location().XYZ().X() )
      return true;

    xmin   = L.Location().XYZ().X();
    xmax   = L.Location().XYZ().X();
    parmin = -1.0e100;
    parmax = 1.0e100;
    xToSet = false;
  }

  if ( Abs( L.Direction().XYZ().Y() ) > 0.0 )
  {
    par1 = ( myYmin - L.Location().XYZ().Y() ) / L.Direction().XYZ().Y();
    par2 = ( myYmax - L.Location().XYZ().Y() ) / L.Direction().XYZ().Y();

    if ( parmax < Min(par1, par2) || parmin > Max(par1, par2) )
      return true;

    parmin = Max(parmin, Min(par1, par2));
    parmax = Min(parmax, Max(par1, par2));
    yToSet = true;
  }
  else 
  {
    if ( L.Location().XYZ().Y() < myYmin || myYmax < L.Location().XYZ().Y() )
      return true;

    ymin   = L.Location().XYZ().Y();
    ymax   = L.Location().XYZ().Y();
    yToSet = false;
  }

  if ( Abs( L.Direction().XYZ().Z() ) > 0.0 )
  {
    par1 = (myZmin - L.Location().XYZ().Z()) / L.Direction().XYZ().Z();
    par2 = (myZmax - L.Location().XYZ().Z()) / L.Direction().XYZ().Z();

    if ( parmax < Min(par1, par2) || parmin > Max(par1, par2) )
      return true;

    parmin = Max(parmin, Min(par1, par2));
    parmax = Min(parmax, Max(par1, par2));
    par1 = L.Location().XYZ().Z() + parmin*L.Direction().XYZ().Z();
    par2 = L.Location().XYZ().Z() + parmax*L.Direction().XYZ().Z();
    zmin = Min(par1, par2);
    zmax = Max(par1, par2);
  }
  else 
  {
    if ( L.Location().XYZ().Z() < myZmin || myZmax < L.Location().XYZ().Z() )
      return true;

    zmin = L.Location().XYZ().Z();
    zmax = L.Location().XYZ().Z();
  }
  if ( zmax < myZmin || myZmax < zmin )
    return true;

  if ( xToSet )
  {
    par1 = L.Location().XYZ().X() + parmin*L.Direction().XYZ().X();
    par2 = L.Location().XYZ().X() + parmax*L.Direction().XYZ().X();
    xmin = Min(par1, par2);
    xmax = Max(par1, par2);
  }
  if ( xmax < myXmin || myXmax < xmin )
    return true;

  if ( yToSet )
  {
    par1 = L.Location().XYZ().Y() + parmin*L.Direction().XYZ().Y();
    par2 = L.Location().XYZ().Y() + parmax*L.Direction().XYZ().Y();
    ymin = Min(par1, par2);
    ymax = Max(par1, par2);
  }
  if ( ymax < myYmin || myYmax < ymin )
    return true;

  return false;
}

//-----------------------------------------------------------------------------

bool asiAlgo_HitFacet::isSameSide(const gp_Pnt& p1, const gp_Pnt& p2,
                                  const gp_Pnt& a,  const gp_Pnt& b) const
{
  const gp_XYZ ab  = b.XYZ()  - a.XYZ();
  const gp_XYZ ap1 = p1.XYZ() - a.XYZ();
  const gp_XYZ ap2 = p2.XYZ() - a.XYZ();

  const gp_XYZ cp1 = ab ^ ap1;
  const gp_XYZ cp2 = ab ^ ap2;

  const double dot = cp1 * cp2;
  return dot >= 0;
}

//-----------------------------------------------------------------------------

bool asiAlgo_HitFacet::isInside(const gp_Pnt& p,
                                const gp_Pnt& a,
                                const gp_Pnt& b,
                                const gp_Pnt& c) const
{
  return this->isSameSide(p, a, /* */ b, c) &&
         this->isSameSide(p, b, /* */ a, c) &&
         this->isSameSide(p, c, /* */ a, b);
}

//-----------------------------------------------------------------------------

bool asiAlgo_HitFacet::isInsideBarycentric(const gp_Pnt& p,
                                           const gp_Pnt& a,
                                           const gp_Pnt& b,
                                           const gp_Pnt& c,
                                           const double  membership_prec) const
{
  // Compute vectors
  gp_XYZ v0 = c.XYZ() - a.XYZ();
  gp_XYZ v1 = b.XYZ() - a.XYZ();
  gp_XYZ v2 = p.XYZ() - a.XYZ();

  // Compute dot products
  const double dot00 = v0.Dot(v0);
  const double dot01 = v0.Dot(v1);
  const double dot02 = v0.Dot(v2);
  const double dot11 = v1.Dot(v1);
  const double dot12 = v1.Dot(v2);

  // Compute barycentric coordinates
  const double invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
  const double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
  const double v = (dot00 * dot12 - dot01 * dot02) * invDenom;

  // Check if point is in triangle
  return (u >= 0.0) &&
         (v >= 0.0) &&
       ( (u + v < 1) || (Abs(u + v - 1) < membership_prec) );
}

//-----------------------------------------------------------------------------

bool asiAlgo_HitFacet::isIntersected(const gp_XYZ& rayStart,
                                     const gp_XYZ& rayFinish,
                                     const gp_XYZ& pntTri1,
                                     const gp_XYZ& pntTri2,
                                     const gp_XYZ& pntTri3,
                                     double&       hitParamNormalized,
                                     gp_XYZ&       hitPoint) const
{
  // Moller�Trumbore intersection algorithm
  // (T. Moller et al, Fast Minimum Storage Ray / Triangle Intersection)
  gp_Vec e1(pntTri1, pntTri2),
         e2(pntTri1, pntTri3);
  gp_Vec dir(rayStart, rayFinish);
  gp_Vec P = dir.Crossed(e2);

  // If determinant is near zero, ray lies in plane of triangle or ray
  // is parallel to plane of triangle
  double det = e1.Dot(P);
  if ( Abs(det) < 1.0e-16 )
    return false;

  double inv_det = 1.0 / det;

  gp_Vec T(pntTri1, rayStart);

  // Calculate u parameter and test bound.
  double u = T.Dot(P) * inv_det;
  if ( u < 0.0 || u > 1.0 )
    return false; // Intersection lies outside the triangle

  // Calculate V parameter and test bound.
  gp_Vec Q = T.Crossed(e1);
  double v = dir.Dot(Q) * inv_det;
  if ( v < 0.0 || (u + v)  > 1.0 )
    return false; // Intersection lies outside the triangle

  double t           = e2.Dot(Q) * inv_det;
  hitParamNormalized = t;
  hitPoint           = (1 - u - v)*pntTri1 + u*pntTri2 + v*pntTri3;

  if ( t < 0.0 || t > 1.0 )
    return false;

  return true;
}
