//-----------------------------------------------------------------------------
// Created on: 11 April 2016
//-----------------------------------------------------------------------------
// Copyright (c) 2017 Sergey Slyadnev
// Code covered by the MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//-----------------------------------------------------------------------------

// Own include
#include <asiEngine_IV.h>

// asiEngine includes
#include <asiEngine_Model.h>

// asiAlgo includes
#include <asiAlgo_MeshConvert.h>

// Active Data includes
#include <ActData_UniqueNodeName.h>

// OCCT includes
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Precision.hxx>

//-----------------------------------------------------------------------------

//! \return newly created IV Node.
Handle(asiData_IVNode) asiEngine_IV::Create_IV()
{
  // Add IV Node to Partition
  Handle(asiData_IVNode) iv_n = Handle(asiData_IVNode)::DownCast( asiData_IVNode::Instance() );
  m_model->GetIVPartition()->AddNode(iv_n);

  // Initialize Node
  iv_n->Init();
  iv_n->SetName("Imperative Viewer");

  // Create underlying Points 2D
  {
    Handle(ActAPI_INode) iv_points_2d_base = asiData_IVPoints2dNode::Instance();
    m_model->GetIVPoints2dPartition()->AddNode(iv_points_2d_base);

    // Initialize
    Handle(asiData_IVPoints2dNode) iv_points_2d_n = Handle(asiData_IVPoints2dNode)::DownCast(iv_points_2d_base);
    iv_points_2d_n->Init();
    iv_points_2d_n->SetName("Points 2D");

    // Add as child
    iv_n->AddChildNode(iv_points_2d_n);
  }

  // Create underlying Points
  {
    Handle(ActAPI_INode) iv_points_base = asiData_IVPointsNode::Instance();
    m_model->GetIVPointsPartition()->AddNode(iv_points_base);

    // Initialize
    Handle(asiData_IVPointsNode) iv_points_n = Handle(asiData_IVPointsNode)::DownCast(iv_points_base);
    iv_points_n->Init();
    iv_points_n->SetName("Points");

    // Add as child
    iv_n->AddChildNode(iv_points_n);
  }

  // Create underlying Curves
  {
    Handle(ActAPI_INode) iv_curves_base = asiData_IVCurvesNode::Instance();
    m_model->GetIVCurvesPartition()->AddNode(iv_curves_base);

    // Initialize
    Handle(asiData_IVCurvesNode) iv_curves_n = Handle(asiData_IVCurvesNode)::DownCast(iv_curves_base);
    iv_curves_n->Init();
    iv_curves_n->SetName("Curves");

    // Add as child
    iv_n->AddChildNode(iv_curves_n);
  }

  // Create underlying Surfaces
  {
    Handle(ActAPI_INode) iv_surfaces_base = asiData_IVSurfacesNode::Instance();
    m_model->GetIVSurfacesPartition()->AddNode(iv_surfaces_base);

    // Initialize
    Handle(asiData_IVSurfacesNode) iv_surfaces_n = Handle(asiData_IVSurfacesNode)::DownCast(iv_surfaces_base);
    iv_surfaces_n->Init();
    iv_surfaces_n->SetName("Surfaces");

    // Add as child
    iv_n->AddChildNode(iv_surfaces_n);
  }

  // Create underlying Topology container
  {
    Handle(ActAPI_INode) iv_topo_base = asiData_IVTopoNode::Instance();
    m_model->GetIVTopoPartition()->AddNode(iv_topo_base);

    // Initialize
    Handle(asiData_IVTopoNode) iv_topo_n = Handle(asiData_IVTopoNode)::DownCast(iv_topo_base);
    iv_topo_n->Init();
    iv_topo_n->SetName("Topology");

    // Add as child
    iv_n->AddChildNode(iv_topo_n);
  }

  // Create underlying Tessellation container
  {
    Handle(ActAPI_INode) iv_tess_base = asiData_IVTessNode::Instance();
    m_model->GetIVTessPartition()->AddNode(iv_tess_base);

    // Initialize
    Handle(asiData_IVTessNode) iv_tess_n = Handle(asiData_IVTessNode)::DownCast(iv_tess_base);
    iv_tess_n->Init();
    iv_tess_n->SetName("Tessellation");

    // Add as child
    iv_n->AddChildNode(iv_tess_n);
  }

  // Create underlying Text container
  {
    Handle(ActAPI_INode) iv_text_base = asiData_IVTextNode::Instance();
    m_model->GetIVTextPartition()->AddNode(iv_text_base);

    // Initialize
    Handle(asiData_IVTextNode) iv_text_n = Handle(asiData_IVTextNode)::DownCast(iv_text_base);
    iv_text_n->Init();
    iv_text_n->SetName("Text");

    // Add as child
    iv_n->AddChildNode(iv_text_n);
  }

  // Return the just created Node
  return iv_n;
}

