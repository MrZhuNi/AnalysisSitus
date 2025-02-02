//-----------------------------------------------------------------------------
// Created on: 04 December 2016
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

#ifndef asiAlgo_PlaneOnPoints_h
#define asiAlgo_PlaneOnPoints_h

// A-Situs includes
#include <asiAlgo_BaseCloud.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

// OCCT includes
#include <gp_Pln.hxx>

//-----------------------------------------------------------------------------

//! Utility to build a plane on the given point set.
class asiAlgo_PlaneOnPoints : public ActAPI_IAlgorithm
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_PlaneOnPoints, ActAPI_IAlgorithm)

public:

  //! Constructs the tool.
  //! \param[in] progress progress indicator.
  //! \param[in] plotter  imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_PlaneOnPoints(ActAPI_ProgressEntry progress,
                          ActAPI_PlotterEntry  plotter);

public:

  //! Constructs the average plane on the given point set.
  //! \param[in]  points point set to build a fitting plane for.
  //! \param[out] result resulting plane.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Build(const std::vector<gp_XYZ>& points,
          gp_Pln&                    result) const;

  //! Constructs the average plane on the given point set passed in
  //! the form of a point cloud.
  //! \param[in]  points point set to build a fitting plane for.
  //! \param[out] result resulting plane.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Build(const Handle(asiAlgo_BaseCloud<double>)& points,
          gp_Pln&                                  result) const;

protected:

  //! Internal method for constructing an average plane for the given point
  //! set. This method is here to provide different public interfaces to the
  //! same core computation.
  //! \param[in]  points point set to build a fitting plane for.
  //! \param[out] result resulting plane.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    internalBuild(const std::vector<gp_XYZ>& points,
                  gp_Pln&                    result) const;

};

#endif
