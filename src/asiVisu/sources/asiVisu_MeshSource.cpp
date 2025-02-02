//-----------------------------------------------------------------------------
// Created on: 20 November 2015
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
#include <asiVisu_MeshSource.h>

// asiVisu includes
#include <asiVisu_MeshPrimitive.h>
#include <asiVisu_MeshUtils.h>
#include <asiVisu_Utils.h>

// VTK includes
#include <vtkCellData.h>
#include <vtkDataObject.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

// Mesh (Active Data) includes
#include <ActData_Mesh_Edge.h>
#include <ActData_Mesh_EdgesIterator.h>
#include <ActData_Mesh_ElementsIterator.h>
#include <ActData_Mesh_Node.h>
#include <ActData_Mesh_Quadrangle.h>
#include <ActData_Mesh_Triangle.h>

//-----------------------------------------------------------------------------
// Construction
//-----------------------------------------------------------------------------

vtkStandardNewMacro(asiVisu_MeshSource)

//-----------------------------------------------------------------------------

//! Default constructor.
asiVisu_MeshSource::asiVisu_MeshSource()
{
  this->EmptyGroupForAllModeOn();
  this->SetNumberOfInputPorts(0); // Connected directly to our own Data Provider
                                  // which has nothing to do with VTK pipeline
}

//-----------------------------------------------------------------------------

//! Destructor.
asiVisu_MeshSource::~asiVisu_MeshSource()
{
}

//-----------------------------------------------------------------------------
// Kernel methods
//-----------------------------------------------------------------------------

//! Switches ON inverse mode of filtering.
void asiVisu_MeshSource::EmptyGroupForAllModeOn()
{
  m_bIsEmptyGroupForAll = true;
}

//-----------------------------------------------------------------------------

//! Switches OFF inverse mode of filtering.
void asiVisu_MeshSource::EmptyGroupForAllModeOff()
{
  m_bIsEmptyGroupForAll = false;
}

//-----------------------------------------------------------------------------

//! Sets the instance of Mesh DS being used as an input for Data Source.
//! \param theMesh [in] input Mesh DS.
void asiVisu_MeshSource::SetInputMesh(const Handle(ActData_Mesh)& theMesh)
{
  m_mesh = theMesh;
}

//-----------------------------------------------------------------------------

//! Sets the Mesh Group used to filter the input mesh.
//! \param theGroup [in] input Mesh Group.
void asiVisu_MeshSource::SetInputElemGroup(const Handle(ActData_Mesh_Group)& theGroup)
{
  m_elemGroup = theGroup;
}

//-----------------------------------------------------------------------------

//! Accessor for the input Mesh DS.
//! \return requested Mesh DS.
const Handle(ActData_Mesh)& asiVisu_MeshSource::GetInputMesh() const
{
  return m_mesh;
}

//-----------------------------------------------------------------------------

//! Accessor for the input Mesh Group.
//! \return requested Mesh Group.
const Handle(ActData_Mesh_Group)& asiVisu_MeshSource::GetInputElemGroup() const
{
  return m_elemGroup;
}

//-----------------------------------------------------------------------------