//-----------------------------------------------------------------------------

//! Cleans all data objects related to imperative plotter.
void asiEngine_IV::Clean_All()
{
  Clean_Curves();
  Clean_Points();
  Clean_Points2d();
  Clean_Surfaces();
  Clean_Tess();
  Clean_Text();
  Clean_Topo();
}

//-----------------------------------------------------------------------------

//! Cleans objects by name. If several objects with the given name are found,
//! all them are deleted. The search is done in a tree branch rooted by
//! Imperative Viewer Node.
//!
//! \param name [in] name of the target object.
void asiEngine_IV::Clean(const TCollection_AsciiString& name)
{
  Handle(ActAPI_HNodeList) nodesToDelete = new ActAPI_HNodeList;

  // Find all Nodes with the given name
  for ( Handle(ActAPI_IChildIterator) cit = m_model->GetIVNode()->GetChildIterator(true); cit->More(); cit->Next() )
  {
    Handle(ActAPI_INode) node = cit->Value();
    //
    if ( node.IsNull() || !node->IsWellFormed() )
      continue;

    TCollection_ExtendedString nodeName = node->GetName();
    //
    if ( nodeName.IsEqual(name) )
      nodesToDelete->Append(node);
  }

  // Delete all Nodes queued for removal
  for ( ActAPI_NodeList::Iterator nit( *nodesToDelete.operator->() ); nit.More(); nit.Next() )
    m_model->DeleteNode( nit.Value()->GetId() );
}

//-----------------------------------------------------------------------------

//! Finds Node with the given name. Returns null if nothing is found.
//! \param name [in] target name.
//! \return found Node (the first one if several exist) or null.
Handle(asiData_IVPointSet2dNode)
  asiEngine_IV::Find_PointSet2d(const TCollection_AsciiString& name)
{
  // Find the first Node with the given name
  for ( Handle(ActAPI_IChildIterator) cit = m_model->GetIVNode()->Points2d()->GetChildIterator(true); cit->More(); cit->Next() )
  {
    Handle(ActAPI_INode) node = cit->Value();
    //
    if ( node.IsNull() || !node->IsWellFormed() )
      continue;

    TCollection_ExtendedString nodeName = node->GetName();
    //
    if ( nodeName.IsEqual(name) )
      return Handle(asiData_IVPointSet2dNode)::DownCast(node);
  }

  return NULL;
}

//-----------------------------------------------------------------------------

//! Creates Point Set 2D Node.
//! \param coords        [in] target point cloud.
//! \param name          [in] name to set (auto-generated if empty).
//! \param useAutoNaming [in] indicates whether to auto-name entities.
//! \return Point Set 2D Node.
Handle(asiData_IVPointSet2dNode)
  asiEngine_IV::Create_PointSet2d(const Handle(TColStd_HArray1OfReal)& coords,
                                  const TCollection_AsciiString&       name,
                                  const bool                           useAutoNaming)
{
  // Access Model and parent Node
  Handle(asiData_IVPoints2dNode) IV_Parent = m_model->GetIVNode()->Points2d();

  // Add Point Set Node to Partition
  Handle(asiData_IVPointSet2dNode) item_n = Handle(asiData_IVPointSet2dNode)::DownCast( asiData_IVPointSet2dNode::Instance() );
  m_model->GetIVPointSet2dPartition()->AddNode(item_n);

  // Generate unique name
  TCollection_ExtendedString item_name = ( name.IsEmpty() ? "Point Set 2D" : name );
  if ( useAutoNaming )
    item_name = ActData_UniqueNodeName::Generate(ActData_SiblingNodes::CreateForChild(item_n, IV_Parent), item_name);

  // Initialize
  item_n->Init();
  item_n->SetName(item_name);
  item_n->SetPoints(coords);

  // Add as child
  IV_Parent->AddChildNode(item_n);

  // Return the just created Node
  return item_n;
}

