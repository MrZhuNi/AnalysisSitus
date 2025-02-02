//-----------------------------------------------------------------------------
// Created on: 14 March 2016
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
#include <asiAlgo_OBJ.h>

// OCCT includes
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepLProp_SLProps.hxx>
#include <Graphic3d_Vec3.hxx>
#include <Graphic3d_Vec4.hxx>
#include <OSD_OpenFile.hxx>
#include <Standard_CLocaleSentry.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

//-----------------------------------------------------------------------------

//! Trivial converter.
Graphic3d_Vec3 objXyzToVec(const gp_XYZ& thePnt)
{
  return Graphic3d_Vec3( (float) thePnt.X(), (float) thePnt.Y(), (float) thePnt.Z() );
}

//-----------------------------------------------------------------------------

//! Auxiliary class to write OBJ.
class ObjWriter
{
public:

  //! Main constructor.
  ObjWriter (const TCollection_AsciiString& theName,
             const bool                     hasNormals)
  //
  : myFile       ( OSD_OpenFile(theName.ToCString(), "wb") ),
    myName       ( theName),
    myHasNormals ( hasNormals )
  {
    if ( myFile == nullptr )
    {
      std::cout << "Error: OBJ file cannot be created" << std::endl;
      return;
    }
  }

  //! Destructor, will emit error message if file was not closed.
  ~ObjWriter()
  {
    if ( myFile != nullptr )
    {
      ::fclose(myFile);
    }
  }

  //! Return true if file has been opened.
  bool IsOpened() const { return myFile != nullptr; }

  //! Correctly close the file.
  void Close()
  {
    ::fclose(myFile);
    myFile = nullptr;
  }

  //! Writes the header.
  bool WriteHeader(const int theNbNodes,
                   const int theNbElems)
  {
    return ::Fprintf(myFile, "# Exported by OpenCASCADE DMU Processor [www.opencascade.com]\n"
                             "#  Vertices: %d\n"
                             "#     Faces: %d\n", theNbNodes, theNbElems) != 0;
  }

  //! Writes a triangle.
  bool WriteTriangle(const Graphic3d_Vec4i& theTri)
  {
    if ( myHasNormals )
    {
      return Fprintf(myFile, "f %d//%d %d//%d %d//%d\n",
                     theTri[0] + 1, theTri[0] + 1,
                     theTri[1] + 1, theTri[1] + 1,
                     theTri[2] + 1, theTri[2] + 1) != 0;
    }
    return Fprintf(myFile, "f %d %d %d\n",
                   theTri[0] + 1,
                   theTri[1] + 1,
                   theTri[2] + 1) != 0;
  }

  //! Writes a quad.
  bool WriteQuad(const Graphic3d_Vec4i& theQuad)
  {
    if ( myHasNormals )
    {
      return Fprintf(myFile, "f %d//%d %d//%d %d//%d %d//%d\n",
                     theQuad[0] + 1, theQuad[0] + 1,
                     theQuad[1] + 1, theQuad[1] + 1,
                     theQuad[2] + 1, theQuad[2] + 1,
                     theQuad[3] + 1, theQuad[3] + 1) != 0;
    }
    return Fprintf(myFile, "f %d %d %d %d\n",
                   theQuad[0] + 1,
                   theQuad[1] + 1,
                   theQuad[2] + 1,
                   theQuad[3] + 1) != 0;
  }

  //! Writes a vector.
  bool WriteVertex(const Graphic3d_Vec3& theValue)
  {
    return Fprintf( myFile, "v %f %f %f\n",  theValue.x(), theValue.y(), theValue.z() ) != 0;
  }

  //! Writes a normal.
  bool WriteNormal (const Graphic3d_Vec3& theValue)
  {
    return Fprintf(myFile, "vn %f %f %f\n", theValue.x(), theValue.y(), theValue.z()) != 0;
  }

  //! Writes a group name.
  bool WriteGroup(const TCollection_AsciiString& theValue)
  {
    return Fprintf( myFile, "g %s\n", theValue.ToCString() ) != 0;
  }

private:

