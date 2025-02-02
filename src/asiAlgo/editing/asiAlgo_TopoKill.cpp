//-----------------------------------------------------------------------------
// Created on: 05 September 2017
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
#include <asiAlgo_TopoKill.h>

// OCCT includes
#include <BRep_Builder.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Iterator.hxx>

//-----------------------------------------------------------------------------

asiAlgo_TopoKill::asiAlgo_TopoKill(const TopoDS_Shape&  masterCAD,
                                   ActAPI_ProgressEntry progress,
                                   ActAPI_PlotterEntry  plotter)
: ActAPI_IAlgorithm (progress, plotter),
  m_master          (masterCAD),
  m_bErrorState     (false)
{}

//-----------------------------------------------------------------------------

bool asiAlgo_TopoKill::AskRemove(const TopoDS_Shape& subshape)
{
  if ( m_toReplace.IsBound(subshape) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Replacement is already asked for the shape you want to remove.");
    return false;
  }

  m_toRemove.Add(subshape);
  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_TopoKill::AskReplace(const TopoDS_Shape& what,
                                  const TopoDS_Shape& with)
{
  // Check replacement type.
  if ( what.ShapeType() != with.ShapeType() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Cannot kill by replacement with different type.");
    return false;
  }

  // Check if any replacement is already recorded for the target sub-shape.
  if ( m_toReplace.IsBound(what) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Such replacement is already asked.");
    return false;
  }

  // Check if removal is already recorded for the target sub-shape.
  if ( m_toRemove.Contains(what) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Removal is already asked for the shape you want to replace.");
    return false;
  }

  m_toReplace.Bind(what, with);
  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_TopoKill::IsAsked(const TopoDS_Shape& subshape,
                               const bool          checkRecursively,
                               bool&               forRemoval,
                               bool&               forReplacement) const
{
  if ( checkRecursively && (forRemoval || forReplacement) )
    return true; // Stop recursion.

  // Check the shape itself (without recursion).
  if ( this->IsAsked(subshape, forRemoval, forReplacement) )
    return true;

  // If the shape itself is not asked and the check is recursive, then
  // apply the same check to the direct children of the shape in question.
  if ( checkRecursively )
  {
    for ( TopoDS_Iterator it(subshape, false, true); it.More(); it.Next() )
    {
      const TopoDS_Shape& currentShape = it.Value();

      if ( this->IsAsked(currentShape, true, forRemoval, forReplacement) )
        return true;
    }
  }

  return false;
}

//-----------------------------------------------------------------------------

bool asiAlgo_TopoKill::IsAsked(const TopoDS_Shape& subshape,
                               bool&               forRemoval,
                               bool&               forReplacement) const
{
  forRemoval     = m_toRemove.Contains(subshape);
  forReplacement = m_toReplace.IsBound(subshape);

  return forRemoval || forReplacement;
}

//-----------------------------------------------------------------------------

bool asiAlgo_TopoKill::Apply()
{
  if ( m_history.IsNull() )
    m_history = new asiAlgo_History; // Construct history if not externally defined.

  bool forRemoval = false, forReplacement = false;
  //
  if ( !this->IsAsked(m_master, true, forRemoval, forReplacement) )
  {
    m_progress.SendLogMessage(LogWarn(Normal) << "No sub-shapes are requested for modification.");
    return false;
  }

  // Prepare root of the same type as a master shape.
  m_result = m_master.EmptyCopied();
  //
  m_history->AddModified(m_master, m_result);

  // Rebuild topology graph recursively.
  this->buildTopoGraphLevel(m_master, m_result);

  return !this->IsErrorState(); // Some error may have occurred in recursion.
}

//-----------------------------------------------------------------------------

void asiAlgo_TopoKill::buildTopoGraphLevel(const TopoDS_Shape& root,
                                           TopoDS_Shape&       result)
{
  if ( this->IsErrorState() )
    return;

  BRep_Builder BB;

  // The "false" flag passed to the iterator below means that we do not
  // want to accumulate orientations of sub-shapes. These orientations we
  // manage ourselves attempting to keep them untouched. At the same time,
  // we do accumulate locations as otherwise we lose local placement of
  // the part.
  for ( TopoDS_Iterator it(root, false, true); it.More(); it.Next() )
  {
    const TopoDS_Shape& currentShape = it.Value();
    TopoDS_Shape newResult;

    // This flag indicates whether the twin element for the current entity
    // has been already built. If so, we only need to link the newly
    // built parent with the already existing (also newly built but in
    // a different branch of recursion) child, not reconstructing the child
    // iteself.
    bool isAlreadyBuilt = false;

    // Check recursively if the current sub-shape is asked for modification.
    // The modification can be asked for the shape itself or for any of its
    // sub-shapes at any nesting level. Therefore, we perform a recursive
    // check here. It allows to avoid creating new shapes (TShape objects)
    // for those entities which were not touched.
    bool forRemoval = false, forReplacement = false, excludeSubshape = false;
    //
    if ( !this->IsAsked(currentShape, true, forRemoval, forReplacement) )
    {
      newResult = currentShape; // Just take the shape itself as a new shape.

      // NOTICE: the recursion is stopped here because the assignment above
      //         takes effect for sub-shapes also. This assignment actually
      //         means that at a certain level of topology graph for the
      //         resulting shape, a pointer to a piece of old topology graph
      //         will be used.
    }
    else
    {
      // Now we need to know if the shape itself was asked for modification.
      // If not, and if we are here, it means that one of the sub-shapes was
      // affected, so we just go deeper in the topology graph. If the sub-shape
      // itself was modified, then we need to know what actually happened.
      if ( this->IsAsked(currentShape, forRemoval, forReplacement) )
      {
        if ( forRemoval )
        {
          excludeSubshape = true;
          //
          m_history->SetDeleted(currentShape);
        }
        else // Replacement
        {
          newResult = m_toReplace(currentShape);

          // If no orientation is passed, it means that the orientation
          // should be defined in-context.
          if ( newResult.Orientation() == TopAbs_EXTERNAL )
          {
            TopAbs_Orientation newOri;
            //
            if ( currentShape.ShapeType() == TopAbs_EDGE )
            {
              // For edges, we should be careful with orientations
              if ( !this->chooseOri(TopoDS::Edge(currentShape),
                                    TopoDS::Edge(newResult),
                                    newOri) )
              {
                this->SetErrorStateOn();
                return;
              }
            }
            else
              newOri = currentShape.Orientation();

            // Set orientation.
            newResult.Orientation(newOri);
          }

          // Set history record.
          m_history->AddModified(currentShape, newResult);
        }
      }
      else // Shape itself is not touched, but some children are
      {
        if ( !m_newElements.IsBound(currentShape) )
        {
          // If any sub-shape or the shape itself was touched, then we have
          // to construct another shape. That's because in OpenCascade it is
          // impossible to modify the existing shape handler.
          newResult = currentShape.EmptyCopied();

          // Bind element for reuse. Otherwise, adjacent elements (e.g. edges
          // sharing a vertex being replaced) will be replicated.
          m_newElements.Bind(currentShape, newResult);
        }
        else
        {
          newResult = m_newElements(currentShape); // Take the already created one.

          // Apply right orientation.
          newResult.Orientation( currentShape.Orientation() );

          // Turn off a flag to prevent recursion on this element.
          isAlreadyBuilt = true;
        }

        // Set history record.
        m_history->AddModified(currentShape, newResult);
      }

      // Proceed recursively.
      if ( !isAlreadyBuilt && !forRemoval && !forReplacement )
        this->buildTopoGraphLevel(currentShape, newResult);
    }

    if ( !excludeSubshape )
    {
      // Have a look at the following shitty code. We have to reverse
      // orientation a sub-shape, because TopoDS_Builder will reverse all
      // sub-shapes of a REVERSED parent. To avoid this effect, we corrupt
      // out shape here, in order to let TopoDS_Builder corrupt it again...
      //
      // For more, see issue #29264 in OpenCascade bugtracker.
      if ( result.Orientation() == TopAbs_REVERSED )
        newResult.Reverse();

      BB.Add(result, newResult);
    }
  }
}

//-----------------------------------------------------------------------------

bool asiAlgo_TopoKill::chooseOri(const TopoDS_Edge&  oldEdge,
                                 const TopoDS_Edge&  newEdge,
                                 TopAbs_Orientation& ori) const
{
  // First, we check if the two edges are geometrically oriented in the
  // same way. We have to check this on 3D curves because the new edge may
  // have no parametric image on the host face.
  double fo, lo, fn, ln;
  //
  Handle(Geom_Curve) oldCurve = BRep_Tool::Curve(oldEdge, fo, lo);
  Handle(Geom_Curve) newCurve = BRep_Tool::Curve(newEdge, fn, ln);
  //
  if ( oldCurve.IsNull() || newCurve.IsNull() )
    return false;

  // Check natural orientation at start point
  // ...

  gp_Pnt oldCurveP;
  gp_Vec oldCurveV1;
  oldCurve->D1(fo, oldCurveP, oldCurveV1);

  gp_Pnt newCurveP;
  gp_Vec newCurveV1;
  newCurve->D1(fn, newCurveP, newCurveV1);

  bool isReversedGeometrically;
  if ( Abs( oldCurveV1.Angle(newCurveV1) ) > Abs( oldCurveV1.Angle( newCurveV1.Reversed() ) ) )
    isReversedGeometrically = true;
  else
    isReversedGeometrically = false;

  // Agree or not with the as-built orientation topologically.
  ori = oldEdge.Orientation();
  //
  if ( isReversedGeometrically )
    ori = (ori == TopAbs_FORWARD ? TopAbs_REVERSED : TopAbs_FORWARD); // Invert.

  return true;
}
