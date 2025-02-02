//-----------------------------------------------------------------------------
// Created on: 02 December 2015
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
#include <asiUI_ViewerDomain.h>

// asiAlgo includes
#include <asiAlgo_DeleteEdges.h>
#include <asiAlgo_JoinEdges.h>
#include <asiAlgo_Utils.h>

// asiEngine includes
#include <asiEngine_Domain.h>
#include <asiEngine_Part.h>

// asiVisu includes
#include <asiVisu_Utils.h>

// VTK includes
#pragma warning(push, 0)
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkTextRepresentation.h>
#pragma warning(pop)

// Qt-VTK includes
#include <asiVisu_QVTKWidget.h>

// Qt includes
#pragma warning(push, 0)
#include <QDesktopWidget>
#include <QVBoxLayout>
#pragma warning(pop)

// OCCT includes
#include <BRep_Tool.hxx>
#include <TColStd_MapIteratorOfPackedMapOfInteger.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>

//-----------------------------------------------------------------------------

//! Creates a new instance of viewer.
//! \param[in] model    Data Model instance.
//! \param[in] progress progress notifier.
//! \param[in] plotter  imperative plotter.
//! \param[in] parent   parent widget.
asiUI_ViewerDomain::asiUI_ViewerDomain(const Handle(asiEngine_Model)& model,
                                       ActAPI_ProgressEntry           progress,
                                       ActAPI_PlotterEntry            plotter,
                                       QWidget*                       parent)
: asiUI_Viewer(progress, plotter, parent), m_model(model)
{
  // Initialize presentation manager along with QVTK widget
  m_prs_mgr = vtkSmartPointer<asiVisu_PrsManager>::New();
  //
  m_prs_mgr->SetModel(model);
  m_prs_mgr->Initialize(this);
  m_prs_mgr->SetInteractionMode(asiVisu_PrsManager::InteractionMode_2D);
  m_prs_mgr->SetSelectionMode(SelectionMode_Workpiece);
  //
  if ( m_prs_mgr->GetCellPicker().Get() )
    m_prs_mgr->GetCellPicker()->SetTolerance(0.005);

  // Widgets and layouts
  asiVisu_QVTKWidget* pViewer     = m_prs_mgr->GetQVTKWidget();
  QHBoxLayout*        pBaseLayout = new QHBoxLayout(this);

  pBaseLayout->addWidget(pViewer);

  // Configure layout
  pBaseLayout->setSpacing(0);
  pBaseLayout->setContentsMargins(0, 0, 0, 0);

  /* ===================================
   *  Setting up picking infrastructure
   * =================================== */

  // Initialize Callback instance for Pick operation
  m_pickCallback = vtkSmartPointer<asiUI_PickCallback>::New();
  m_pickCallback->SetViewer(this);

  // Initialize Callback instance for Domain operations
  m_domainCallback = vtkSmartPointer<asiUI_PDomainCallback>::New();
  m_domainCallback->SetViewer(this);

  // Set observer for picking
  if ( !m_prs_mgr->GetImageInteractorStyle()->HasObserver(EVENT_PICK_DEFAULT) )
    m_prs_mgr->GetImageInteractorStyle()->AddObserver(EVENT_PICK_DEFAULT, m_pickCallback);

  // Set observer for detection
  if ( !m_prs_mgr->GetImageInteractorStyle()->HasObserver(EVENT_DETECT_DEFAULT) )
    m_prs_mgr->GetImageInteractorStyle()->AddObserver(EVENT_DETECT_DEFAULT, m_pickCallback);

  // Set observer for edge removal
  if ( !m_prs_mgr->GetImageInteractorStyle()->HasObserver(EVENT_DELETE) )
    m_prs_mgr->GetImageInteractorStyle()->AddObserver(EVENT_DELETE, m_domainCallback);

  // Set observer for edge joining
  if ( !m_prs_mgr->GetImageInteractorStyle()->HasObserver(EVENT_JOIN) )
    m_prs_mgr->GetImageInteractorStyle()->AddObserver(EVENT_JOIN, m_domainCallback);

  // Get notified once any sensitive is picked on a section
  connect( m_pickCallback, SIGNAL( picked() ), this, SLOT( onDomainPicked() ) );

  // Get notified of edge removal
  connect( m_domainCallback, SIGNAL( killEdges() ), this, SLOT( onKillEdges() ) );

  // Get notified of edge joining
  connect( m_domainCallback, SIGNAL( joinEdges() ), this, SLOT( onJoinEdges() ) );

  /* =====================================
   *  Finalize initial state of the scene
   * ===================================== */

  // Initialize text widget used for annotations
  m_textWidget = vtkSmartPointer<vtkTextWidget>::New();
  m_textWidget->SelectableOff();
  //
  m_textWidget->SetInteractor      ( m_prs_mgr->GetRenderer()->GetRenderWindow()->GetInteractor() );
  m_textWidget->SetDefaultRenderer ( m_prs_mgr->GetRenderer() );
  m_textWidget->SetCurrentRenderer ( m_prs_mgr->GetRenderer() );
  //
  vtkTextRepresentation* textRep = vtkTextRepresentation::SafeDownCast( m_textWidget->GetRepresentation() );
  vtkSmartPointer<vtkTextActor> textActor = vtkSmartPointer<vtkTextActor>::New();
  textRep->SetTextActor(textActor);
  //
  textRep->GetPositionCoordinate()->SetValue(0.3, 0.8);
  textRep->GetPosition2Coordinate()->SetValue(0.69, 0.19);
  //
  textActor->GetTextProperty()->SetJustificationToLeft();
  textActor->GetTextProperty()->SetVerticalJustificationToTop();

  // Enable context menu
  if ( pViewer )
  {
    pViewer->setContextMenuPolicy(Qt::CustomContextMenu);
    connect( pViewer, SIGNAL ( customContextMenuRequested(const QPoint&) ),
             this,    SLOT   ( onContextMenu(const QPoint&) ) );

    this->onResetView();
  }
}