  FILE*                   myFile;
  TCollection_AsciiString myName;
  bool                    myHasNormals;
  bool                    myIsOpened;

};

//-----------------------------------------------------------------------------

bool asiAlgo_OBJ::Write(const TopoDS_Shape&            theShape,
                        const TCollection_AsciiString& theFilename)
{
  if ( theShape.IsNull() )
    return false;

  int aNbNodesAll = 0;
  int aNbElemsAll = 0;
  for ( TopExp_Explorer aFaceIter(theShape, TopAbs_FACE); aFaceIter.More(); aFaceIter.Next() )
  {
    TopLoc_Location L;
    const TopoDS_Face&                F = TopoDS::Face( aFaceIter.Current() );
    const Handle(Poly_Triangulation)& T = BRep_Tool::Triangulation(F, L);
    //
    if ( T.IsNull() )
      continue;
    //
    aNbNodesAll += T->Nodes().Length();
    aNbElemsAll += T->Triangles().Length();
  }

  if ( aNbNodesAll == 0 || aNbElemsAll == 0 )
  {
    std::cout << "Error: no mesh data to save" << std::endl;
    return false;
  }

  // Open file and dump header info
  Standard_CLocaleSentry aLocaleSentry;
  ObjWriter anObjFile(theFilename, false);
  if ( !anObjFile.IsOpened() || !anObjFile.WriteHeader(aNbNodesAll, aNbElemsAll) )
  {
    std::cout << "Error: cannot write header" << std::endl;
    return false;
  }

  // Write tessellation
  int             aFirstNode = 0;
  Graphic3d_Vec4i aTriNodes(-1, -1, -1, -1);
  int             aNbFaces = 0;
  //
  for ( TopExp_Explorer aFaceIter(theShape, TopAbs_FACE); aFaceIter.More(); aFaceIter.Next() )
  {
    ++aNbFaces;

    TopLoc_Location L;
    const TopoDS_Face&                F = TopoDS::Face( aFaceIter.Current() );
    const Handle(Poly_Triangulation)& T = BRep_Tool::Triangulation(F, L);
    //
    if ( T.IsNull() )
      continue;
    //
    const int              aLower     = T->Triangles().Lower();
    const bool isMirrored = L.Transformation().VectorialPart().Determinant() < 0.0;

    TCollection_AsciiString aRefName("unnamed");
    if ( !anObjFile.WriteGroup(aRefName) )
    {
      std::cout << "Error: cannot write group" << std::endl;
      return false;
    }

    // Write nodes
    const TColgp_Array1OfPnt& aNodes = T->Nodes();
    const gp_Trsf&            aTrsf  = L.Transformation();
    for ( int aNodeIter = aNodes.Lower(); aNodeIter <= aNodes.Upper(); ++aNodeIter )
    {
      gp_Pnt aNode = aNodes(aNodeIter);
      aNode.Transform(aTrsf);
      //
      if ( !anObjFile.WriteVertex( objXyzToVec( aNode.XYZ() ) ) )
      {
        std::cout << "Error: cannot write vertex" << std::endl;
        return false;
      }
    }

    // Write indices
    for ( Poly_Array1OfTriangle::Iterator tIter( T->Triangles() ); tIter.More(); tIter.Next() )
    {
      if ( (F.Orientation() == TopAbs_REVERSED) ^ isMirrored )
      {
        tIter.Value().Get(aTriNodes[0], aTriNodes[2], aTriNodes[1]);
      }
      else
      {
        tIter.Value().Get(aTriNodes[0], aTriNodes[1], aTriNodes[2]);
      }

      aTriNodes[0] = aFirstNode + aTriNodes[0] - aLower;
      aTriNodes[1] = aFirstNode + aTriNodes[1] - aLower;
      aTriNodes[2] = aFirstNode + aTriNodes[2] - aLower;

      if ( !anObjFile.WriteTriangle(aTriNodes) )
      {
        std::cout << "Error: cannot write triangle" << std::endl;
        return false;
      }
    }
    aFirstNode += T->NbNodes();
  }

  anObjFile.Close();
  return true;
}