//-----------------------------------------------------------------------------

//! Creates a two-dimensional point set from a vector of gp_Pnt2d objects.
//! \param points        [in] vector to store.
//! \param name          [in] desired object name.
//! \param useAutoNaming [in] indicates whether to auto-name entities.
Handle(asiData_IVPointSet2dNode)
  asiEngine_IV::Create_PointSet2d(const std::vector<gp_Pnt2d>&   points,
                                  const TCollection_AsciiString& name,
                                  const bool                     useAutoNaming)
{
  Handle(TColStd_HArray1OfReal) hcoords = new TColStd_HArray1OfReal(0, (int) points.size()*2 - 1);
  //
  for ( int i = 0; i < (int) points.size(); ++i )
  {
    hcoords->ChangeValue(i*2 + 0) = points[i].X();
    hcoords->ChangeValue(i*2 + 1) = points[i].Y();
  }

  // Create from coordinates
  return Create_PointSet2d(hcoords, name, useAutoNaming);
}

//-----------------------------------------------------------------------------

//! Deletes all Point Set 2D Nodes.
void asiEngine_IV::Clean_Points2d()
{
  Handle(asiData_IVPoints2dNode)
    IV_Parent = m_model->GetIVNode()->Points2d();
  //
  _cleanChildren(IV_Parent);
}

//-----------------------------------------------------------------------------

//! Finds Node with the given name. Returns null if nothing is found.
//! \param name [in] target name.
//! \return found Node (the first one if several exist) or null.
Handle(asiData_IVPointSetNode)
  asiEngine_IV::Find_PointSet(const TCollection_AsciiString& name)
{
  // Find the first Node with the given name
  for ( Handle(ActAPI_IChildIterator) cit = m_model->GetIVNode()->Points()->GetChildIterator(true); cit->More(); cit->Next() )
  {
    Handle(ActAPI_INode) node = cit->Value();
    //
    if ( node.IsNull() || !node->IsWellFormed() )
      continue;

    TCollection_ExtendedString nodeName = node->GetName();
    //
    if ( nodeName.IsEqual(name) )
      return Handle(asiData_IVPointSetNode)::DownCast(node);
  }

  return NULL;
}

//-----------------------------------------------------------------------------

//! Creates Point Set Node.
//! \param points        [in] target point cloud.
//! \param name          [in] name to set (auto-generated if empty).
//! \param useAutoNaming [in] indicates whether to auto-name entities.
//! \return Point Set Node.
Handle(asiData_IVPointSetNode)
  asiEngine_IV::Create_PointSet(const Handle(asiAlgo_BaseCloud<double>)& points,
                                const TCollection_AsciiString&           name,
                                const bool                               useAutoNaming)
{
  // Access Model and parent Node
  Handle(asiData_IVPointsNode) IV_Parent = m_model->GetIVNode()->Points();

  // Add Point Set Node to Partition
  Handle(asiData_IVPointSetNode) item_n = Handle(asiData_IVPointSetNode)::DownCast( asiData_IVPointSetNode::Instance() );
  m_model->GetIVPointSetPartition()->AddNode(item_n);

  // Generate unique name
  TCollection_ExtendedString item_name = ( name.IsEmpty() ? "Point Set" : name );
  if ( useAutoNaming )
    item_name = ActData_UniqueNodeName::Generate(ActData_SiblingNodes::CreateForChild(item_n, IV_Parent), item_name);

  // Initialize
  item_n->Init();
  item_n->SetName(item_name);
  item_n->SetPoints(points);

  // Add as child
  IV_Parent->AddChildNode(item_n);

  // Return the just created Node
  return item_n;
}

//-----------------------------------------------------------------------------

//! Deletes all Point Set Nodes.
void asiEngine_IV::Clean_Points()
{
  Handle(asiData_IVPointsNode)
    IV_Parent = m_model->GetIVNode()->Points();
  //
  _cleanChildren(IV_Parent);
}

//-----------------------------------------------------------------------------