//-----------------------------------------------------------------------------

//! Destructor.
asiUI_ViewerDomain::~asiUI_ViewerDomain()
{
}

//-----------------------------------------------------------------------------

//! \return size hint.
QSize asiUI_ViewerDomain::sizeHint() const
{
  QDesktopWidget desktop;
  const int side   = std::min( desktop.height(), desktop.width() );
  const int width  = (int) (side*0.25);
  const int height = (int) (side*0.25);

  QSize s(width, height);
  return s;
}

//-----------------------------------------------------------------------------

//! Updates viewer.
void asiUI_ViewerDomain::Repaint()
{
  m_prs_mgr->GetQVTKWidget()->GetRenderWindow()->Render();
}

//-----------------------------------------------------------------------------

//! Resets view.
void asiUI_ViewerDomain::onResetView()
{
  asiVisu_Utils::CameraOnTop( m_prs_mgr->GetRenderer() );
  this->Repaint();
}

//-----------------------------------------------------------------------------

//! Callback for picking event.
void asiUI_ViewerDomain::onDomainPicked()
{
  //Handle(asiData_PartNode) N = m_model->GetPartNode();
  ////
  //if ( N.IsNull() || !N->IsWellFormed() || N->GetShape().IsNull() )
  //  return;

  //// Build a map of shapes
  //const TopTools_IndexedMapOfShape& subShapesMap = N->GetAAG()->GetMapOfSubShapes();

  //// Get face
  //const int face_idx = N->GetFaceRepresentation()->GetSelectedFace();
  ////
  //TopoDS_Face F;
  //if ( face_idx > 0 )
  //  F = TopoDS::Face( subShapesMap.FindKey(face_idx) );
  ////
  //if ( F.IsNull() )
  //  return;

  ////---------------------------------------------------------------------------
  //// Retrieve current selection
  ////---------------------------------------------------------------------------

  //// Access picking results
  //const asiVisu_ActualSelection& sel      = m_prs_mgr->GetCurrentSelection();
  //const Handle(asiVisu_PickerResult)&      pick_res = sel.PickResult(SelectionNature_Pick);
  //const asiVisu_ActorElemMap&    elem_map = pick_res.GetPickMap();

  //// Check if there is anything selected
  //if ( elem_map.IsEmpty() )
  //{
  //  m_textWidget->Off();
  //  this->Repaint();
  //  return;
  //}

  //// Prepare cumulative set of all picked element IDs
  //for ( asiVisu_ActorElemMap::Iterator it(elem_map); it.More(); it.Next() )
  //{
  //  const vtkSmartPointer<vtkActor>&  picked_actor = it.Key();
  //  const TColStd_PackedMapOfInteger& cellGIDs     = it.Value();

  //  // Access polygonal data mapper
  //  vtkPolyDataMapper* pMapper = vtkPolyDataMapper::SafeDownCast( picked_actor->GetMapper() );
  //  if ( !pMapper )
  //  {
  //    m_textWidget->Off();
  //    return;
  //  }

  //  // Access polygonal data
  //  vtkPolyData* pData = vtkPolyData::SafeDownCast( pMapper->GetInput() );
  //  if ( !pData )
  //  {
  //    m_textWidget->Off();
  //    return;
  //  }

  //  // Get edge. We make an exploration loop here in order not to miss seams
  //  const int   edgeId     = cellGIDs.GetMinimalMapped();
  //  int         current_id = 0;
  //  TopoDS_Edge edge;
  //  //
  //  for ( TopExp_Explorer eexp(F, TopAbs_EDGE); eexp.More(); eexp.Next() )
  //  {
  //    ++current_id;
  //    if ( current_id == edgeId )
  //    {
  //      edge = TopoDS::Edge( eexp.Current() );
  //      break;
  //    }
  //  }
  //  //
  //  if ( edge.IsNull() )
  //    return;

  //  // Prepare label
  //  TCollection_AsciiString TITLE  = "(Edge #";
  //                          TITLE += edgeId;
  //                          TITLE += ", ";
  //                          TITLE += asiAlgo_Utils::ShapeAddr(edge).c_str();
  //                          TITLE += ", ";
  //                          TITLE += asiAlgo_Utils::OrientationToString(edge);
  //                          TITLE += " in face)\n";
  //  //
  //  double f, l;
  //  Handle(Geom_Curve) c3d = BRep_Tool::Curve(edge, f, l);
  //  //
  //  TITLE += "3D: ";
  //  TITLE += c3d->DynamicType()->Name();
  //  TITLE += " [";
  //  TITLE += f;
  //  TITLE += ", ";
  //  TITLE += l;
  //  TITLE += " ]\n";

  //  double f2, l2;
  //  Handle(Geom2d_Curve) c2d = BRep_Tool::CurveOnSurface(edge, F, f2, l2);
  //  //
  //  TITLE += "2D: ";
  //  TITLE += c2d->DynamicType()->Name();
  //  TITLE += " [";
  //  TITLE += f2;
  //  TITLE += ", ";
  //  TITLE += l2;
  //  TITLE += " ]";

  //  // Update text on the annotation
  //  m_textWidget->GetTextActor()->SetInput( TITLE.ToCString() );
  //  m_textWidget->On();
  //}
  //this->Repaint();

  // Take picked position from interactor
  //double pickedX = 0.0, pickedY = 0.0;
  //this->PrsMgr()->GetImageInteractorStyle()->GetPickedPos(pickedX, pickedY);

  //// Pick world position
  //vtkSmartPointer<vtkWorldPointPicker>
  //  worldPicker = vtkSmartPointer<vtkWorldPointPicker>::New();
  ////
  //worldPicker->Pick( pickedX, pickedY, 0, this->PrsMgr()->GetRenderer() );
  //double coord[3];
  //worldPicker->GetPickPosition(coord);
  //
  //emit pointPicked(coord[0], coord[1]);
}

