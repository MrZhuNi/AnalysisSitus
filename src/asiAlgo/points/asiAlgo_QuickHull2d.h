//-----------------------------------------------------------------------------
// Created on: 15 April 2016
//-----------------------------------------------------------------------------
// Copyright (c) 2016-present, Sergey Slyadnev
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

#ifndef asiAlgo_QuickHull2d_h
#define asiAlgo_QuickHull2d_h

// asiAlgo includes
#include <asiAlgo_PointWithAttr.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

// OCCT includes
#include <TColgp_HSequenceOfXY.hxx>
#include <TColStd_HSequenceOfInteger.hxx>

//-----------------------------------------------------------------------------

//! Algorithm constructing convex hull of two-dimensional point cloud
//! by Quick Hull (divide and conquer) method.
template <typename TPoint>
class asiAlgo_QuickHull2d : public ActAPI_IAlgorithm
{
public:

  //! Constructs the tool initializing it with the passed two-dimensional
  //! point cloud.
  //! \param[in] cloud    initial cloud.
  //! \param[in] progress progress notifier.
  //! \param[in] plotter  imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_QuickHull2d(const Handle(asiAlgo_PointWithAttrCloud<TPoint>)& cloud,
                        ActAPI_ProgressEntry                              progress,
                        ActAPI_PlotterEntry                               plotter);

  //! Destructor.
  asiAlgo_EXPORT
    ~asiAlgo_QuickHull2d();

public:

  //! Constructs convex hull from the initial cloud.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Perform();

public:

  //! Returns initial point cloud.
  //! \return initial point cloud.
  const Handle(asiAlgo_PointWithAttrCloud<TPoint>)& GetInputPoints() const
  {
    return m_cloud;
  }

  //! Returns resulting convex hull as indices.
  //! \return resulting convex hull.
  const Handle(TColStd_HSequenceOfInteger)& GetHull() const
  {
    return m_hull;
  }

private:

  //! Node of traversal hierarchy.
  struct PNode
  {
    int    idx;    //!< 0-based index of the corresponding point.
    PNode* left;   //!< Left sub-tree.
    PNode* right;  //!< Right sub-tree.
    PNode* parent; //!< Pointer to parent node.

    //! Default constructor.
    PNode() : idx(-1), left(nullptr), right(nullptr), parent(nullptr) {}

    //! Constructor accepting point index.
    //! \param index [in] 0-based point index.
    PNode(const int index) : idx(index), left(nullptr), right(nullptr), parent(nullptr) {}

    //! Constructor accepting point index and parent node.
    //! \param index [in] 0-based point index.
    //! \param node  [in] parent node.
    PNode(const int index,
          PNode* node) : idx(index), left(nullptr), right(nullptr), parent(node) {}
  };

private:

  //! Finds the point with the maximum distance against P1-P2.
  //! \param p1_idx [in] 0-based index of P1.
  //! \param p2_idx [in] 0-based index of P2.
  //! \param up     [in] indicates whether to look-up for a point above (true)
  //!                    or below (false) the line.
  //! \return index of the found point or -1 if nothing has been found.
  int findDistantPoint(const int  p1_idx,
                       const int  p2_idx,
                       const bool up) const;

  //! Recursive processing.
  //! \param prev [in] previous node.
  //! \param curr [in] current node.
  //! \param left [in] indicates whether to look-up for a point on the left
  //!                  (true) or on the right (false) regarding to the current point.
  //! \param up   [in] indicates whether to look-up for a point above (true)
  //!                  or below (false) the line.
  void process(PNode*     prev,
               PNode*     curr,
               const bool left,
               const bool up);

  //! Recursive iteration of tree by triplets, where {parent} can possess
  //! {left} and {right} children. The traversing order is
  //! {left} -> {parent} -> {right}.
  //!
  //! \param parent [in] parent node to start from.
  //! \param ccw    [in] indicates whether to iterate in counter-clockwise
  //!                    order (true) or not (false).
  void traverseTriplet(PNode*     parent,
                       const bool ccw);

  //! Releases memory occupied for the passed node.
  //! \param node [in/out] pointer to node to free heap memory for.
  void releaseNode(PNode* node);

  //! Adds the passed node to the resulting collection.
  //! \param node [in] node to add.
  void addNode(PNode* node);

private:

  Handle(asiAlgo_PointWithAttrCloud<TPoint>) m_cloud; //!< Initial cloud.
  Handle(TColStd_HSequenceOfInteger)         m_hull;  //!< Resulting hull.

private:

  PNode* m_P_up;    //!< Sentinel node on the top of the cloud.
  PNode* m_P_down;  //!< Sentinel node on the bottom of the cloud.
  PNode* m_P_left;  //!< Sentinel node on the left of the cloud.
  PNode* m_P_right; //!< Sentinel node on the right of the cloud.

};

#endif