//! Finds Node with the given name. Returns null if nothing is found.
//! \param name [in] target name.
//! \return found Node (the first one if several exist) or null.
Handle(asiData_IVCurveNode)
  asiEngine_IV::Find_Curve(const TCollection_AsciiString& name)
{
  // Find the first Node with the given name
  for ( Handle(ActAPI_IChildIterator) cit = m_model->GetIVNode()->Curves()->GetChildIterator(true); cit->More(); cit->Next() )
  {
    Handle(ActAPI_INode) node = cit->Value();
    //
    if ( node.IsNull() || !node->IsWellFormed() )
      continue;

    TCollection_ExtendedString nodeName = node->GetName();
    //
    if ( nodeName.IsEqual(name) )
      return Handle(asiData_IVCurveNode)::DownCast(node);
  }

  return NULL;
}

//-----------------------------------------------------------------------------

//! Creates Curve Node.
//! \param curve         [in] parametric curve to store.
//! \param uLimit        [in] absolute value to bound infinite parametric domain.
//! \param name          [in] name to set (auto-generated if empty).
//! \param useAutoNaming [in] indicates whether to auto-name entities.
//! \return newly created Node.
Handle(asiData_IVCurveNode)
  asiEngine_IV::Create_Curve(const Handle(Geom_Curve)&      curve,
                             const double                   uLimit,
                             const TCollection_AsciiString& name,
                             const bool                     useAutoNaming)
{
  if ( curve.IsNull() )
    return NULL;

  // Access Model and parent Node
  Handle(asiData_IVCurvesNode) IV_Parent = m_model->GetIVNode()->Curves();

  // Add Curve Node to Partition
  Handle(asiData_IVCurveNode) item_n = Handle(asiData_IVCurveNode)::DownCast( asiData_IVCurveNode::Instance() );
  m_model->GetIVCurvePartition()->AddNode(item_n);

  // Generate unique name
  TCollection_ExtendedString item_name = ( name.IsEmpty() ? "Curve" : name );
  if ( useAutoNaming )
    item_name = ActData_UniqueNodeName::Generate(ActData_SiblingNodes::CreateForChild(item_n, IV_Parent), item_name);
  //
  item_n->SetName(item_name);

  // Initialize Node
  Update_Curve(item_n, curve, uLimit);

  // Add as child
  IV_Parent->AddChildNode(item_n);

  // Return the just created Node
  return item_n;
}

//-----------------------------------------------------------------------------

//! Updates Curve Node with the passed curve data.
//! \param node   [in] Curve Node to update.
//! \param curve  [in] parametric curve to store.
//! \param uLimit [in] absolute value to bound infinite parametric domain.
void asiEngine_IV::Update_Curve(const Handle(asiData_IVCurveNode)& node,
                                const Handle(Geom_Curve)&          curve,
                                const double                       uLimit)
{
  // Handle infinite domains
  double             f      = curve->FirstParameter();
  double             l      = curve->LastParameter();
  bool               toTrim = false;
  Handle(Geom_Curve) geometry;
  //
  if ( Precision::IsInfinite(f) )
  {
    f      = -uLimit;
    toTrim = true;
  }
  if ( Precision::IsInfinite(l) )
  {
    l = uLimit;
    if ( !toTrim ) toTrim = true;
  }
  //
  if ( toTrim )
    geometry = new Geom_TrimmedCurve(curve, f, l);
  else
    geometry = curve;

  // Initialize
  node->Init();
  node->SetCurve(geometry, f, l);
}

//-----------------------------------------------------------------------------

//! Deletes all Curve Nodes.
void asiEngine_IV::Clean_Curves()
{
  Handle(asiData_IVCurvesNode)
    IV_Parent = m_model->GetIVNode()->Curves();
  //
  _cleanChildren(IV_Parent);
}

//-----------------------------------------------------------------------------

//! Finds Node with the given name. Returns null if nothing is found.
//! \param name [in] target name.
//! \return found Node (the first one if several exist) or null.
Handle(asiData_IVSurfaceNode)
  asiEngine_IV::Find_Surface(const TCollection_AsciiString& name)
{
  // Find the first Node with the given name
  for ( Handle(ActAPI_IChildIterator) cit = m_model->GetIVNode()->Surfaces()->GetChildIterator(true); cit->More(); cit->Next() )
  {
    Handle(ActAPI_INode) node = cit->Value();
    //
    if ( node.IsNull() || !node->IsWellFormed() )
      continue;

    TCollection_ExtendedString nodeName = node->GetName();
    //
    if ( nodeName.IsEqual(name) )
      return Handle(asiData_IVSurfaceNode)::DownCast(node);
  }

  return NULL;
}

//-----------------------------------------------------------------------------

