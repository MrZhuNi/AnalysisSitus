//-----------------------------------------------------------------------------
// Created on: 03 December 2015
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
#include <asiUI_ObjectBrowser.h>

// asiUI includes
#include <asiUI_Common.h>
#include <asiUI_DependencyGraph.h>
#include <asiUI_DialogPipelines.h>
#include <asiUI_ViewerPart.h>

// asiEngine includes
#include <asiEngine_IV.h>
#include <asiEngine_Part.h>
#include <asiEngine_PatchJointAdaptor.h>
#include <asiEngine_RE.h>
#include <asiEngine_Tessellation.h>

// asiVisu includes
#include <asiVisu_PartPrs.h>

// asiAlgo includes
#include <asiAlgo_JSON.h>
#include <asiAlgo_MeshConvert.h>
#include <asiAlgo_PatchJointAdaptor.h>
#include <asiAlgo_Utils.h>
#include <asiAlgo_WriteREK.h>

// OpenCascade includes
#include <TDF_Tool.hxx>

// Qt includes
#pragma warning(push, 0)
#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QMenu>
#include <QTreeWidgetItemIterator>
#pragma warning(pop)

#define TREEVIEW_MINSIZE 200

//-----------------------------------------------------------------------------

namespace
{
  void PropagateFlag(QTreeWidgetItem* pItem, Qt::CheckState state)
  {
    if ( pItem->flags() & Qt::ItemIsUserCheckable )
      pItem->setCheckState(1, state);

    // Proceed with its children.
    for ( int c = 0; c < pItem->childCount(); ++c )
      PropagateFlag(pItem->child(c), state);
  }
}

//-----------------------------------------------------------------------------

asiUI_ObjectBrowser::asiUI_ObjectBrowser(const Handle(asiEngine_Model)& model,
                                         ActAPI_ProgressEntry           progress,
                                         QWidget*                       parent)
: QTreeWidget(parent), m_model(model), m_progress(progress)
{
  // Configure.
  this->setMinimumWidth(TREEVIEW_MINSIZE);
  this->setEditTriggers(QAbstractItemView::NoEditTriggers);
  this->header()->hide();
  this->setColumnCount(2);
  this->setAutoExpandDelay(0);

  // Populate.
  this->Populate();

  // Configure selection.
  this->setSelectionMode(QAbstractItemView::ExtendedSelection);
  this->setSelectionBehavior(QAbstractItemView::SelectRows);

  /* Reactions. */

  connect( this, SIGNAL( itemSelectionChanged() ), this, SLOT( onSelectionChanged() ) );

  this->setContextMenuPolicy(Qt::CustomContextMenu);

  connect( this, SIGNAL( customContextMenuRequested(QPoint) ), this, SLOT( onContextMenu(QPoint) ) );

  connect( this->model(), SIGNAL( dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&) ),
           this,          SLOT( onDataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&) ) );
}

//-----------------------------------------------------------------------------

