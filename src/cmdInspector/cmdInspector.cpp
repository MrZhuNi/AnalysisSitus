//-----------------------------------------------------------------------------
// Created on: 24 August 2017
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

// cmdInspector includes
#include <cmdInspector.h>

// asiTcl includes
#include <asiTcl_PluginMacro.h>

// asiEngine includes
#include <asiEngine_Part.h>

// asiAlgo includes
#include <asiAlgo_TopoKill.h>

// OCCT includes
#include <ShapeFix_Shape.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Solid.hxx>

//-----------------------------------------------------------------------------

Handle(exe_CommonFacilities) cmdInspector::cf = NULL;

//-----------------------------------------------------------------------------

int INSPECTOR_SetAsPart(const Handle(asiTcl_Interp)& interp,
                        int                          argc,
                        const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  Handle(asiData_IVTopoItemNode)
    node = Handle(asiData_IVTopoItemNode)::DownCast( cmdInspector::cf->Model->FindNodeByName(argv[1]) );
  //
  if ( node.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find topological object with name %1." << argv[1]);
    return TCL_ERROR;
  }

  // Modify Data Model
  cmdInspector::cf->Model->OpenCommand();
  {
    asiEngine_Part(cmdInspector::cf->Model, NULL).Update( node->GetShape() );
  }
  cmdInspector::cf->Model->CommitCommand();

  // Update UI
  cmdInspector::UpdateUI();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int INSPECTOR_FixPart(const Handle(asiTcl_Interp)& interp,
                      int                          argc,
                      const char**                 argv)
{
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Part Node
  Handle(asiData_PartNode) part_n = cmdInspector::cf->Model->GetPartNode();

  // Fix shape
  ShapeFix_Shape fix( part_n->GetShape() );
  fix.Perform();
  TopoDS_Shape result = fix.Shape();

  // Modify Data Model
  cmdInspector::cf->Model->OpenCommand();
  {
    asiEngine_Part(cmdInspector::cf->Model, NULL).Update(result);
  }
  cmdInspector::cf->Model->CommitCommand();

  // Update UI
  cmdInspector::UpdateUI();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int INSPECTOR_KillEdge(const Handle(asiTcl_Interp)& interp,
                       int                          argc,
                       const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  const int eidx = atoi(argv[1]);
  //
  if ( eidx < 1 )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Edge index should be 1-based.");
    return TCL_ERROR;
  }

  // Get Part Node
  Handle(asiData_PartNode) part_n = cmdInspector::cf->Model->GetPartNode();

  // Get map of edges with respect to those the passed index is relevant
  const TopTools_IndexedMapOfShape& allEdges = part_n->GetAAG()->GetMapOfEdges();

  // Prepare killer
  asiAlgo_TopoKill killer( cmdInspector::cf->Model->GetPartNode()->GetShape(),
                           interp->GetProgress(),
                           interp->GetPlotter() );
  //
  if ( !killer.AskRemove( allEdges(eidx) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Request on removal of edge %1 was rejected." << eidx);
    return TCL_OK;
  }

  if ( !killer.Apply() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Topological killer failed.");
    return TCL_OK;
  }

  // Get result
  const TopoDS_Shape& result = killer.GetResult();

  // Modify Data Model
  cmdInspector::cf->Model->OpenCommand();
  {
    asiEngine_Part(cmdInspector::cf->Model, NULL).Update(result);
  }
  cmdInspector::cf->Model->CommitCommand();

  // Update UI
  cmdInspector::UpdateUI();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int INSPECTOR_KillFace(const Handle(asiTcl_Interp)& interp,
                       int                          argc,
                       const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  const int fidx = atoi(argv[1]);
  //
  if ( fidx < 1 )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Face index should be 1-based.");
    return TCL_ERROR;
  }

  // Get Part Node
  Handle(asiData_PartNode) part_n = cmdInspector::cf->Model->GetPartNode();

  // Get map of faces with respect to those the passed index is relevant
  const TopTools_IndexedMapOfShape& allFaces = part_n->GetAAG()->GetMapOfFaces();

  // Prepare killer
  asiAlgo_TopoKill killer( cmdInspector::cf->Model->GetPartNode()->GetShape(),
                           interp->GetProgress(),
                           interp->GetPlotter() );
  //
  if ( !killer.AskRemove( allFaces(fidx) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Request on removal of face %1 was rejected." << fidx);
    return TCL_OK;
  }

  if ( !killer.Apply() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Topological killer failed.");
    return TCL_OK;
  }

  // Get result
  const TopoDS_Shape& result = killer.GetResult();

  // Modify Data Model
  cmdInspector::cf->Model->OpenCommand();
  {
    asiEngine_Part(cmdInspector::cf->Model, NULL).Update(result);
  }
  cmdInspector::cf->Model->CommitCommand();

  // Update UI
  cmdInspector::UpdateUI();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int INSPECTOR_KillSolidByFace(const Handle(asiTcl_Interp)& interp,
                              int                          argc,
                              const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  const int fidx = atoi(argv[1]);
  //
  if ( fidx < 1 )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Face index should be 1-based.");
    return TCL_ERROR;
  }

  // Get Part Node
  Handle(asiData_PartNode) part_n = cmdInspector::cf->Model->GetPartNode();

  // Get map of faces with respect to those the passed index is relevant
  const TopTools_IndexedMapOfShape& allFaces = part_n->GetAAG()->GetMapOfFaces();

  // Get face in question
  TopoDS_Face face = TopoDS::Face( allFaces(fidx) );

  // Get owner solid
  TopTools_IndexedDataMapOfShapeListOfShape faceOwners;
  TopExp::MapShapesAndAncestors(part_n->GetShape(), TopAbs_FACE, TopAbs_SOLID, faceOwners);
  //
  if ( faceOwners.IsEmpty() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find parent solid for face %1." << fidx);
    return TCL_OK;
  }

  // Get owner shapes
  TopTools_ListOfShape owners = faceOwners.FindFromKey(face);
  //
  if ( owners.IsEmpty() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "There are no parents for face %1. Cannot proceed." << fidx);
    return TCL_OK;
  }

  // Get solid
  TopoDS_Solid ownerSolid = TopoDS::Solid( owners.First() );
  //
  if ( ownerSolid.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Owner solid is null. Cannot proceed." << fidx);
    return TCL_OK;
  }

  // Prepare killer
  asiAlgo_TopoKill killer( cmdInspector::cf->Model->GetPartNode()->GetShape(),
                           interp->GetProgress(),
                           interp->GetPlotter() );
  //
  if ( !killer.AskRemove(ownerSolid) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Request on removal of solid was rejected." << fidx);
    return TCL_OK;
  }

  if ( !killer.Apply() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Topological killer failed.");
    return TCL_OK;
  }

  // Get result
  const TopoDS_Shape& result = killer.GetResult();

  // Modify Data Model
  cmdInspector::cf->Model->OpenCommand();
  {
    asiEngine_Part(cmdInspector::cf->Model, NULL).Update(result);
  }
  cmdInspector::cf->Model->CommitCommand();

  // Update UI
  cmdInspector::UpdateUI();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

void cmdInspector::UpdateUI()
{
  cf->ViewerPart->PrsMgr()->Actualize(cf->Model->GetPartNode(), false, false, true);
}

//-----------------------------------------------------------------------------

void cmdInspector::Factory(const Handle(asiTcl_Interp)&      interp,
                           const Handle(Standard_Transient)& data)
{
  static const char* group = "cmdInspector";

  /* ==============================
   *  Initialize common facilities
   * ============================== */

  cmdInspector::cf = Handle(exe_CommonFacilities)::DownCast(data);
  //
  if ( cf.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "[cmdInspector] Common facilities are NULL.");
    return;
  }

  // Add commands
  // ...

  //-------------------------------------------------------------------------//
  interp->AddCommand("set-as-part",
    //
    "set-as-part varName\n"
    "\t Sets the object with the given name as a part for analysis. \n"
    "\t The object is expected to exist as a topological item in \n"
    "\t imperative plotter.",
    //
    __FILE__, group, INSPECTOR_SetAsPart);

  //-------------------------------------------------------------------------//
  interp->AddCommand("fix-part",
    //
    "fix-part\n"
    "\t Performs automatic shape repair on the active part.",
    //
    __FILE__, group, INSPECTOR_FixPart);

  //-------------------------------------------------------------------------//
  interp->AddCommand("kill-edge",
    //
    "kill-edge edgeIndex\n"
    "\t Kills edge with the passed 1-based index from the active part. \n"
    "\t This is a pure topological operation which does not attempt to \n"
    "\t modify geometry. Moreover, unlike Euler operator, this function \n"
    "\t does not preserve the topological consistency of the CAD part. \n"
    "\t We have introduced this function to ground Euler operators on it.",
    //
    __FILE__, group, INSPECTOR_KillEdge);

  //-------------------------------------------------------------------------//
  interp->AddCommand("kill-face",
    //
    "kill-face faceIndex\n"
    "\t Kills face with the passed 1-based index from the active part. \n"
    "\t This is a pure topological operation which does not attempt to \n"
    "\t modify geometry. Moreover, unlike Euler operator, this function \n"
    "\t does not preserve the topological consistency of the CAD part. \n"
    "\t We have introduced this function to ground Euler operators on it.",
    //
    __FILE__, group, INSPECTOR_KillFace);

  //-------------------------------------------------------------------------//
  interp->AddCommand("kill-solid-by-face",
	  //
	  "kill-solid-by-face faceIndex\n"
	  "\t Kills solid which contains a face with the passed 1-based index.",
	  //
	  __FILE__, group, INSPECTOR_KillSolidByFace);
}

// Declare entry point PLUGINFACTORY
ASIPLUGIN(cmdInspector)
