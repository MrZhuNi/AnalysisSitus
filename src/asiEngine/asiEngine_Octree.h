//-----------------------------------------------------------------------------
// Created on: 27 February 2020
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

#ifndef asiEngine_Octree_h
#define asiEngine_Octree_h

// asiEngine includes
#include <asiEngine_Base.h>

// asiData includes
#include <asiData_OctreeNode.h>

//-----------------------------------------------------------------------------

//! Data Model API for octrees.
class asiEngine_Octree : public asiEngine_Base
{
public:

  //! Ctor.
  //! \param[in] model    Data Model instance.
  //! \param[in] progress progress notifier.
  //! \param[in] plotter  imperative plotter.
  asiEngine_Octree(const Handle(asiEngine_Model)& model,
                   ActAPI_ProgressEntry           progress = nullptr,
                   ActAPI_PlotterEntry            plotter  = nullptr)
  //
  : asiEngine_Base(model, progress, plotter)
  {}

public:

  //! Creates octree under the passed owner Node.
  //! \param[in] owner     owner Node.
  //! \param[in] operation CSG operation to set.
  //! \param[in] left      left operand for a Boolean function.
  //! \param[in] right     right operand for a Boolean function.
  //! \return newly created Octree Node.
  asiEngine_EXPORT Handle(asiData_OctreeNode)
    CreateOctree(const Handle(ActAPI_INode)&       owner,
                 const asiAlgo_CSG                 operation,
                 const Handle(asiData_OctreeNode)& left  = nullptr,
                 const Handle(asiData_OctreeNode)& right = nullptr);

};

#endif