asiUI_ObjectBrowser::~asiUI_ObjectBrowser()
{}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::SetPlotter(ActAPI_PlotterEntry plotter)
{
  m_plotter = plotter;
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::AddAssociatedViewer(asiUI_Viewer* pViewer)
{
  m_viewers.push_back(pViewer);
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::SetParameterEditor(const Handle(asiUI_IParamEditor)& editor)
{
  m_paramEditor = editor;
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::Populate()
{
  // Get selected Node (if any).
  Handle(ActAPI_INode) selN;
  this->selectedNode(selN);

  // Clean up the existing contents.
  this->clear();

  // Add root node.
  Handle(ActAPI_INode) root_n = m_model->GetRootNode();
  //
  if ( root_n.IsNull() || !root_n->IsWellFormed() )
    return;
  //
  QTreeWidgetItem* root_ui = new QTreeWidgetItem( QStringList() << ExtStr2QStr( root_n->GetName() ) );
  root_ui->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  root_ui->setData( 0, BrowserRoleNodeId, AsciiStr2QStr( root_n->GetId() ) );
  //
  this->addTopLevelItem(root_ui);

  const bool isHidden = ( root_n->GetUserFlags() & NodeFlag_IsHiddenInBrowser ) > 0;
  root_ui->setHidden(isHidden);

  // Add child nodes.
  this->addChildren(root_n, root_ui);

  // Expand tree.
  this->expandAll();

  // Reselect Node.
  if ( !selN.IsNull() )
    this->SetSelectedNode( selN->GetId() );

  // Beautify.
  this->resizeColumnToContents(0);
  this->resizeColumnToContents(1);
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::UpdateSelectedNode()
{
  Handle(ActAPI_INode) N;
  QTreeWidgetItem*     pItem = nullptr;

  if ( !this->selectedNode(N, pItem) )
    return;

  if ( pItem )
  {
    pItem->setText( 0, ExtStr2QStr( N->GetName() ) );

    // Do any other actualization on the widget item.
  }
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::SetSelectedNode(const ActAPI_DataObjectId& nodeId)
{
  // Clear current selection.
  this->selectionModel()->clearSelection();

  // Find the Node asked for selection and select it.
  QTreeWidgetItemIterator it(this);
  //
  while ( *it )
  {
    QString itemData = (*it)->data(0, BrowserRoleNodeId).toString();
    if ( QStr2AsciiStr(itemData) == nodeId )
    {
      (*it)->setSelected(true);
      break;
    }
    else
      ++it;
  }
}

//-----------------------------------------------------------------------------

Handle(ActAPI_INode) asiUI_ObjectBrowser::GetSelectedNode() const
{
  Handle(ActAPI_INode) selected;
  if ( !this->selectedNode(selected) )
    return nullptr;

  return selected;
}

//-----------------------------------------------------------------------------

Handle(ActAPI_INode) asiUI_ObjectBrowser::GetSelectedNode(QTreeWidgetItem*& pItem) const
{
  Handle(ActAPI_INode) selected;
  if ( !this->selectedNode(selected, pItem) )
    return nullptr;

  return selected;
}

//-----------------------------------------------------------------------------

Handle(ActAPI_HNodeList) asiUI_ObjectBrowser::GetSelectedNodes() const
{
  Handle(ActAPI_HNodeList) selected;
  if ( !this->selectedNodes(selected) )
    return nullptr;

  return selected;
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::addChildren(const Handle(ActAPI_INode)& root_n,
                                      QTreeWidgetItem*            root_ui)
{
  if ( root_n.IsNull() )
    return;

  // Allow adding bad-formed Nodes, just mark them specifically.
  if ( !root_n->IsWellFormed() )
    root_ui->setBackgroundColor(0, Qt::darkRed);

  // Loop over the children.
  for ( Handle(ActAPI_IChildIterator) cit = root_n->GetChildIterator(); cit->More(); cit->Next() )
  {
    Handle(ActAPI_INode) child_n = cit->Value();

    // Create child.
    QTreeWidgetItem* child_ui = new QTreeWidgetItem( QStringList() << ExtStr2QStr( child_n->GetName() ) );

    // Configure child.
    child_ui->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    //
    if ( child_n->GetUserFlags() & NodeFlag_IsPresentedInPartView ) 
    {
      child_ui->setFlags(child_ui->flags() | Qt::ItemIsUserCheckable);
      child_ui->setCheckState(1, (child_n->GetUserFlags() & NodeFlag_IsPresentationVisible) ? Qt::Checked : Qt::Unchecked);
    }
    //
    child_ui->setData( 0, BrowserRoleNodeId, AsciiStr2QStr( child_n->GetId() ) );

    // Add child.
    root_ui->addChild(child_ui);

    // Check user flags which may customize visibility of an item.
    const bool isHidden = ( child_n->GetUserFlags() & NodeFlag_IsHiddenInBrowser ) > 0;
    child_ui->setHidden(isHidden);

    // Repeat recursively.
    this->addChildren(child_n, child_ui);
  }
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onDataChanged(const QModelIndex&  topLeft,
                                        const QModelIndex&  asiUI_NotUsed(bottomRight),
                                        const QVector<int>& asiUI_NotUsed(roles))
{
  if ( topLeft.column() == 1 )
  {
    QTreeWidgetItem*         item_ui = this->itemFromIndex(topLeft);
    TCollection_AsciiString  entry   = QStr2AsciiStr( item_ui->data(0, BrowserRoleNodeId).toString() );
    Handle(ActAPI_INode)     N       = m_model->FindNode(entry);
    Handle(ActAPI_HNodeList) nodes   = new ActAPI_HNodeList;
    nodes->Append(N);

    if ( item_ui->checkState(1) == Qt::Checked )
    {
      this->showNodes(nodes);
    }
    else
    {
      this->hideNodes(nodes);
    }
  }
}

//-----------------------------------------------------------------------------

//! Reaction on selection in a tree view.
void asiUI_ObjectBrowser::onSelectionChanged()
{
  // Populate parameter editor.
  ActAPI_DataObjectIdList selIds;
  //
  if ( !m_paramEditor.IsNull() )
  {
    Handle(ActAPI_HNodeList) nodes = this->GetSelectedNodes();
    //
    if ( nodes.IsNull() || nodes->Length() != 1 )
      m_paramEditor->SetParameters( nullptr );
    else
      m_paramEditor->SetParameters( nodes->First()->Parameters() );

    // Collect IDs of the selected Nodes to pass them to the listeners.
    if ( !nodes.IsNull() )
      for ( ActAPI_HNodeList::Iterator nit(*nodes); nit.More(); nit.Next() )
        selIds.Append( nit.Value()->GetId() );
  }

  emit nodesSelected(selIds);
}

//-----------------------------------------------------------------------------

//! Reaction on context menu opening.
//! \param[in] pos position.
void asiUI_ObjectBrowser::onContextMenu(QPoint pos)
{
  Handle(ActAPI_HNodeList) selected_n;
  if ( !this->selectedNodes(selected_n) ) return;

  // Create and populate the menu.
  QMenu* pMenu = new QMenu(this);
  //
  this->populateContextMenu(selected_n, pMenu);
  //
  if ( !pMenu->isEmpty() )
    pMenu->popup( this->mapToGlobal(pos) );
}

//-----------------------------------------------------------------------------

//! Reaction on "show" action.
void asiUI_ObjectBrowser::onShow()
{
  Handle(ActAPI_HNodeList) selected_n;
  if ( !this->selectedNodes(selected_n) ) return;

  // Modify UI.
  this->blockSignals(true);
  this->model()->blockSignals(true);
  {
    QList<QTreeWidgetItem*> items = this->selectedItems();
    for ( QList<QTreeWidgetItem*>::iterator iit = items.begin(); iit != items.end(); ++iit )
    {
      if ( (*iit)->flags() & Qt::ItemIsUserCheckable )
        (*iit)->setCheckState(1, Qt::Checked);
    }
  }
  this->blockSignals(false);
  this->model()->blockSignals(false);

  // Modify Data Model.
  this->showNodes(selected_n);
}

//-----------------------------------------------------------------------------

//! Reaction on "show only" action.
void asiUI_ObjectBrowser::onShowOnly()
{
  Handle(ActAPI_HNodeList) selected_n;
  if ( !this->selectedNodes(selected_n) ) return;

  this->blockSignals(true);
  this->model()->blockSignals(true);
  {
    // Hide all items.
    for ( int i = 0; i < this->topLevelItemCount(); ++i )
    {
      QTreeWidgetItem* pItem = this->topLevelItem(i);
      PropagateFlag(pItem, Qt::Unchecked);
    }

    // Check items which go visible.
    QList<QTreeWidgetItem*> items = this->selectedItems();
    for ( QList<QTreeWidgetItem*>::iterator iit = items.begin(); iit != items.end(); ++iit )
    {
      if ( (*iit)->flags() & Qt::ItemIsUserCheckable )
        (*iit)->setCheckState(1, Qt::Checked);
    }
  }
  this->blockSignals(false);
  this->model()->blockSignals(false);

  // Modify Data Model.
  this->showOnlyNodes(selected_n);
}

//-----------------------------------------------------------------------------

//! Reaction on "hide" action.
void asiUI_ObjectBrowser::onHide()
{
  Handle(ActAPI_HNodeList) selected_n;
  if ( !this->selectedNodes(selected_n) ) return;

  // Modify UI.
  this->blockSignals(true);
  this->model()->blockSignals(true);
  {
    QList<QTreeWidgetItem*> items = this->selectedItems();
    for ( QList<QTreeWidgetItem*>::iterator iit = items.begin(); iit != items.end(); ++iit )
    {
      if ( (*iit)->flags() & Qt::ItemIsUserCheckable )
        (*iit)->setCheckState(1, Qt::Unchecked);
    }
  }
  this->blockSignals(false);
  this->model()->blockSignals(false);

  // Modify Data Model.
  this->hideNodes(selected_n);
}

//-----------------------------------------------------------------------------

//! Reaction on "manage pipelines" action.
void asiUI_ObjectBrowser::onManagePipelines()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  // Iterate over the associated viewers to find presentation.
  Handle(asiVisu_Prs)                 prs;
  vtkSmartPointer<asiVisu_PrsManager> prsMgr;
  //
  for ( size_t k = 0; k < m_viewers.size(); ++k )
    if ( m_viewers[k] && m_viewers[k]->PrsMgr()->IsPresented(selected_n) )
    {
      prsMgr = m_viewers[k]->PrsMgr();
      prs    = prsMgr->GetPresentation(selected_n);
      break;
    }

  // Show dialog to manage presentation.
  if ( !prs.IsNull() )
  {
    asiUI_DialogPipelines* pDlg = new asiUI_DialogPipelines(prs, prsMgr, m_progress, this);
    pDlg->show();
  }
  else
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Cannot manage pipelines for a non-presented Node.");
  }
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onShowExecutionGraph()
{
  asiUI_DependencyGraph*
    pGraph = new asiUI_DependencyGraph(m_model);
  //
  pGraph->Render();
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onHidePartEdges()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  if ( !selected_n->IsKind( STANDARD_TYPE(asiData_PartNode) ) )
    return;

  for ( size_t k = 0; k < m_viewers.size(); ++k )
    if ( m_viewers[k] && m_viewers[k]->PrsMgr()->IsPresented(selected_n) )
    {
      Handle(asiVisu_PartPrs)
        prs = Handle(asiVisu_PartPrs)::DownCast( m_viewers[k]->PrsMgr()->GetPresentation(selected_n) );

      prs->ContourActor()->SetVisibility(0);

      m_viewers[k]->Repaint();
    }
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onShowPartEdges()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  if ( !selected_n->IsKind( STANDARD_TYPE(asiData_PartNode) ) )
    return;

  for ( size_t k = 0; k < m_viewers.size(); ++k )
    if ( m_viewers[k] && m_viewers[k]->PrsMgr()->IsPresented(selected_n) )
    {
      Handle(asiVisu_PartPrs)
        prs = Handle(asiVisu_PartPrs)::DownCast( m_viewers[k]->PrsMgr()->GetPresentation(selected_n) );

      prs->ContourActor()->SetVisibility(1);

      m_viewers[k]->Repaint();
    }
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onResetPartPrs()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  if ( !selected_n->IsKind( STANDARD_TYPE(asiData_PartNode) ) )
    return;

  for ( size_t k = 0; k < m_viewers.size(); ++k )
    if ( m_viewers[k] && m_viewers[k]->PrsMgr()->IsPresented(selected_n) )
    {
      m_viewers[k]->PrsMgr()->DeletePresentation(selected_n);
      m_viewers[k]->PrsMgr()->Actualize(selected_n, false, true);
    }
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onCopyAsJSON()
{
  Handle(ActAPI_INode) sel;
  if ( !this->selectedNode(sel) ) return;

  // Dump either curve or surface to JSON.
  std::ostringstream jsonStream;
  //
  if ( sel->IsKind( STANDARD_TYPE(asiData_IVCurveNode) ) )
  {
    asiAlgo_JSON::DumpCurve(Handle(asiData_IVCurveNode)::DownCast(sel)->GetCurve(),
                            jsonStream);
  }
  else if ( sel->IsKind( STANDARD_TYPE(asiData_IVSurfaceNode) ) )
  {
    asiAlgo_JSON::DumpSurface(Handle(asiData_IVSurfaceNode)::DownCast(sel)->GetSurface(),
                              jsonStream);
  }
  else
    return;

  // Copy to clipboard.
  QApplication::clipboard()->setText( jsonStream.str().c_str() );
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onSaveToBREP()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  if ( !selected_n->IsKind( STANDARD_TYPE(asiData_IVTopoItemNode) ) )
    return;

  Handle(asiData_IVTopoItemNode)
    topoNode = Handle(asiData_IVTopoItemNode)::DownCast(selected_n);

  QString filename = asiUI_Common::selectBRepFile(asiUI_Common::OpenSaveAction_Save);

  // Save shape.
  if ( !asiAlgo_Utils::WriteBRep( topoNode->GetShape(), QStr2AsciiStr(filename) ) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Cannot save shape.");
    return;
  }
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onSetAsPart()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  if ( !selected_n->IsKind( STANDARD_TYPE(asiData_IVTopoItemNode) ) )
    return;

  // Convert to the only supported type.
  Handle(asiData_IVTopoItemNode)
    topoNode = Handle(asiData_IVTopoItemNode)::DownCast(selected_n);

  // Get shape to convert.
  TopoDS_Shape shapeToSet = topoNode->GetShape();
  const double linDefl    = topoNode->GetLinearDeflection();
  const double angDefl    = topoNode->GetAngularDeflection();

  // Modify Data Model.
  Handle(asiData_PartNode) part_n;
  m_model->OpenCommand();
  {
    part_n = asiEngine_Part(m_model).Update(shapeToSet);
    //
    part_n->SetLinearDeflection(linDefl);
    part_n->SetAngularDeflection(angDefl);
  }
  m_model->CommitCommand();

  // Update UI.
  for ( size_t k = 0; k < m_viewers.size(); ++k )
  {
    // Clear topological item.
    if ( m_viewers[k] && m_viewers[k]->PrsMgr()->IsPresented(topoNode) )
      m_viewers[k]->PrsMgr()->DeRenderPresentation(topoNode);

    // Actualize part.
    if ( dynamic_cast<asiUI_ViewerPart*>(m_viewers[k]) )
      m_viewers[k]->PrsMgr()->Actualize(part_n);
  }
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onSaveToXYZ()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  if ( !selected_n->IsKind( STANDARD_TYPE(asiData_IVPointSetNode) ) )
    return;

  Handle(asiData_IVPointSetNode)
    ptsNode = Handle(asiData_IVPointSetNode)::DownCast(selected_n);

  QString filename = asiUI_Common::selectXYZFile(asiUI_Common::OpenSaveAction_Save);

  // Get point cloud.
  Handle(asiAlgo_BaseCloud<double>) pts = ptsNode->GetPoints();

  // Save points.
  if ( !pts->SaveAs( QStr2AsciiStr(filename).ToCString() ) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Cannot save point cloud.");
    return;
  }
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onSaveToREK()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  if ( !selected_n->IsKind( STANDARD_TYPE(asiData_OctreeNode) ) )
    return;

  Handle(asiData_OctreeNode)
    N = Handle(asiData_OctreeNode)::DownCast(selected_n);

  // Get the uniform grid to save to REK.
  Handle(asiAlgo_UniformGrid<float>) grid = N->GetUniformGrid();
  //
  if ( grid.IsNull() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "No uniform grid is available.");
    return;
  }

  QString filename = asiUI_Common::selectREKFile(asiUI_Common::OpenSaveAction_Save);

  // Save the grid.
  asiAlgo_WriteREK WriteREK( QStr2StdStr(filename) );
  //
  if ( !WriteREK.Write(grid) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Cannot write REK file.");
    return;
  }
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onPrintParameters()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  // Loop over the parameters.
  TCollection_AsciiString dump("Parameters of \'");
  dump += selected_n->GetName();
  dump += "\' [";
  dump += selected_n->GetId();
  dump += "]";
  dump += " (";
  dump += selected_n->DynamicType()->Name();
  dump += ")";
  dump += ":\n";
  //
  for ( Handle(ActAPI_IParamIterator) pit = selected_n->GetParamIterator(); pit->More(); pit->Next() )
  {
    const Handle(ActAPI_IUserParameter)& P = pit->Value();

    TCollection_AsciiString paramEntry;
    TDF_Tool::Entry(P->RootLabel(), paramEntry);

    dump += "\t";
    dump += "[";
    dump += pit->Key();
    dump += "] ";
    dump += P->DynamicType()->Name();
    dump += " (";
    dump += paramEntry;
    dump += ")";
    dump += "\n";
  }

  m_progress.SendLogMessage(LogInfo(Normal) << dump);
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onCopyName()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  // Set to clipboard.
  QClipboard* clipboard = QApplication::clipboard();
  clipboard->setText( ExtStr2QStr( selected_n->GetName() ) );

  // Notify.
  m_progress.SendLogMessage( LogInfo(Normal) << "Selected Node's name: '%1'."
                                             << selected_n->GetName() );
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onCopyID()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  // Set to clipboard.
  QClipboard* clipboard = QApplication::clipboard();
  clipboard->setText( ExtStr2QStr( selected_n->GetId() ) );

  // Notify.
  m_progress.SendLogMessage( LogInfo(Normal) << "Selected Node's ID: %1."
                                             << selected_n->GetId() );
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onComputeNorms(const bool doElemNorms)
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  if ( !selected_n->IsKind( STANDARD_TYPE(asiData_TessNode) ) )
    return;

  Handle(asiData_TessNode)
    tessNode = Handle(asiData_TessNode)::DownCast(selected_n);

  // Modify Data Model.
  Handle(asiData_TessNormsNode) tessNormsNode;
  //
  m_model->OpenCommand();
  {
    tessNormsNode = asiEngine_Tessellation(m_model).ComputeNorms(tessNode, doElemNorms);
  }
  m_model->CommitCommand();

  // Update UI.
  for ( size_t k = 0; k < m_viewers.size(); ++k )
  {
    // Actualize in Part Viewer.
    asiUI_ViewerPart* pViewerPart = dynamic_cast<asiUI_ViewerPart*>(m_viewers[k]);
    //
    if ( pViewerPart )
      pViewerPart->PrsMgr()->Actualize(tessNormsNode);
  }
  //
  this->Populate();
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onComputeNodalNorms()
{
  this->onComputeNorms(false);
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onComputeElementalNorms()
{
  this->onComputeNorms(true);
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onConvertToTris()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  if ( !selected_n->IsKind( STANDARD_TYPE(asiData_TessNode) ) )
    return;

  Handle(asiData_TessNode)
    tessNode = Handle(asiData_TessNode)::DownCast(selected_n);

  // Convert to Poly triangulation.
  Handle(Poly_Triangulation) polyTris;
  asiAlgo_MeshConvert::FromPersistent(tessNode->GetMesh(), polyTris);

  // Modify Data Model.
  m_model->OpenCommand();
  {
    m_model->GetTriangulationNode()->SetTriangulation(polyTris);
  }
  m_model->CommitCommand();

  // Update UI.
  for ( size_t k = 0; k < m_viewers.size(); ++k )
  {
    // Actualize in Part Viewer.
    asiUI_ViewerPart* pViewerPart = dynamic_cast<asiUI_ViewerPart*>(m_viewers[k]);
    //
    if ( pViewerPart )
      pViewerPart->PrsMgr()->Actualize( m_model->GetTriangulationNode() );
  }
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onConvertToTess()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  if ( !selected_n->IsKind( STANDARD_TYPE(asiData_TriangulationNode) ) )
    return;

  Handle(asiData_TriangulationNode)
    trisNode = Handle(asiData_TriangulationNode)::DownCast(selected_n);

  // Convert to Poly triangulation.
  Handle(ActData_Mesh) mesh;
  asiAlgo_MeshConvert::ToPersistent(trisNode->GetTriangulation(), mesh);

  // Modify Data Model.
  m_model->OpenCommand();
  {
    m_model->GetTessellationNode()->SetMesh(mesh);
  }
  m_model->CommitCommand();

  // Update UI.
  for ( size_t k = 0; k < m_viewers.size(); ++k )
  {
    // Actualize in Part Viewer.
    asiUI_ViewerPart* pViewerPart = dynamic_cast<asiUI_ViewerPart*>(m_viewers[k]);
    //
    if ( pViewerPart )
      pViewerPart->PrsMgr()->Actualize( m_model->GetTessellationNode() );
  }
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onDumpJoint()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  if ( !selected_n->IsKind( STANDARD_TYPE(asiData_ReEdgeNode) ) )
    return;

  // Get Edge Node.
  Handle(asiData_ReEdgeNode)
    edgeNode = Handle(asiData_ReEdgeNode)::DownCast(selected_n);

  // Check if the Node is regular.
  asiEngine_RE reApi(m_model, m_progress, m_plotter);
  //
  int valencef = 0, valencel = 0;
  //
  if ( reApi.IsRegular(edgeNode) )
    m_progress.SendLogMessage(LogNotice(Normal) << "Edge is regular.");
  else
    m_progress.SendLogMessage(LogNotice(Normal) << "Edge is irregular.");

  m_progress.SendLogMessage(LogInfo(Normal) << "Valence of the first vertex is %1." << valencef);
  m_progress.SendLogMessage(LogInfo(Normal) << "Valence of the last vertex is %1." << valencel);

  // Get coedges.
  Handle(ActAPI_HParameterList) backRefs = edgeNode->GetReferrers();
  //
  for ( ActAPI_HParameterList::Iterator it(*backRefs); it.More(); it.Next() )
  {
    const Handle(ActAPI_IUserParameter)& refParam = it.Value();
    Handle(asiData_ReCoedgeNode)         refNode  = Handle(asiData_ReCoedgeNode)::DownCast( refParam->GetNode() );

    if ( !refNode.IsNull() )
      m_progress.SendLogMessage( LogInfo(Normal) << "Referring coedge of edge %1 is %2 (%3)."
                                                 << edgeNode->GetId() << refNode->GetId() << refNode->GetName() );
  }

  // Analyze joint.
  asiEngine_PatchJointAdaptor jointAdaptor(m_model, m_progress, m_plotter);
  //
  if ( !jointAdaptor.Init(edgeNode) )
    return;

  // Dump graphically.
  jointAdaptor.DumpGraphically();
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onUnifyKnotsAndAlign()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  if ( !selected_n->IsKind( STANDARD_TYPE(asiData_ReEdgeNode) ) )
    return;

  // Get Edge Node.
  Handle(asiData_ReEdgeNode)
    edgeNode = Handle(asiData_ReEdgeNode)::DownCast(selected_n);

  // Analyze joint.
  asiEngine_PatchJointAdaptor jointAdaptor(m_model, m_progress, m_plotter);
  //
  if ( !jointAdaptor.Init(edgeNode) )
    return;

  // Extract isos by performing pure geometric analysis.
  Handle(Geom_BSplineCurve) isoLeft, isoRight;
  bool                      isoLeftU, isoRightU, isoLeftMin, isoRightMin, areOpposite;
  //
  if ( !jointAdaptor.ExtractIsos(isoLeft, isoLeftU, isoLeftMin,
                                 isoRight, isoRightU, isoRightMin,
                                 areOpposite) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Failed to extract isos.");
    return; // Failure.
  }

  // Make surfaces compatible.
  if ( !jointAdaptor.UnifySurfaces(isoLeft, isoLeftU, isoRight, isoRightU, areOpposite) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Failed to unify surfaces.");
    return;
  }

  // Align poles.
  if ( !jointAdaptor.AlignControlPoles(isoLeft, isoLeftU, isoLeftMin,
                                       isoRight, isoRightU, isoRightMin,
                                       areOpposite) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Failed to align poles of surfaces.");
    return;
  }

  const Handle(asiData_RePatchNode)& leftPatchNode  = jointAdaptor.GetPatchLeft();
  const Handle(asiData_RePatchNode)& rightPatchNode = jointAdaptor.GetPatchRight();

  // Store the updated surfaces.
  m_model->OpenCommand();
  {
    leftPatchNode->SetSurface( jointAdaptor.GetSurfaceLeft() );
    rightPatchNode->SetSurface( jointAdaptor.GetSurfaceRight() );
  }
  m_model->CommitCommand();

  // Actualize presentations.
  for ( size_t k = 0; k < m_viewers.size(); ++k )
  {
    if ( dynamic_cast<asiUI_ViewerPart*>(m_viewers[k]) )
    {
      m_viewers[k]->PrsMgr()->Actualize(leftPatchNode);
      m_viewers[k]->PrsMgr()->Actualize(rightPatchNode);
    }
  }
}

//-----------------------------------------------------------------------------

//! Populates context menu with actions.
//! \param[in]     activeNodes currently active Nodes.
//! \param[in,out] pMenu       menu to populate.
void asiUI_ObjectBrowser::populateContextMenu(const Handle(ActAPI_HNodeList)& activeNodes,
                                              QMenu*                          pMenu)
{
  if ( activeNodes.IsNull() || activeNodes->IsEmpty() )
    return;

  // Get type of the first Node.
  const int             numSelected = activeNodes->Length();
  Handle(ActAPI_INode)  node        = activeNodes->First();
  Handle(Standard_Type) type        = node->DynamicType();

  // Check whether other Nodes are of the same type.
  bool isOfSameType = true;
  //
  for ( ActAPI_HNodeList::Iterator nit(*activeNodes); nit.More(); nit.Next() )
  {
    const Handle(ActAPI_INode)& N = nit.Value();
    //
    if ( N->DynamicType() != type )
    {
      isOfSameType = false;
      break;
    }
  }
  //
  if ( !isOfSameType )
  {
    m_progress.SendLogMessage(LogWarn(Normal) << "Selected Nodes are of different types.");
    return;
  }

  // Iterate over the associated viewer to find the one where the selected
  // Node is presented.
  bool isPresented = false;
  for ( size_t k = 0; k < m_viewers.size(); ++k )
  {
    if ( m_viewers[k] && m_viewers[k]->PrsMgr()->IsPresented(node) )
    {
      isPresented = true;
      break;
    }
  }

  if ( numSelected == 1 )
  {
    pMenu->addAction( "Print parameters", this, SLOT( onPrintParameters () ) );
    pMenu->addAction( "Copy name",        this, SLOT( onCopyName        () ) );
    pMenu->addAction( "Copy ID",          this, SLOT( onCopyID          () ) );
  }
  //
  if ( node->IsKind( STANDARD_TYPE(asiData_RootNode) ) )
  {
    pMenu->addSeparator();
    pMenu->addAction( "Show execution graph", this, SLOT( onShowExecutionGraph () ) );
  }
  //
  if ( isPresented )
  {
    pMenu->addSeparator();
    pMenu->addAction( "Show",      this, SLOT( onShow     () ) );
    pMenu->addAction( "Show only", this, SLOT( onShowOnly () ) );
    pMenu->addAction( "Hide",      this, SLOT( onHide     () ) );

    if ( numSelected == 1 )
    {
      pMenu->addAction( "Manage pipelines...", this, SLOT( onManagePipelines () ) );

      if ( node->IsKind( STANDARD_TYPE(asiData_PartNode) ) )
      {
        pMenu->addSeparator();
        pMenu->addAction( "Hide edges",         this, SLOT( onHidePartEdges () ) );
        pMenu->addAction( "Show edges",         this, SLOT( onShowPartEdges () ) );
        pMenu->addAction( "Reset presentation", this, SLOT( onResetPartPrs  () ) );
      }

      if ( node->IsKind( STANDARD_TYPE(asiData_IVSurfaceNode) ) ||
           node->IsKind( STANDARD_TYPE(asiData_IVCurveNode) ) )
      {
        pMenu->addSeparator();
        pMenu->addAction( "Copy as JSON", this, SLOT( onCopyAsJSON () ) );
      }

      if ( node->IsKind( STANDARD_TYPE(asiData_IVTopoItemNode) ) )
      {
        pMenu->addSeparator();
        pMenu->addAction( "Save to BREP...", this, SLOT( onSaveToBREP () ) );
        pMenu->addAction( "Set as part",     this, SLOT( onSetAsPart  () ) );
      }

      if ( node->IsKind( STANDARD_TYPE(asiData_IVPointSetNode) ) )
      {
        pMenu->addSeparator();
        pMenu->addAction( "Save to XYZ...", this, SLOT( onSaveToXYZ () ) );
      }

      if ( node->IsKind( STANDARD_TYPE(asiData_OctreeNode) ) )
      {
        pMenu->addSeparator();
        pMenu->addAction( "Save to REK...", this, SLOT( onSaveToREK () ) );
      }

      if ( node->IsKind( STANDARD_TYPE(asiData_TessNode) ) )
      {
        pMenu->addSeparator();
        pMenu->addAction( "Compute nodal norms",      this, SLOT( onComputeNodalNorms     () ) );
        pMenu->addAction( "Compute elemental norms",  this, SLOT( onComputeElementalNorms () ) );
        pMenu->addAction( "Convert to triangulation", this, SLOT( onConvertToTris         () ) );
      }

      if ( node->IsKind( STANDARD_TYPE(asiData_TriangulationNode) ) )
      {
        pMenu->addSeparator();
        pMenu->addAction( "Convert to tessellation",  this, SLOT( onConvertToTess () ) );
      }

      if ( node->IsKind( STANDARD_TYPE(asiData_ReEdgeNode) ) )
      {
        pMenu->addSeparator();
        pMenu->addAction( "Dump joint info",       this, SLOT( onDumpJoint          () ) );
        pMenu->addAction( "Unify knots and align", this, SLOT( onUnifyKnotsAndAlign () ) );
      }
    }
  }
}

//-----------------------------------------------------------------------------

//! Returns the currently active Node.
//! \param[out] Node  requested Node.
//! \param[out] pItem UI item corresponding to the selected Node.
//! \return true in case of success, false -- otherwise.
bool asiUI_ObjectBrowser::selectedNode(Handle(ActAPI_INode)& Node,
                                       QTreeWidgetItem*&     pItem) const
{
  QList<QTreeWidgetItem*> items = this->selectedItems();
  if ( !items.length() || items.length() > 1 )
    return false;

  pItem = items.at(0);
  TCollection_AsciiString entry = QStr2AsciiStr( pItem->data(0, BrowserRoleNodeId).toString() );

  // Take the corresponding data object
  Handle(ActAPI_INode) selected_n = m_model->FindNode(entry);
  //
  if ( selected_n.IsNull() || !selected_n->IsWellFormed() )
  {
    std::cout << "Error: selected Node is invalid" << std::endl;

    m_progress.SendLogMessage(LogWarn(Normal) << "Selected Node is invalid.");
    pItem->setBackgroundColor(0, Qt::darkRed);
    return false;
  }

  // Set result
  Node = selected_n;
  return true;
}

//-----------------------------------------------------------------------------

//! Returns the currently active Node.
//! \param[out] Node requested Node.
//! \return true in case of success, false -- otherwise.
bool asiUI_ObjectBrowser::selectedNode(Handle(ActAPI_INode)& Node) const
{
  QTreeWidgetItem* pItem = nullptr;
  return this->selectedNode(Node, pItem);
}

//-----------------------------------------------------------------------------

//! Returns the currently active Nodes.
//! \param[out] Nodes requested Nodes.
//! \return true in case of success, false -- otherwise.
bool asiUI_ObjectBrowser::selectedNodes(Handle(ActAPI_HNodeList)& Nodes) const
{
  QList<QTreeWidgetItem*> items = this->selectedItems();
  if ( !items.length() )
    return false;

  Nodes = new ActAPI_HNodeList;

  for ( QList<QTreeWidgetItem*>::iterator it = items.begin(); it != items.end(); ++it )
  {
    QTreeWidgetItem* item = *it;
    TCollection_AsciiString entry = QStr2AsciiStr( item->data(0, BrowserRoleNodeId).toString() );

    // Take the corresponding data object.
    Handle(ActAPI_INode) selected_n = m_model->FindNode(entry);
    //
    if ( selected_n.IsNull() || !selected_n->IsWellFormed() )
    {
      m_progress.SendLogMessage(LogWarn(Normal) << "Selected Node is invalid");
      item->setBackgroundColor(0, Qt::darkRed);
      continue;
    }

    // Populate the output list.
    Nodes->Append(selected_n);
  }

  return true;
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::showNodes(const Handle(ActAPI_HNodeList)& nodes)
{
  m_model->OpenCommand();
  {
    for ( ActAPI_HNodeList::Iterator nit(*nodes); nit.More(); nit.Next() )
    {
      const Handle(ActAPI_INode)& N = nit.Value();

      N->AddUserFlags(NodeFlag_IsPresentationVisible);

      // Iterate over the associated viewers to find the one where the selected
      // Node is presented.
      for ( size_t k = 0; k < m_viewers.size(); ++k )
        if ( m_viewers[k] && m_viewers[k]->PrsMgr()->IsPresented(N) )
        {
          m_viewers[k]->PrsMgr()->DeRenderPresentation(N);
          m_viewers[k]->PrsMgr()->Actualize(N);
        }

      emit show( N->GetId() );
    }
  }
  m_model->CommitCommand();
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::showOnlyNodes(const Handle(ActAPI_HNodeList)& nodes)
{
  /* ============================
   *  Gather the Nodes to affect.
   * ============================ */

  Handle(ActAPI_HNodeList) allNodes = new ActAPI_HNodeList; // Nodes to affect.

  // Remove visibility flags from all Nodes.
  Handle(ActAPI_INode) root_n = m_model->GetRootNode();
  //
  for ( Handle(ActAPI_IChildIterator) cit = root_n->GetChildIterator(true);
        cit->More();
        cit->Next() )
  {
    Handle(ActAPI_INode) N = cit->Value();
    //
    if ( !N.IsNull() &&
          N->HasUserFlags(NodeFlag_IsPresentationVisible) &&
          N->HasUserFlags(NodeFlag_IsPresentedInPartView) )
    {
      allNodes->Append(N);
    }
  }

  /* =================================
   *  Perform Data Model modification.
   * ================================= */

  m_model->OpenCommand();
  {
    for ( ActAPI_HNodeList::Iterator nit(*allNodes); nit.More(); nit.Next() )
    {
      const Handle(ActAPI_INode)& N = nit.Value();

      // Set inivisible state in the Data Model.
      N->RemoveUserFlags(NodeFlag_IsPresentationVisible);

      // Derender all presentations.
      for ( size_t k = 0; k < m_viewers.size(); ++k )
        if ( m_viewers[k] )
          m_viewers[k]->PrsMgr()->DeRenderPresentation(N);
    }

    // For the Nodes to keep visible, do the opposite.
    for ( ActAPI_HNodeList::Iterator nit(*nodes); nit.More(); nit.Next() )
    {
      const Handle(ActAPI_INode)& N = nit.Value();

      N->AddUserFlags(NodeFlag_IsPresentationVisible);

      // Iterate over the associated viewers to find the one where the selected
      // Node is presented.
      for ( size_t k = 0; k < m_viewers.size(); ++k )
        if ( m_viewers[k] && m_viewers[k]->PrsMgr()->IsPresented(N) )
          m_viewers[k]->PrsMgr()->Actualize(N);

      emit showOnly( N->GetId() );
    }
  }
  m_model->CommitCommand();
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::hideNodes(const Handle(ActAPI_HNodeList)& nodes)
{
  m_model->OpenCommand();
  {
    for ( ActAPI_HNodeList::Iterator nit(*nodes); nit.More(); nit.Next() )
    {
      const Handle(ActAPI_INode)& N = nit.Value();

      N->RemoveUserFlags(NodeFlag_IsPresentationVisible);

      // Iterate over the associated viewers to find the one where the selected
      // Node is presented.
      for ( size_t k = 0; k < m_viewers.size(); ++k )
        if ( m_viewers[k] && m_viewers[k]->PrsMgr()->IsPresented(N) )
        {
          m_viewers[k]->PrsMgr()->DeRenderPresentation(N);
          m_viewers[k]->Repaint();
        }

      emit hide( N->GetId() );
    }
  }
  m_model->CommitCommand();
}