//! Creates Surface Node.
//! \param surface       [in] parametric surface to store.
//! \param uLimit        [in] absolute value to bound infinite parametric domain in U.
//! \param vLimit        [in] absolute value to bound infinite parametric domain in V.
//! \param name          [in] name to set (auto-generated if empty).
//! \param useAutoNaming [in] indicates whether to auto-name entities.
//! \return newly created Node.
Handle(asiData_IVSurfaceNode)
  asiEngine_IV::Create_Surface(const Handle(Geom_Surface)&    surface,
                               const double                   uLimit,
                               const double                   vLimit,
                               const TCollection_AsciiString& name,
                               const bool                     useAutoNaming)
{
  if ( surface.IsNull() )
    return NULL;

  // Access Model and parent Node
  Handle(asiData_IVSurfacesNode) IV_Parent = m_model->GetIVNode()->Surfaces();

  // Add Surface Node to Partition
  Handle(asiData_IVSurfaceNode) item_n = Handle(asiData_IVSurfaceNode)::DownCast( asiData_IVSurfaceNode::Instance() );
  m_model->GetIVSurfacePartition()->AddNode(item_n);

  // Generate unique name if a good name is not passed
  TCollection_ExtendedString item_name = ( name.IsEmpty() ? "Surface" : name );
  if ( useAutoNaming )
    item_name = ActData_UniqueNodeName::Generate(ActData_SiblingNodes::CreateForChild(item_n, IV_Parent), item_name);
  //
  item_n->SetName(item_name);

  // Initialize
  Update_Surface(item_n, surface, uLimit, vLimit);

  // Add as child
  IV_Parent->AddChildNode(item_n);

  // Return the just created Node
  return item_n;
}

//-----------------------------------------------------------------------------

//! Updates Surface Node.
//! \param node    [in] Data Node to update.
//! \param surface [in] parametric surface to store.
//! \param uLimit  [in] absolute value to bound infinite parametric domain in U.
//! \param vLimit  [in] absolute value to bound infinite parametric domain in V.
void asiEngine_IV::Update_Surface(const Handle(asiData_IVSurfaceNode)& node,
                                  const Handle(Geom_Surface)&          surface,
                                  const double                         uLimit,
                                  const double                         vLimit)
{
  // Initialize
  node->Init();
  node->SetSurface(surface);
  node->SetLimits(uLimit, vLimit);
}

//-----------------------------------------------------------------------------

//! Deletes all Surface Nodes.
void asiEngine_IV::Clean_Surfaces()
{
  Handle(asiData_IVSurfacesNode)
    IV_Parent = m_model->GetIVNode()->Surfaces();
  //
  _cleanChildren(IV_Parent);
}

//-----------------------------------------------------------------------------

//! Finds Node with the given name. Returns null if nothing is found.
//! \param name [in] target name.
//! \return found Node (the first one if several exist) or null.
Handle(asiData_IVTopoItemNode)
  asiEngine_IV::Find_TopoItem(const TCollection_AsciiString& name)
{
  // Find the first Node with the given name
  for ( Handle(ActAPI_IChildIterator) cit = m_model->GetIVNode()->Topology()->GetChildIterator(true); cit->More(); cit->Next() )
  {
    Handle(ActAPI_INode) node = cit->Value();
    //
    if ( node.IsNull() || !node->IsWellFormed() )
      continue;

    TCollection_ExtendedString nodeName = node->GetName();
    //
    if ( nodeName.IsEqual(name) )
      return Handle(asiData_IVTopoItemNode)::DownCast(node);
  }

  return NULL;
}

//-----------------------------------------------------------------------------

//! Creates Topological Item Node.
//! \param shape         [in] shape to store.
//! \param name          [in] name to set (auto-generated if empty).
//! \param useAutoNaming [in] indicates whether to auto-name entities.
//! \return newly created Node.
Handle(asiData_IVTopoItemNode)
  asiEngine_IV::Create_TopoItem(const TopoDS_Shape&            shape,
                                const TCollection_AsciiString& name,
                                const bool                     useAutoNaming)
{
  // Access Model and parent Node
  Handle(asiData_IVTopoNode) IV_Parent = m_model->GetIVNode()->Topology();

  // Add Topological Item Node to Partition
  Handle(asiData_IVTopoItemNode) item_n = Handle(asiData_IVTopoItemNode)::DownCast( asiData_IVTopoItemNode::Instance() );
  m_model->GetIVTopoItemPartition()->AddNode(item_n);

  // Generate unique name
  TCollection_ExtendedString item_name = ( name.IsEmpty() ? "Shape" : name );
  if ( useAutoNaming )
    item_name = ActData_UniqueNodeName::Generate(ActData_SiblingNodes::CreateForChild(item_n, IV_Parent), item_name);
  //
  item_n->SetName(item_name);

  // Initialize
  Update_TopoItem(item_n, shape);

  // Add as child
  IV_Parent->AddChildNode(item_n);

  // Return the just created Node
  return item_n;
}