//-----------------------------------------------------------------------------

//! Callback for edges removal.
void asiUI_ViewerDomain::onKillEdges()
{
  Handle(asiData_PartNode) N = m_model->GetPartNode();
  //
  if ( N.IsNull() || !N->IsWellFormed() || N->GetShape().IsNull() )
    return;

  TopoDS_Shape part = N->GetShape();

  // Get edges
  TopTools_IndexedMapOfShape selectedEdges;
  asiEngine_Domain::GetHighlightedEdges(N, this->PrsMgr(), selectedEdges);

  // Delete selected edges
  asiAlgo_DeleteEdges eraser(part);
  if ( !eraser.Perform(selectedEdges, true) )
  {
    std::cout << "Error: cannot delete edges" << std::endl;
    return;
  }

  const TopoDS_Shape& result = eraser.Result();

  // Save to model
  m_model->OpenCommand();
  {
    asiEngine_Part(m_model).Update(result);
  }
  m_model->CommitCommand();

  // Update viewer
  this->PrsMgr()->DeleteAllPresentations();
  this->PrsMgr()->Actualize( N.get() );

  // Notify
  emit partModified();
}

//-----------------------------------------------------------------------------

//! Callback for edges joining.
void asiUI_ViewerDomain::onJoinEdges()
{
  Handle(asiData_PartNode) N = m_model->GetPartNode();
  //
  if ( N.IsNull() || !N->IsWellFormed() || N->GetShape().IsNull() )
    return;

  TopoDS_Shape part = N->GetShape();

  // Get edges
  TopoDS_Face face;
  TopTools_IndexedMapOfShape selectedEdges;
  asiEngine_Domain::GetHighlightedEdges(N, this->PrsMgr(), selectedEdges, face);

  // Join selected edges
  asiAlgo_JoinEdges joiner(part);
  if ( !joiner.Perform(selectedEdges, face) )
  {
    std::cout << "Error: cannot join edges" << std::endl;
    return;
  }

  const TopoDS_Shape& result = joiner.Result();

  // Save to model
  m_model->OpenCommand();
  {
    asiEngine_Part(m_model).Update(result);
  }
  m_model->CommitCommand();

  // Update viewer
  this->PrsMgr()->DeleteAllPresentations();
  this->PrsMgr()->Actualize( N.get() );
}

//-----------------------------------------------------------------------------

void asiUI_ViewerDomain::onContextMenu(const QPoint& pos)
{
  asiVisu_QVTKWidget* pViewer   = m_prs_mgr->GetQVTKWidget();
  QPoint              globalPos = pViewer->mapToGlobal(pos);

  emit contextMenu(globalPos);
}
