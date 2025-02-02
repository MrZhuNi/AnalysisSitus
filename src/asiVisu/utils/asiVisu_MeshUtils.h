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

#ifndef asiVisu_MeshUtils_h
#define asiVisu_MeshUtils_h

// Visualization includes
#include <asiVisu.h>

// VTK includes
#include <vtkLookupTable.h>
#include <vtkMapper.h>
#include <vtkSmartPointer.h>

// OCCT includes
#include <Standard_Type.hxx>

//! Auxiliary functions supporting mesh presentations in VTK viewer.
namespace asiVisu_MeshUtils
{
  asiVisu_EXPORT vtkSmartPointer<vtkLookupTable>
    InitLookupTable(const double ref_r,
                    const double ref_g,
                    const double ref_b);

  asiVisu_EXPORT vtkSmartPointer<vtkLookupTable>
    InitLookupTable();

  asiVisu_EXPORT void
    InitMapper(vtkMapper*      pMapper,
               vtkLookupTable* pLookup,
               const char*     pScalarsArrName);

  asiVisu_EXPORT void
    InitMapper(vtkMapper*  pMapper,
               const char* pScalarsArrName);

  asiVisu_EXPORT void
    InitMapper(vtkMapper*   pMapper,
               const char*  pScalarsArrName,
               const double ref_r,
               const double ref_g,
               const double ref_b);

  asiVisu_EXPORT double
    DefaultShrinkFactor();

  asiVisu_EXPORT double
    DefaultPointSize();

  asiVisu_EXPORT void
    DefaultContourColor(double& fR,
                        double& fG,
                        double& fB);

  asiVisu_EXPORT double
    DefaultContourLineWidth();

  asiVisu_EXPORT double
    DefaultContourOpacity();
};

#endif
