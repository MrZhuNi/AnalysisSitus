//-----------------------------------------------------------------------------
// Created on: 03 December 2016
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
#include <asiAlgo_PlaneOnPoints.h>

// Eigen includes
#pragma warning(disable : 4701 4702)
#include <Eigen/Dense>
#pragma warning(default : 4701 4702)

#undef COUT_DEBUG
#if defined COUT_DEBUG
  #pragma message("===== warning: COUT_DEBUG is enabled")
#endif

//-----------------------------------------------------------------------------

bool compare(const std::pair<double, int>& p1, const std::pair<double, int>& p2)
{
  return p1.first > p2.first;
}

//-----------------------------------------------------------------------------

asiAlgo_PlaneOnPoints::asiAlgo_PlaneOnPoints(ActAPI_ProgressEntry progress,
                                             ActAPI_PlotterEntry  plotter)
: ActAPI_IAlgorithm(progress, plotter)
{}

//-----------------------------------------------------------------------------

bool asiAlgo_PlaneOnPoints::Build(const std::vector<gp_XYZ>& points,
                                  gp_Pln&                    result) const
{
  return this->internalBuild(points, result);
}

//-----------------------------------------------------------------------------

bool asiAlgo_PlaneOnPoints::Build(const Handle(asiAlgo_BaseCloud<double>)& points,
                                  gp_Pln&                                  result) const
{
  // Repack point cloud to a vector.
  std::vector<gp_XYZ> pointsVec;
  //
  for ( int k = 0; k < points->GetNumberOfElements(); ++k )
    pointsVec.push_back( points->GetElement(k) );

  return this->internalBuild(pointsVec, result);
}

//-----------------------------------------------------------------------------

bool asiAlgo_PlaneOnPoints::internalBuild(const std::vector<gp_XYZ>& points,
                                          gp_Pln&                    result) const
{
  const int nPts = (int) points.size();

  /* ======================
   *  Calculate mean point
   * ====================== */

  gp_XYZ mu;
  for ( size_t i = 0; i < points.size(); ++i )
  {
    mu += points[i];
  }
  mu /= nPts;

  /* =========================
   *  Build covariance matrix
   * ========================= */

  Eigen::Matrix3d C;
  for ( int j = 1; j <= 3; ++j )
  {
    for ( int k = 1; k <= 3; ++k )
    {
      C(j-1, k-1) = 0.0; // TODO: is that necessary?
    }
  }

  for ( size_t i = 0; i < points.size(); ++i )
  {
    const gp_XYZ& p      = points[i];
    gp_XYZ        p_dash = p - mu;

    for ( int j = 1; j <= 3; ++j )
    {
      for ( int k = 1; k <= 3; ++k )
      {
        C(j-1, k-1) += ( p_dash.Coord(j)*p_dash.Coord(k) );
      }
    }
  }

  for ( int j = 1; j <= 3; ++j )
  {
    for ( int k = 1; k <= 3; ++k )
    {
      C(j-1, k-1) /= nPts;
    }
  }

  Eigen::EigenSolver<Eigen::Matrix3d> EigenSolver(C);

#if defined COUT_DEBUG
  std::cout << "\tCovariance matrix: " << std::endl << C << std::endl;
  std::cout << "\tThe eigen values of C are:" << std::endl << EigenSolver.eigenvalues() << std::endl;
  std::cout << "\tThe matrix of eigenvectors, V, is:" << std::endl << EigenSolver.eigenvectors() << std::endl << std::endl;
#endif

  Eigen::Vector3cd v1 = EigenSolver.eigenvectors().col(0);
  Eigen::Vector3cd v2 = EigenSolver.eigenvectors().col(1);
  Eigen::Vector3cd v3 = EigenSolver.eigenvectors().col(2);

  gp_Vec V[3] = { gp_Vec( v1.x().real(), v1.y().real(), v1.z().real() ),
                  gp_Vec( v2.x().real(), v2.y().real(), v2.z().real() ),
                  gp_Vec( v3.x().real(), v3.y().real(), v3.z().real() ) };
  //
  std::vector< std::pair<double, int> >
    lambda { std::pair<double, int>( EigenSolver.eigenvalues()(0).real(), 0 ),
             std::pair<double, int>( EigenSolver.eigenvalues()(1).real(), 1 ),
             std::pair<double, int>( EigenSolver.eigenvalues()(2).real(), 2 ) };
  //
  std::sort(lambda.begin(), lambda.end(), compare);
  //
  gp_Ax1 ax_X(mu, V[lambda[0].second]);
  gp_Ax1 ax_Y(mu, V[lambda[1].second]);
  gp_Ax1 ax_Z(mu, V[lambda[2].second]);
  //
  gp_Vec vec_X( ax_X.Direction() );
  gp_Vec vec_Y( ax_Y.Direction() );
  gp_Vec vec_Z( ax_Z.Direction() );
  //
  if ( (vec_X ^ vec_Y).Magnitude() < gp::Resolution() ||
       (vec_X ^ vec_Z).Magnitude() < gp::Resolution() ||
       (vec_Y ^ vec_Z).Magnitude() < gp::Resolution() )
  {
    std::cout << "Warning: degenerated normal" << std::endl;
    return false; // Degenerated normal
  }

  // Check if the system is right-handed
  const double ang = ax_X.Direction().AngleWithRef( ax_Y.Direction(), ax_Z.Direction() );
  if ( ang < 0 )
  {
    gp_Ax1 tmp = ax_X;
    ax_X = ax_Y;
    ax_Y = tmp;
  }

  // Store results
  gp_Ax3 ax3( gp_Pnt(mu), ax_Z.Direction(), ax_X.Direction() );
  result.SetPosition(ax3);
  //
  return true;
}