//-----------------------------------------------------------------------------

//! Updates Topo Item Node.
//! \param node  [in] Data Node to update.
//! \param shape [in] B-Rep shape to store.
void asiEngine_IV::Update_TopoItem(const Handle(asiData_IVTopoItemNode)& node,
                                   const TopoDS_Shape&                   shape)
{
  // Initialize
  node->Init();
  node->SetShape(shape);
}

//-----------------------------------------------------------------------------

//! Deletes all topological items.
void asiEngine_IV::Clean_Topo()
{
  Handle(asiData_IVTopoNode)
    IV_Parent = m_model->GetIVNode()->Topology();
  //
  _cleanChildren(IV_Parent);
}

//-----------------------------------------------------------------------------

//! Finds Node with the given name. Returns null if nothing is found.
//! \param name [in] target name.
//! \return found Node (the first one if several exist) or null.
Handle(asiData_IVTessItemNode)
  asiEngine_IV::Find_TessItem(const TCollection_AsciiString& name)
{
  // Find the first Node with the given name
  for ( Handle(ActAPI_IChildIterator) cit = m_model->GetIVNode()->Tessellation()->GetChildIterator(true); cit->More(); cit->Next() )
  {
    Handle(ActAPI_INode) node = cit->Value();
    //
    if ( node.IsNull() || !node->IsWellFormed() )
      continue;

    TCollection_ExtendedString nodeName = node->GetName();
    //
    if ( nodeName.IsEqual(name) )
      return Handle(asiData_IVTessItemNode)::DownCast(node);
  }

  return NULL;
}

//-----------------------------------------------------------------------------

//! Creates tessellation item.
//! \param tess          [in] surface triangulation to create a tessellation item from.
//! \param name          [in] name of the tessellation item.
//! \param useAutoNaming [in] indicates whether to auto-name entities.
//! \return newly created tessellation item.
Handle(asiData_IVTessItemNode)
  asiEngine_IV::Create_TessItem(const Handle(Poly_Triangulation)& tess,
                                const TCollection_AsciiString&    name,
                                const bool                        useAutoNaming)
{
  // Access Model and parent Node
  Handle(asiData_IVTessNode) IV_Parent = m_model->GetIVNode()->Tessellation();

  // Add Tessellation Item Node to Partition
  Handle(asiData_IVTessItemNode) item_n = Handle(asiData_IVTessItemNode)::DownCast( asiData_IVTessItemNode::Instance() );
  m_model->GetIVTessItemPartition()->AddNode(item_n);

  // Generate unique name
  TCollection_ExtendedString item_name = ( name.IsEmpty() ? "Mesh" : name );
  if ( useAutoNaming )
    item_name = ActData_UniqueNodeName::Generate(ActData_SiblingNodes::CreateForChild(item_n, IV_Parent), item_name);
  //
  item_n->SetName(item_name);

  // Initialize
  Update_TessItem(item_n, tess);

  // Add as child
  IV_Parent->AddChildNode(item_n);

  // Return the just created Node
  return item_n;
}

//-----------------------------------------------------------------------------

//! Updates Tessellation Item Node.
//! \param node [in] Data Node to update.
//! \param tess [in] triangulation to store.
void asiEngine_IV::Update_TessItem(const Handle(asiData_IVTessItemNode)& node,
                                   const Handle(Poly_Triangulation)&     tess)
{
  // Convert to AD mesh
  Handle(ActData_Mesh) mesh;
  if ( !asiAlgo_MeshConvert::ToPersistent(tess, mesh) )
    return;

  // Initialize
  node->Init();
  node->SetMesh(mesh);
}

//-----------------------------------------------------------------------------

