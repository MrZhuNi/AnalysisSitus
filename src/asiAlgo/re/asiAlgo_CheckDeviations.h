//-----------------------------------------------------------------------------
// Created on: 21 August 2019
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

#ifndef asiAlgo_CheckDeviations_h
#define asiAlgo_CheckDeviations_h

// asiAlgo includes
#include <asiAlgo_BaseCloud.h>
#include <asiAlgo_BVHFacets.h>
#include <asiAlgo_Mesh.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

//-----------------------------------------------------------------------------

//! Utility to check deviations between a CAD part and a point cloud.
class asiAlgo_CheckDeviations : public ActAPI_IAlgorithm
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_CheckDeviations, ActAPI_IAlgorithm)

public:

  //! Ctor.
  //! \param[in] points   point cloud to compare with.
  //! \param[in] progress progress notifier.
  //! \param[in] plotter  imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_CheckDeviations(const Handle(asiAlgo_BaseCloud<double>)& points,
                            ActAPI_ProgressEntry                     progress = nullptr,
                            ActAPI_PlotterEntry                      plotter  = nullptr);

public:

  //! Performs deviation check on CAD model.
  //! \param[in] part CAD part to check.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Perform(const TopoDS_Shape& part);

  //! Performs deviation check on triangulation.
  //! \param[in] tris triangulation to check.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Perform(const Handle(Poly_Triangulation)& tris);

public:

  //! \return result of deviation check which is a faceted representation
  //!         of the CAD part with associated distance field. The scalar
  //!         values representing the distance field are bounded to the
  //!         mesh nodes.
  const asiAlgo_Mesh& GetResult() const
  {
    return m_result;
  }

protected:

  //! Internal method to perform deviation check. Triangulation should
  //! already be prepared.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    internalPerform();

protected:

  Handle(asiAlgo_BaseCloud<double>) m_points;   //!< Point cloud.
  Handle(asiAlgo_BVHFacets)         m_bvh;      //!< BVH for point-to-mesh projection.
  asiAlgo_Mesh                      m_result;   //!< Mesh with field.

};

#endif