//! This method (called by superclass) performs conversion of OCCT
//! data structures to VTK polygonal representation.
//!
//! \param request      [in]  describes "what" algorithm should do. This is
//!                           typically just one key such as REQUEST_INFORMATION.
//! \param inputVector  [in]  inputs of the algorithm.
//! \param outputVector [out] outputs of the algorithm.
int asiVisu_MeshSource::RequestData(vtkInformation*        theRequest,
                                    vtkInformationVector** theInputVector,
                                    vtkInformationVector*  theOutputVector)
{
  /* ==============================
   *  Prepare involved collections
   * ============================== */

  vtkPolyData* aPolyOutput = vtkPolyData::GetData(theOutputVector);
  aPolyOutput->Allocate();

  vtkPointData* aPD = aPolyOutput->GetPointData();
  vtkCellData*  aCD = aPolyOutput->GetCellData();

  vtkSmartPointer<vtkPoints> aPoints = vtkSmartPointer<vtkPoints>::New();
  aPolyOutput->SetPoints(aPoints);

  /* ==================================
   *  Initialize necessary data arrays
   * ================================== */

  // Array for mesh item types
  vtkSmartPointer<vtkIntArray>
    aTypeArr = asiVisu_Utils::InitIntArray(ARRNAME_MESH_ITEM_TYPE);
  aCD->AddArray(aTypeArr);

  // Array for mesh node IDs
  vtkSmartPointer<vtkIntArray>
    aNodeIDsArr = asiVisu_Utils::InitIntArray(ARRNAME_MESH_NODE_IDS);
  aPD->AddArray(aNodeIDsArr);

  // Array for mesh element IDs
  vtkSmartPointer<vtkIntArray>
    aFaceIDsArr = asiVisu_Utils::InitIntArray(ARRNAME_MESH_ELEM_IDS);
  aCD->AddArray(aFaceIDsArr);

  /* ==============================================================
   *  Take care of free nodes by collecting them into a dedicated
   *  VTK_POLY_VERTEX cell. Notice that such cell (as well as
   *  simple VTK_VERTEX) must be added FIRST when working with
   *  vtkPolyData objects in order to have the data set consistent
   * ============================================================== */

  // Prepare back references for the elements stored in Mesh DS. Thus we are
  // able to track free nodes
  m_mesh->RebuildAllInverseConnections();

  // Iterate over the entire collection of nodes to cumulate them into
  // a sequence and prepare a single VTK cell for all detected free nodes
  NCollection_Sequence<int> aFreeNodeIDs;
  for ( ActData_Mesh_ElementsIterator nit(m_mesh, ActData_Mesh_ET_Node); nit.More(); nit.Next() )
  {
    // Access next node
    Handle(ActData_Mesh_Node) aNode = Handle(ActData_Mesh_Node)::DownCast( nit.GetValue() );
    const ActData_Mesh_ListOfElements& lstInvElements = aNode->InverseElements();

    // Skip connected nodes as we're looking for free ones here
    if ( lstInvElements.Extent() > 0 )
      continue;

    aFreeNodeIDs.Append( aNode->GetID() );
  }
  //
  this->registerFreeNodesCell(aFreeNodeIDs, aPolyOutput);

  /* =======================================
   *  Pass mesh elements to VTK data source
   * ======================================= */

  if ( m_elemGroup.IsNull() && m_bIsEmptyGroupForAll ) // No filtering by group enabled
  {
    ActData_Mesh_ElementsIterator aMeshElemsIt(m_mesh, ActData_Mesh_ET_Face);
    for ( ; aMeshElemsIt.More(); aMeshElemsIt.Next() )
      this->translateElement(aMeshElemsIt.GetValue(), aPolyOutput);
  }
  else if ( !m_elemGroup.IsNull() )
  {
    const ActData_Mesh_MapOfElements& aGroupElems = m_elemGroup->Elements();
    ActData_Mesh_MapOfElements::Iterator aGroupIt(aGroupElems);
    for ( ; aGroupIt.More(); aGroupIt.Next() )
      this->translateElement(aGroupIt.Key(), aPolyOutput);
  }

  return Superclass::RequestData(theRequest, theInputVector, theOutputVector);
}

//-----------------------------------------------------------------------------

//! Translates the passed mesh element to VTK polygonal cell.
//! \param theElem     [in]     Mesh element to translate.
//! \param thePolyData [in/out] output polygonal data.
void asiVisu_MeshSource::translateElement(const Handle(ActData_Mesh_Element)& theElem,
                                          vtkPolyData*                thePolyData)
{
  // Proceed with TRIANGLE elements
  if ( theElem->IsInstance( STANDARD_TYPE(ActData_Mesh_Triangle) ) )
  {
    // Access element data
    Handle(ActData_Mesh_Triangle) aTriElem = Handle(ActData_Mesh_Triangle)::DownCast(theElem);
    int aTriNodeIds[3], aNbNodes;
    aTriElem->GetFaceDefinedByNodes(3, aTriNodeIds, aNbNodes);

    // Transfer element to VTK polygonal source
    this->registerMeshFace(aTriElem->GetID(), aTriNodeIds, aNbNodes, thePolyData);
  }
  // Proceed with QUADRANGLE elements
  else if ( theElem->IsInstance( STANDARD_TYPE(ActData_Mesh_Quadrangle) ) )
  {
    // Access element data
    Handle(ActData_Mesh_Quadrangle) aQuadElem = Handle(ActData_Mesh_Quadrangle)::DownCast(theElem);
    int aQuadNodeIds[4], aNbNodes;
    aQuadElem->GetFaceDefinedByNodes(4, aQuadNodeIds, aNbNodes);

    // Transfer element to VTK polygonal source
    this->registerMeshFace(aQuadElem->GetID(), aQuadNodeIds, aNbNodes, thePolyData);
  }
}

//-----------------------------------------------------------------------------