//! Deletes all tessellation items.
void asiEngine_IV::Clean_Tess()
{
  Handle(asiData_IVTessNode)
    IV_Parent = m_model->GetIVNode()->Tessellation();
  //
  _cleanChildren(IV_Parent);
}

//-----------------------------------------------------------------------------

//! Finds Node with the given name. Returns null if nothing is found.
//! \param name [in] target name.
//! \return found Node (the first one if several exist) or null.
Handle(asiData_IVTextItemNode)
  asiEngine_IV::Find_TextItem(const TCollection_AsciiString& name)
{
  // Find the first Node with the given name
  for ( Handle(ActAPI_IChildIterator) cit = m_model->GetIVNode()->Text()->GetChildIterator(true); cit->More(); cit->Next() )
  {
    Handle(ActAPI_INode) node = cit->Value();
    //
    if ( node.IsNull() || !node->IsWellFormed() )
      continue;

    TCollection_ExtendedString nodeName = node->GetName();
    //
    if ( nodeName.IsEqual(name) )
      return Handle(asiData_IVTextItemNode)::DownCast(node);
  }

  return NULL;
}

//-----------------------------------------------------------------------------

//! Creates text item.
//! \param text          [in] text to store in the persistent item.
//! \param name          [in] name to set.
//! \param useAutoNaming [in] indicates whether to auto-name entities.
//! \return newly created text item.
Handle(asiData_IVTextItemNode)
  asiEngine_IV::Create_TextItem(const TCollection_AsciiString& text,
                                const TCollection_AsciiString& name,
                                const bool                     useAutoNaming)
{
  // Access Model and parent Node
  Handle(asiData_IVTextNode) IV_Parent = m_model->GetIVNode()->Text();

  // Add Text Item Node to Partition
  Handle(asiData_IVTextItemNode) item_n = Handle(asiData_IVTextItemNode)::DownCast( asiData_IVTextItemNode::Instance() );
  m_model->GetIVTextItemPartition()->AddNode(item_n);

  // Generate name from text
  TCollection_ExtendedString item_name;
  if ( name.IsEmpty() )
  {
    //
    if ( text.IsEmpty() )
      item_name = "Text";
    else
    {
      item_name = text.SubString( 1, Min(text.Length(), 16) );

      if ( text.Length() > item_name.Length() )
        item_name += "...";
    }
  }
  else
    item_name = name;

  // Generate unique
  if ( useAutoNaming )
    item_name = ActData_UniqueNodeName::Generate(ActData_SiblingNodes::CreateForChild(item_n, IV_Parent), item_name);
  //
  item_n->SetName(item_name);

  // Initialize
  Update_TextItem(item_n, text);

  // Add as child
  IV_Parent->AddChildNode(item_n);

  // Return the just created Node
  return item_n;
}

//-----------------------------------------------------------------------------

//! Updates Text Item Node.
//! \param node [in] Data Node to update.
//! \param text [in] text to store.
void asiEngine_IV::Update_TextItem(const Handle(asiData_IVTextItemNode)& node,
                                   const TCollection_AsciiString&        text)
{
  node->Init();
  node->SetText(text);
}

//-----------------------------------------------------------------------------

//! Deletes all text items.
void asiEngine_IV::Clean_Text()
{
  Handle(asiData_IVTextNode)
    IV_Parent = m_model->GetIVNode()->Text();
  //
  _cleanChildren(IV_Parent);
}

//-----------------------------------------------------------------------------

//! Removes all child Nodes for the given parent.
//! \param parent [in] parent Node to clean up children for.
void asiEngine_IV::_cleanChildren(const Handle(ActAPI_INode)& parent)
{
  Handle(ActAPI_HNodeList) nodesToDelete = new ActAPI_HNodeList;

  // Loop over direct children of a Surfaces Node
  for ( Handle(ActAPI_IChildIterator) cit = parent->GetChildIterator(); cit->More(); cit->Next() )
  {
    Handle(ActAPI_INode) child_n = cit->Value();

    // Check if the given Node is consistent
    if ( child_n.IsNull() || !child_n->IsWellFormed() )
      continue;

    // Set Node for deletion
    nodesToDelete->Append(child_n);
  }

  // Delete all Nodes queued for removal
  for ( ActAPI_NodeList::Iterator nit( *nodesToDelete.operator->() ); nit.More(); nit.Next() )
    m_model->DeleteNode( nit.Value()->GetId() );
}
