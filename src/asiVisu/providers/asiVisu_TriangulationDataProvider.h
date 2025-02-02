//-----------------------------------------------------------------------------
// Created on: 11 July 2017
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

#ifndef asiVisu_TriangulationDataProvider_h
#define asiVisu_TriangulationDataProvider_h

// asiVisu includes
#include <asiVisu_DataProvider.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

//-----------------------------------------------------------------------------

//! Data provider from triangulation parameter to visualization pipeline.
//! This provider is independent of a Node type.
class asiVisu_TriangulationDataProvider : public asiVisu_DataProvider
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiVisu_TriangulationDataProvider, asiVisu_DataProvider)

public:

  //! Default ctor.
  asiVisu_EXPORT
    asiVisu_TriangulationDataProvider();

  //! Ctor accepting Triangulation Parameter.
  //! \param[in] triangulationParam Parameter holding the triangulation to visualize.
  //! \param[in] hasScalarsParam    Parameter holding the Boolean value that indicates
  //!                               whether to apply colors by scalars.
  //! \param[in] colorParam         Parameter holding the custom color.
  asiVisu_EXPORT
    asiVisu_TriangulationDataProvider(const Handle(ActData_TriangulationParameter)& triangulationParam,
                                      const Handle(ActData_BoolParameter)&          hasScalarsParam,
                                      const Handle(ActData_IntParameter)&           colorParam);

public:

  //! Returns ID of the Data Node which is being sourced by the visualization
  //! pipeline. This ID is bound to the pipeline's actor in order to have a
  //! back-reference from Presentation to Data Object.
  //! \return Node ID.
  asiVisu_EXPORT virtual ActAPI_DataObjectId
    GetNodeID() const;

  //! Returns the OCCT triangulation to be visualized.
  //! \return OCCT facet model.
  asiVisu_EXPORT virtual Handle(Poly_Triangulation)
    GetTriangulation() const;

  //! \return true if scalars are active.
  asiVisu_EXPORT bool
    HasScalars() const;

  //! Returns persistent color.
  //! \param[out] r red component.
  //! \param[out] g green component.
  //! \param[out] b blue component.
  asiVisu_EXPORT void
    GetColor(double& r, double& g, double& b) const;

public:

  //! \return copy.
  asiVisu_EXPORT Handle(asiVisu_TriangulationDataProvider)
    Clone() const;

protected:

  //! Enumerates Data Parameters playing as sources for DOMAIN -> VTK
  //! translation process.
  //! \return source Parameters.
  virtual Handle(ActAPI_HParameterList)
    translationSources() const;

protected:

  //! Source Node.
  Handle(ActAPI_INode) m_node;

  //! Source Parameter with triangulation.
  Handle(ActData_TriangulationParameter) m_triangulationParam;

  //! Source Parameter with scalars on/off Boolean flag.
  Handle(ActData_BoolParameter) m_hasScalarsParam;

  //! Source Parameter with color.
  Handle(ActData_IntParameter) m_colorParam;

};

#endif
