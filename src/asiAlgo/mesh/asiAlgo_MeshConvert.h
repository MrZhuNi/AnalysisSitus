//-----------------------------------------------------------------------------
// Created on: 14 December 2015
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

#ifndef asiAlgo_MeshConvert_h
#define asiAlgo_MeshConvert_h

// A-Situs includes
#include <asiAlgo.h>

// OCCT includes
#include <Poly_Triangulation.hxx>
#include <TopoDS_Shape.hxx>

// Mesh (Active Data) includes
#include <ActData_Mesh.h>

// VTK includes
#pragma warning(push, 0)
#include <vtkPolyData.h>
#pragma warning(pop)

//-----------------------------------------------------------------------------

//! Services to convert mesh from one format to another.
namespace asiAlgo_MeshConvert
{
  asiAlgo_EXPORT bool
    ToPersistent(const TopoDS_Shape&   source,
                 Handle(ActData_Mesh)& result);

  asiAlgo_EXPORT bool
    ToPersistent(const Handle(Poly_Triangulation)& source,
                 Handle(ActData_Mesh)&             result);

  asiAlgo_EXPORT bool
    FromPersistent(const Handle(ActData_Mesh)& source,
                   Handle(Poly_Triangulation)& result);

  asiAlgo_EXPORT bool
    ToPersistent(vtkPolyData*          source,
                 Handle(ActData_Mesh)& result);

  asiAlgo_EXPORT bool
    FromVTK(vtkPolyData*                source,
            Handle(Poly_Triangulation)& result);

  asiAlgo_EXPORT bool
    ToVTK(const Handle(Poly_Triangulation)& source,
          vtkSmartPointer<vtkPolyData>&     result);

//-----------------------------------------------------------------------------

  //! Translates the passed mesh element to VTK polygonal cell.
  //! \param[in]      source   source mesh.
  //! \param[in]      elem     mesh element to translate.
  //! \param[in, out] polyData output polygonal data.
  //! \param[in, out] nodeRepo already processed nodes.
  void
    __translateElement(const Handle(Poly_Triangulation)&    source,
                       const Poly_Triangle&                 elem,
                       vtkPolyData*                         polyData,
                       NCollection_DataMap<int, vtkIdType>& nodeRepo);

  //! Adds the mesh element with the given ID to VTK polygonal data as a
  //! Triangle or Quadrangle VTK element depending on the actual number
  //! of nodes.
  //! \param[in]      source   source mesh.
  //! \param[in]      nodes    node IDs.
  //! \param[in]      nbNodes  number of nodes.
  //! \param[in, out] polyData polygonal data being populated.
  //! \param[in, out] nodeRepo registered nodes.
  //! \return ID of the just added VTK cell.
  vtkIdType
    __registerMeshFace(const Handle(Poly_Triangulation)&    source,
                       const void*                          nodes,
                       const int                            nbNodes,
                       vtkPolyData*                         polyData,
                       NCollection_DataMap<int, vtkIdType>& nodeRepo);

  //! Adds a mesh node as a point to the passed polygonal data.
  //! \param[in] source       source mesh.
  //! \param[in] nodeID       node ID.
  //! \param[in,out] polyData output polygonal data.
  //! \param[in,out] nodeRepo registered nodes.
  //! \return internal VTK ID for the newly added point.
  vtkIdType
    __registerMeshNode(const Handle(Poly_Triangulation)&    source,
                       const int                            nodeID,
                       vtkPolyData*                         polyData,
                       NCollection_DataMap<int, vtkIdType>& nodeRepo);

};

#endif