//! Adds a mesh node as a point to the passed polygonal data.
//! \param theNodeID   [in]     node ID.
//! \param thePolyData [in/out] output polygonal data.
//! \return internal VTK ID for the newly added point.
vtkIdType asiVisu_MeshSource::registerMeshNode(const int    theNodeID,
                                               vtkPolyData* thePolyData)
{
  // Access necessary arrays
  vtkPoints* aPoints = thePolyData->GetPoints();
  vtkIntArray* aNodeIDsArr =
    vtkIntArray::SafeDownCast( thePolyData->GetPointData()->GetArray(ARRNAME_MESH_NODE_IDS) );

  // Access mesh node
  Handle(ActData_Mesh_Node) aNode = m_mesh->FindNode(theNodeID);

  vtkIdType aResPid;
  if ( !m_regPoints.IsBound(theNodeID) )
  {
    // Get nodal coordinates
    gp_Pnt aPnt = aNode->Pnt();

    // Push the point into VTK data set
    aResPid = aPoints->InsertNextPoint( aPnt.X(), aPnt.Y(), aPnt.Z() );
    aNodeIDsArr->InsertNextValue( aNode->GetID() );

    m_regPoints.Bind(theNodeID, aResPid);
  }
  else
    aResPid = m_regPoints.Find(theNodeID);

  return aResPid;
}

//-----------------------------------------------------------------------------

//! Adds the mesh element with the given ID to VTK polygonal data as a
//! Triangle or Quadrangle VTK element depending on the actual number
//! of nodes.
//! \param theFaceID   [in] ID of the element.
//! \param theNodes    [in] node IDs.
//! \param theNbNodes  [in] number of nodes.
//! \param thePolyData [in] polygonal data being populated.
//! \return ID of the just added VTK cell.
vtkIdType asiVisu_MeshSource::registerMeshFace(const int    theFaceID,
                                               const void*  theNodes,
                                               const int    theNbNodes,
                                               vtkPolyData* thePolyData)
{
  /* ==============================================
   *  Define the type of the cell being registered
   * ============================================== */

  VTKCellType aCellType = VTK_EMPTY_CELL;
  switch ( theNbNodes )
  {
    case 3: aCellType = VTK_TRIANGLE; break;
    case 4: aCellType = VTK_QUAD; break;
    default: return VTK_BAD_ID;
  }

  /* =========================================
   *  Register cell in the polygonal data set
   * ========================================= */

  int* aNodeIDs = (int*) theNodes;
  std::vector<vtkIdType> aPids;
  for ( int k = 0; k < theNbNodes; k++ )
  {
    vtkIdType aPointID = this->registerMeshNode(aNodeIDs[k], thePolyData);
    aPids.push_back(aPointID);
  }

  vtkIdType aCellID =
    thePolyData->InsertNextCell( aCellType, (int) aPids.size(), &aPids[0] );

  /* ==================================================
   *  Associate the newly added cell with element type
   * ================================================== */

  vtkIntArray* aTypeArr =
    vtkIntArray::SafeDownCast( thePolyData->GetCellData()->GetArray(ARRNAME_MESH_ITEM_TYPE) );
  int aType = (aCellType == VTK_TRIANGLE ? MeshPrimitive_CellTriangle : MeshPrimitive_CellQuad);
  aTypeArr->InsertNextValue(aType);

  /* =========================================================
   *  Associate the newly added cell with original element ID
   * ========================================================= */

  vtkIntArray* aFaceIDsArr =
    vtkIntArray::SafeDownCast( thePolyData->GetCellData()->GetArray(ARRNAME_MESH_ELEM_IDS) );
  aFaceIDsArr->InsertNextValue(theFaceID);

  return aCellID;
}

//-----------------------------------------------------------------------------

//! Registers the passed mesh nodes as free nodes by pushing them into a
//! dedicated VTK cell of VTK_POLY_VERTEX type.
//! \param theNodeIDs  [in] collection of free node IDs.
//! \param thePolyData [in] polygonal data being populated.
//! \return ID of the registered cell.
vtkIdType
  asiVisu_MeshSource::registerFreeNodesCell(const NCollection_Sequence<int>& theNodeIDs,
                                            vtkPolyData*                     thePolyData)
{
  if ( theNodeIDs.IsEmpty() )
    return VTK_BAD_ID;

  std::vector<vtkIdType> aPids;
  NCollection_Sequence<int>::Iterator anIt(theNodeIDs);
  for ( ; anIt.More(); anIt.Next() )
  {
    int aNodeID = anIt.Value();
    aPids.push_back( this->registerMeshNode(aNodeID, thePolyData) );
  }

  vtkIdType aCellID =
    thePolyData->InsertNextCell( VTK_POLY_VERTEX, (int) aPids.size(), &aPids[0] );

  vtkIntArray* aTypeArr =
    vtkIntArray::SafeDownCast( thePolyData->GetCellData()->GetArray(ARRNAME_MESH_ITEM_TYPE) );
  aTypeArr->InsertNextValue(MeshPrimitive_FreeNode);

  return aCellID;
}
